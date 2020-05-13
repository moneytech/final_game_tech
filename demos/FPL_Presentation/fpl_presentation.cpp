#define FPL_IMPLEMENTATION
#define FPL_PRIVATE
#include <final_platform_layer.h>

#define FGL_IMPLEMENTATION
#include <final_dynamic_opengl.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb/stb_truetype.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_STATIC
#include <stb/stb_image.h>

#define FXML_IMPLEMENTATION
#define FXML_PRIVATE
#include <final_xml.h>

#include <final_math.h> // Vec2f, Vec4f, Mat4f, etc.

// Contains fonts files as byte-array (Arimo, Sulphur-Point, Bitstream Vera Sans)
#include "fonts.h"

// Contains image files as byte-array (FPL-Logos)
#include "images.h"

// Contains the slide text for the FPL presentation
#include "slides.h" // TextDefinition, SlideDefinition

#include "types.h" // HorizontalAlignment, VerticalAlignment

#define DRAW_TEXT_BOUNDS 0
#define DRAW_IMAGE_BOUNDS 0
#define DRAW_SLIDE_CENTER 1
#define DRAW_VIEW_CENTER 0

template <typename T>
struct GrowablePool {
private:
	struct Entry {
		T value;
		Entry *next;
	};

	struct Bucket {
		Entry *firstFree;
		Bucket *next;
	};

	Entry *firstUsed;
	Entry *lastUsed;
	Bucket *firstBucket;
	Bucket *lastBucket;
	size_t entriesPerBucket;

	Bucket *AllocBucket() {
		fplAssert(entriesPerBucket > 0);
		size_t size = sizeof(Bucket) + sizeof(uintptr_t) + sizeof(Entry) * entriesPerBucket;
		Bucket *result = (Bucket *)fplMemoryAllocate(size);
		result->firstFree = (Entry *)((uint8_t *)result + sizeof(Bucket) + sizeof(uintptr_t));
		for (size_t i = 0; i < entriesPerBucket - 1; ++i) {
			Entry *thisEntry = result->firstFree + i;
			Entry *nextEntry = result->firstFree + (i + 1);
			thisEntry->next = nextEntry;
		}
		return(result);
	}
public:
	static GrowablePool<T> Make(const size_t entriesPerBucket) {
		GrowablePool<T> result = {};
		result.entriesPerBucket = entriesPerBucket;
	}

	T *Aquire() {
		// Init
		if (entriesPerBucket == 0) {
			entriesPerBucket = 64;
		}

		Bucket *bucket;
		if (!lastBucket) {
			// Initial bucket
			bucket = AllocBucket();
			lastBucket = firstBucket = bucket;
		}
		if (lastBucket->firstFree == nullptr) {
			// Bucket full, add another
			bucket = AllocBucket();
			lastBucket->next = bucket;
			lastBucket = bucket;
		} else {
			bucket = lastBucket;
		}
		// Add entry
		fplAssert(bucket != nullptr);
		fplAssert(bucket->firstFree != nullptr);
		Entry *entry = bucket->firstFree;
		Entry *next = entry->next;
		bucket->firstFree = next;
		T *result = &entry->value;
		entry->next = nullptr;
		return(result);
	}

	void Release() {
		Bucket *bucket = firstBucket;
		while (bucket != nullptr) {
			Bucket *next = bucket->next;
			fplMemoryFree(bucket);
			bucket = next;
		}
		firstBucket = lastBucket = nullptr;
	}
};

template <typename T>
struct LinkedList {
private:
	struct LinkedItem {
		T value;
		LinkedItem *next;
	};

	GrowablePool<LinkedItem> pool;
	LinkedItem *first;
	LinkedItem *last;
	size_t count;
public:
	struct Iterator {
		LinkedItem *cur;
		size_t index;

		bool HasNext() const {
			bool result = cur != nullptr;
			return(result);
		}

		T *MoveNext() {
			if (cur != nullptr) {
				cur = cur->next;
				++index;
				return(&cur->value);
			}
			return(nullptr);
		}

		T *Value() {
			if (cur != nullptr) {
				return(&cur->value);
			}
			return(nullptr);
		}
	};

	struct ConstIterator {
		const LinkedItem *cur;
		size_t index;

		bool HasNext() const {
			bool result = cur != nullptr;
			return(result);
		}

		const T *MoveNext() {
			if (cur != nullptr) {
				cur = cur->next;
				++index;
				return(&cur->value);
			}
			return(nullptr);
		}

		const T *Value() const {
			if (cur != nullptr) {
				return(&cur->value);
			}
			return(nullptr);
		}
	};

	size_t Count() const {
		return count;
	}

	T *Add() {
		LinkedItem *item = pool.Aquire();
		if (last == nullptr) {
			last = first = item;
		} else {
			last->next = item;
			last = item;
		}
		++count;
		return(&item->value);
	}

	Iterator GetIterator() {
		Iterator result = { first };
		return(result);
	}

	ConstIterator GetConstIterator() const {
		ConstIterator result = { first };
		return(result);
	}

	void Release() {
		pool.Release();
	}
};

typedef int32_t b32;

static char glErrorCodeBuffer[16];
static const char *GetGLErrorString(const GLenum err) {
	switch (err) {
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		default:
			if (_itoa_s(err, glErrorCodeBuffer, fplArrayCount(glErrorCodeBuffer), 10) == 0)
				return (const char *)glErrorCodeBuffer;
			else
				return "";
	}
}

static void CheckGLError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		const char *msg = GetGLErrorString(err);
		assert(!msg);
	}
}

struct String {
	const char *base;
	size_t len;

	String() {}
	String(const char *base) { this->base = base; this->len = 0; }
	String(const char *base, size_t len) { this->base = base; this->len = len; }
};

struct StringTable {
private:
	static constexpr size_t EntryPadding = 8;

	struct Entry {
		String str;
		size_t size;
		Entry *next;
	};

	Entry *first;
	Entry *last;
	size_t count;

	Entry *AllocEntry(const size_t size) {
		// @MEMORY(final): Use a memory pool instead
		Entry *entry = (Entry *)malloc(sizeof(Entry) + EntryPadding + size);
		*entry = {};
		entry->size = size;

		if (first == nullptr) {
			first = entry;
		}
		if (last != nullptr) {
			last->next = entry;
		}
		last = entry;

		++count;

		return(entry);
	}

	void ReleaseEntry(Entry *entry) {
		free(entry);
	}

public:
	String MakeString(size_t len) {
		Entry *entry = AllocEntry(len + 1);
		String *result = &entry->str;
		result->base = (char *)(uint8_t *)entry + sizeof(Entry) + EntryPadding;
		result->len = len + 1;
		return(*result);
	}

	const char *CopyString(const char *str, const size_t len) {
		String s = MakeString(len);
		fplCopyStringLen(str, len, (char *)s.base, len + 1);
		const char *result = s.base;
		return(result);
	}

	const char *CopyString(const char *str) {
		size_t len = fplGetStringLength(str);
		const char *result = CopyString(str, len);
		return(result);
	}

	const char *CopyString(const String &str) {
		size_t len;
		if (str.len == 0)
			len = fplGetStringLength(str.base);
		else
			len = str.len - 1;
		const char *result = CopyString(str.base, len);
		return(result);
	}

	void ReleaseAll() {
		Entry *p = first;
		while (p != nullptr) {
			Entry *n = p->next;
			ReleaseEntry(p);
			p = n;
		}
	}
};

// We just support a couple of fonts, built-in.
enum class FontResourceType {
	Debug = 0,
	Arimo,
	SulphurPoint,
	BitStreamVerySans,
};

struct FontResource {
	const uint8_t *data;
	const char *name;
	FontResourceType type;
};

namespace FontResources {
	static FontResource Debug = { bitstreamVerySansFontData, "Debug", FontResourceType::Debug };
	static FontResource Arimo = { arimoRegularFontData, "Arimo", FontResourceType::Arimo };
	static FontResource SulphurPoint = { sulphurPointRegularData, "Sulphur Point", FontResourceType::SulphurPoint };
	static FontResource BitStreamVerySans = { bitstreamVerySansFontData, "Bitstream Vera Sans", FontResourceType::BitStreamVerySans };
};

struct FontID {
	const char *name;

	static FontID Make(StringTable &table, const char *name) {
		const char *nameCopy = table.CopyString(name); // @TODO(final): Intern the name, so we can just do a ptr match
		FontID result = { nameCopy };
		return(result);
	}
};

struct LoadedFont {
	struct Glyph {
		// Pixel coordinates: TR, TL, BL, BR ([0, W] [0, 0] [0, H] [W, H])
		Vec2f uv[4]; // In range of 0.0 to 1.0
		Vec2f offset[4]; // In range of -1.0 to 1.0
		float advance; // In range of -1.0 to 1.0
		uint32_t codePoint; // The unicode codepoint
	};

	FontID id;
	Glyph *glyphs;
	uint32_t minChar;
	uint32_t maxChar;
	uint32_t bitmapWidth;
	uint32_t bitmapHeight;
	GLuint textureId;
	float fontSize;
	float ascent;
	float descent;

	static b32 LoadFromMemory(LoadedFont *outFont, const uint8_t *fontData, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize = 256, const uint32_t maxBitmapSize = 8192) {
		uint32_t charCount = (maxChar - minChar) + 1;

		int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
		if (fontOffset < 0) {
			return(false);
		}

		stbtt_fontinfo fontInfo;
		if (!stbtt_InitFont(&fontInfo, fontData, fontOffset)) {
			return(false);
		}

		float pixelScale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

		stbtt_packedchar *packedChars = (stbtt_packedchar *)malloc(sizeof(stbtt_packedchar) * charCount);

		uint32_t bitmapSize = minBitmapSize;
		uint8_t *fontBitmap = nullptr;

		// @TODO(final): Support for multiple textures / codepoint-ranges, instead of finding the biggest bitmap

		// Find bitmap size where every character fits in
		b32 isPacked = false;
		do {
			fontBitmap = (uint8_t *)malloc(bitmapSize * bitmapSize);

			stbtt_pack_context context;
			stbtt_PackBegin(&context, fontBitmap, bitmapSize, bitmapSize, 0, 1, nullptr);

			int oversampleX = 2, oversampleY = 2;
			stbtt_PackSetOversampling(&context, oversampleX, oversampleY);

			if (stbtt_PackFontRange(&context, fontData, 0, fontSize, minChar, charCount, packedChars)) {
				isPacked = true;
			} else {
				free(fontBitmap);
				bitmapSize *= 2;
			}

			stbtt_PackEnd(&context);
		} while (!isPacked && (bitmapSize < maxBitmapSize));

		if (fontBitmap == nullptr) {
			free(packedChars);
			return(false);
		}

		float invAtlasW = 1.0f / (float)bitmapSize;
		float invAtlasH = 1.0f / (float)bitmapSize;

		float fontScale = 1.0f / fontSize;

		Glyph *glyphs = (Glyph *)fplMemoryAllocate(sizeof(Glyph) * charCount);

		for (uint32_t charIndex = 0; charIndex < charCount; ++charIndex) {
			const stbtt_packedchar *b = packedChars + charIndex;

			Glyph *outGlyph = glyphs + charIndex;

			outGlyph->codePoint = minChar + charIndex;

			float s0 = b->x0 * invAtlasW;
			float t0 = b->y0 * invAtlasH;
			float s1 = b->x1 * invAtlasW;
			float t1 = b->y1 * invAtlasH;

			float x0 = b->xoff * fontScale;
			float y0 = b->yoff * fontScale;
			float x1 = b->xoff2 * fontScale;
			float y1 = b->yoff2 * fontScale;

			outGlyph->offset[0] = V2f(x1, y0); // Top-right
			outGlyph->offset[1] = V2f(x0, y0); // Top-left
			outGlyph->offset[2] = V2f(x0, y1); // Bottom-left
			outGlyph->offset[3] = V2f(x1, y1); // Bottom-right

			outGlyph->uv[0] = V2f(s1, t0);
			outGlyph->uv[1] = V2f(s0, t0);
			outGlyph->uv[2] = V2f(s0, t1);
			outGlyph->uv[3] = V2f(s1, t1);

			outGlyph->advance = b->xadvance * fontScale;
		}

		free(packedChars);

		GLuint fontTexture;
		glGenTextures(1, &fontTexture);
		glBindTexture(GL_TEXTURE_2D, fontTexture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bitmapSize, bitmapSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fontBitmap);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, bitmapSize.w, bitmapSize.h, 0, GL_RED, GL_UNSIGNED_BYTE, fontBitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		CheckGLError();

		free(fontBitmap);

		LoadedFont font = fplZeroInit;
		font.fontSize = fontSize;
		font.glyphs = glyphs;
		font.minChar = minChar;
		font.maxChar = maxChar;
		font.bitmapWidth = bitmapSize;
		font.bitmapHeight = bitmapSize;
		font.textureId = fontTexture;
		font.descent = descent * pixelScale * fontScale;
		font.ascent = ascent * pixelScale * fontScale;

		*outFont = font;

		return(true);
	}

	static b32 LoadFromFile(LoadedFont *outFont, const char *filePath, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize = 256, const uint32_t maxBitmapSize = 8192) {
		fplFileHandle fontFile;
		if (fplOpenBinaryFile(filePath, &fontFile)) {
			uint32_t fileSize = fplGetFileSizeFromHandle32(&fontFile);
			uint8_t *fontData = (uint8_t *)fplMemoryAllocate(fileSize);
			fplReadFileBlock32(&fontFile, fileSize, fontData, fileSize);
			fplCloseFile(&fontFile);
			b32 result = LoadFromMemory(outFont, fontData, fontIndex, fontSize, minChar, maxChar, minBitmapSize, maxBitmapSize);
			fplMemoryFree(fontData);
			return(result);
		} else {
			return(false);
		}
	}

	void Release() {
		if (textureId > 0) {
			glDeleteTextures(1, &textureId);
			textureId = 0;
		}
		if (glyphs != nullptr) {
			fplMemoryFree(glyphs);
		}
	}
};

enum class ImageResourceType {
	FPLLogo128x128 = 0,
};

struct ImageResource {
	const uint8_t *bytes;
	const char *name;
	const size_t length;
	ImageResourceType type;
};

namespace ImageResources {
	static ImageResource FPLLogo128x128 = { fplLogo128x128ImageData, "FPL Logo 128x128", fplLogo128x128ImageDataSize, ImageResourceType::FPLLogo128x128 };
}

struct ImageID {
	const char *name;
	size_t index;

	static ImageID Make(StringTable &table, const char *name, const size_t index) {
		const char *nameCopy = table.CopyString(name);
		ImageID result = { nameCopy, index };
		return(result);
	}
};

struct LoadedImage {
	ImageID id;
	uint32_t width;
	uint32_t height;
	GLuint textureId;

	static b32 LoadFromMemory(LoadedImage *outImage, const uint8_t *bytes, const size_t length) {
		int w, h, comp;
		stbi_uc *pixels = stbi_load_from_memory((const stbi_uc *)bytes, (int)length, &w, &h, &comp, 4);
		if (pixels == nullptr) {
			return(false);
		}

		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		CheckGLError();

		stbi_image_free(pixels);

		LoadedImage result = fplZeroInit;
		result.width = w;
		result.height = h;
		result.textureId = textureId;

		*outImage = result;

		return(true);
	}

	static b32 LoadFromFile(LoadedImage *outImage, const char *filePath) {
		fplFileHandle fontFile;
		if (fplOpenBinaryFile(filePath, &fontFile)) {
			uint32_t fileSize = fplGetFileSizeFromHandle32(&fontFile);
			uint8_t *bytes = (uint8_t *)fplMemoryAllocate(fileSize);
			fplReadFileBlock32(&fontFile, fileSize, bytes, fileSize);
			fplCloseFile(&fontFile);
			b32 result = LoadFromMemory(outImage, bytes, fileSize);
			fplMemoryFree(bytes);
			return(result);
		} else {
			return(false);
		}
	}

	void Release() {
		if (textureId > 0) {
			glDeleteTextures(1, &textureId);
			textureId = 0;
		}
	}
};

constexpr int MaxFontCount = 16;
constexpr int MaxImagesCount = 64;

struct Renderer {
	LoadedFont fonts[MaxFontCount]; // First font is always the debug font
	LoadedImage images[MaxImagesCount];
	const LoadedFont *debugFont;
	StringTable *strings;
	size_t numFonts;
	size_t numImages;

	static int CompareFont(const void *oa, const void *ob) {
		const LoadedFont *a = (const LoadedFont *)oa;
		const LoadedFont *b = (const LoadedFont *)ob;

		int delta;

		// Font name
		delta = strcmp(a->id.name, b->id.name);
		if (delta != 0) {
			return(delta);
		}

		// Font-size
		float fontSizeDelta = a->fontSize - b->fontSize;
		if (fontSizeDelta < 0) delta = -1;
		else if (fontSizeDelta > 0) delta = 1;
		else delta = 0;
		if (delta != 0) {
			return(delta);
		}

		return(0);
	}

	const LoadedFont *AddFontFromResource(const FontResource &resource, const float fontSize, const uint32_t minChar = 32, const uint32_t maxChar = 255) {
		fplAssert(numFonts < fplArrayCount(fonts));
		LoadedFont *font = fonts + numFonts;
		if (!LoadedFont::LoadFromMemory(font, resource.data, 0, fontSize, minChar, maxChar)) {
			return {};
		}
		numFonts++;
		FontID id = FontID::Make(*strings, resource.name);
		font->id = id;
		qsort(fonts, numFonts, sizeof(fonts[0]), CompareFont);
		return(font);
	}

	const LoadedFont *AddFontFromFile(const char *filePath, const char *name, const float fontSize, const uint32_t minChar = 32, const uint32_t maxChar = 255) {
		fplAssert(numFonts < fplArrayCount(fonts));
		LoadedFont *font = fonts + numFonts;
		if (!LoadedFont::LoadFromFile(font, filePath, 0, fontSize, minChar, maxChar)) {
			return {};
		}
		numFonts++;
		FontID id = FontID::Make(*strings, name);
		font->id = id;
		qsort(fonts, numFonts, sizeof(fonts[0]), CompareFont);
		return(font);
	}

	inline const LoadedFont *FindFont(const char *name, const float fontSize = 0.0f) const {
		const LoadedFont *result = nullptr;

		// First: Try to find a font which is equal or greater
		for (size_t fontIndex = 0; fontIndex < numFonts; ++fontIndex) {
			const LoadedFont *font = fonts + fontIndex;
			if ((strcmp(font->id.name, name) == 0) && (fontSize == 0.0f || font->fontSize > fontSize)) {
				result = font;
				break;
			}
		}

		// Second: Find font which is smaller
		if (result == nullptr && numFonts > 0) {
			for (size_t fontIndex = numFonts - 1; fontIndex > 0; fontIndex--) {
				const LoadedFont *font = fonts + fontIndex;
				if ((strcmp(font->id.name, name) == 0) && font->fontSize <= fontSize) {
					result = font;
				}
			}
		}

		return(result);
	}

	const LoadedImage *AddImageFromResource(const ImageResource &resource) {
		fplAssert(numImages < fplArrayCount(images));
		LoadedImage *image = images + numImages;
		if (!LoadedImage::LoadFromMemory(image, resource.bytes, resource.length)) {
			return {};
		}
		ImageID id = ImageID::Make(*strings, resource.name, numImages++);
		image->id = id;
		return(image);
	}

	const LoadedImage *AddImageFromFile(const char *filePath) {
		fplAssert(numImages < fplArrayCount(images));
		LoadedImage *image = images + numImages;
		if (!LoadedImage::LoadFromFile(image, filePath)) {
			return {};
		}
		ImageID id = ImageID::Make(*strings, filePath, numImages++);
		image->id = id;
		return(image);
	}

	const LoadedImage *FindImage(const char *name) const {
		const LoadedImage *result = nullptr;

		for (size_t imageIndex = 0; imageIndex < numImages; ++imageIndex) {
			const LoadedImage *image = images + imageIndex;
			if (strcmp(image->id.name, name) == 0) {
				result = image;
				break;
			}
		}

		return(result);
	}
};

typedef float(EasingFunction)(const float x);

struct Easing {
	EasingFunction *func;
};

// Based on https://easings.net
namespace Easings {
	static Easing Linear = { [](float x) { return x; } };

	static Easing EaseInSine = { [](float x) { return 1.0f - Cosine((x * Pi32) * 0.5f); } };
	static Easing EaseOutSine = { [](float x) { return Sine((x * Pi32) * 0.5f); } };
	static Easing EaseInOutSine = { [](float x) { return -(Cosine(Pi32 * x) - 1.0f) * 0.5f; } };

	static Easing EaseInQuad = { [](float x) { return x * x; } };
	static Easing EaseOutQuad = { [](float x) { return 1.0f - (1.0f - x) * (1.0f - x); } };
	static Easing EaseInOutQuad = { [](float x) { return x < 0.5f ? 2.0f * x * x : 1.0f - Power(-2.0f * x + 2.0f, 2.0f) * 0.5f; } };

	static Easing EaseInCube = { [](float x) { return x * x * x; } };
	static Easing EaseOutCube = { [](float x) { return 1.0f - Power(1.0f - x, 3.0f); } };
	static Easing EaseInOutCube = { [](float x) { return x < 0.5f ? 4.0f * x * x * x : 1.0f - Power(-2.0f * x + 2.0f, 3.0f) * 0.5f; } };

	static Easing EaseInQuart = { [](float x) { return x * x * x * x; } };
	static Easing EaseOutQuart = { [](float x) { return 1.0f - Power(1.0f - x, 4.0f); } };
	static Easing EaseInOutQuart = { [](float x) { return x < 0.5f ? 8.0f * x * x * x * x : 1.0f - Power(-2.0f * x + 2.0f, 4.0f) * 0.5f; } };

	static Easing EaseInQuint = { [](float x) { return x * x * x * x * x; } };
	static Easing EaseOutQuint = { [](float x) { return 1.0f - Power(1.0f - x, 5.0f); } };
	static Easing EaseInOutQuint = { [](float x) { return x < 0.5f ? 16.0f * x * x * x * x * x : 1.0f - Power(-2.0f * x + 2.0f, 5.0f) * 0.5f; } };

	static Easing EaseInExpo = { [](float x) { return x == 0.0f ? 0 : Power(2.0f, 10.0f * x - 10.0f); } };
	static Easing EaseOutExpo = { [](float x) { return x == 1.0f ? 1.0f : 1.0f - Power(2.0f, -10.0f * x); } };
	static Easing EaseInOutExpo = { [](float x) {
		return x == 0.0f
		  ? 0.0f
		  : x == 1.0f
		  ? 1.0f
		  : x < 0.5f ? Power(2.0f, 20.0f * x - 10.0f) * 0.5f
		  : (2.0f - Power(2.0f, -20.0f * x + 10.0f)) * 0.5f;
	} };

	static Easing EaseInCircle = { [](float x) { return 1.0f - SquareRoot(1.0f - Power(x, 2.0f)); } };
	static Easing EaseOutCircle = { [](float x) { return SquareRoot(1.0f - Power(x - 1.0f, 2.0f)); } };
	static Easing EaseInOutCircle = { [](float x) {
		return x < 0.5f
		  ? (1.0f - SquareRoot(1.0f - Power(2.0f * x, 2.0f))) * 0.5f
		  : (SquareRoot(1.0f - Power(-2.0f * x + 2.0f, 2.0f)) + 1.0f) * 0.5f;
	} };
};

enum class AnimationState: int32_t {
	Stopped = 0,
	Running,
	Done,
};

struct Animation {
	Easing easing;
	float duration;
	float currentTime;
	float currentAlpha;
	float startAlpha;
	float targetAlpha;
	AnimationState state;
	b32 reverse;

	b32 IsActive() const {
		b32 result = state == AnimationState::Running;
		return(result);
	}

	void RunToggle(const float maxDuration, const Easing &easing) {
		float ta;
		if (!reverse) {
			ta = 0.0f;
		} else {
			ta = 1.0f;
		}
		RunTo(ta, maxDuration, easing);
	}

	void RunTo(const float targetAlpha, const float maxDuration, const Easing &easing) {
		this->targetAlpha = targetAlpha;
		this->startAlpha = currentAlpha;

		float span;
		if (targetAlpha < currentAlpha) {
			reverse = true;
			span = currentAlpha - targetAlpha;
		} else {
			reverse = false;
			span = targetAlpha - currentAlpha;
		}

		this->duration = maxDuration * span;
		this->currentTime = 0.0f;

		this->state = AnimationState::Running;
	}

	void Stop() {
		this->state = AnimationState::Stopped;
	}

	void ResetAndStart(const float duration, const b32 reverse, const Easing &easing) {
		this->easing = easing;

		this->duration = duration;
		this->currentTime = 0.0f;

		this->currentAlpha = reverse ? 1.0f : 0.0f;
		this->startAlpha = this->currentAlpha;
		this->targetAlpha = reverse ? 0.0f : 1.0f;

		this->state = AnimationState::Running;
		this->reverse = reverse;
	}

	void Update(const float dt) {
		switch (state) {
			case AnimationState::Stopped:
				break;

			case AnimationState::Running:
			{
				currentTime += dt;
				float t = fplMin(currentTime, duration) / duration;
				currentAlpha = easing.func(ScalarLerp(startAlpha, t, targetAlpha));
				if (currentTime >= duration) {
					currentTime = duration;
					currentAlpha = easing.func(targetAlpha);
					state = AnimationState::Done;
				}
			} break;
		}
	}
};

enum class BackgroundKind {
	None = 0,
	Solid,
	GradientHorizontal,
	GradientVertical,
};

struct Background {
	Vec4f primaryColor;
	Vec4f secondaryColor;
	BackgroundKind kind;
};

struct LabelStyle {
	Background background;
	Vec4f foregroundColor;
	Vec4f shadowColor;
	Vec2f shadowOffset;
	b32 drawShadow;
};

struct Label {
	LabelStyle style;
	Vec2f pos;
	const char *fontName;
	const char *text;
	float fontSize;
	HorizontalAlignment hAlign;
	VerticalAlignment vAlign;
};

struct RectStyle {
	Background background;
};

struct Rect {
	RectStyle style;
	Vec2f pos;
	Vec2f size;
};

struct ImageStyle {
	Background background;
};

struct Image {
	ImageStyle style;
	Vec2f pos;
	Vec2f size;
	const char *imageName;
};

enum class ElementType {
	None = 0,
	Label,
	Rect,
	Image,
};

struct Element {
	union {
		Label label;
		Rect rect;
		Image image;
	};
	ElementType type;
};

struct SlideVariables {
	uint32_t slideNum;
	uint32_t slideCount;
};

struct Slide {
	LinkedList<Element> elements;
	SlideVariables vars;
	Vec4f backgroundColor;
	Vec2f size;
	StringTable *strings;
	const char *name;
	size_t numElements;

	Element *AddElement(const ElementType type) {
		Element *result = elements.Add();
		result->type = type;
		return(result);
	}

	Label *AddLabel(const String &text, const Vec2f &pos, const char *fontName, const float fontSize, const HorizontalAlignment hAlign = HorizontalAlignment::Left, const VerticalAlignment vAlign = VerticalAlignment::Top, const LabelStyle &style = {}) {
		Element *element = AddElement(ElementType::Label);
		Label *result = &element->label;
		result->pos = pos;
		result->fontName = strings->CopyString(fontName);
		result->fontSize = fontSize;
		result->hAlign = hAlign;
		result->vAlign = vAlign;
		result->text = strings->CopyString(text);
		result->style = style;
		return(result);
	}

	Rect *AddRect(const Vec2f &pos, const Vec2f &size) {
		Element *element = AddElement(ElementType::Rect);
		Rect *result = &element->rect;
		result->pos = pos;
		result->size = size;
		return(result);
	}

	Image *AddImage(const Vec2f &pos, const Vec2f &size, const char *imageName) {
		Element *element = AddElement(ElementType::Image);
		Image *result = &element->image;
		result->pos = pos;
		result->size = size;
		result->imageName = strings->CopyString(imageName);
		return(result);
	}

	void Release() {
		elements.Release();
	}
};

struct Presentation {
	LinkedList<Slide> slides;
	Vec2f size;
	StringTable *strings;

	Slide *AddSlide(const Vec2f &size, const char *name) {
		Slide *result = slides.Add();
		result->strings = strings;
		result->size = size;
		result->name = strings->CopyString(name);
		return(result);
	}

	void Release() {
		auto it = slides.GetIterator();
		for (Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext())
			slide->Release();
		slides.Release();
	}
};

struct SlideTemplate {
	Vec2f size;
	const char *name;
};

struct PresentationTemplate {
	LinkedList<SlideTemplate> slides;
	Vec2f size;

	SlideTemplate *AddSlide(const SlideTemplate &slideTemplate) {
		SlideTemplate *result = slides.Add();
		*result = slideTemplate;
		return(result);
	}

	void Release() {
		slides.Release();
	}
};

struct PresentationState {
	Animation slideAnimation;
	Vec2f startOffset;
	Vec2f currentOffset;
	Vec2f targetOffset;
	Slide *activeSlide;
	int32_t activeSlideIndex;
};

struct App {
	Presentation presentation;
	PresentationState state;
	Renderer renderer;
	StringTable strings;
};

static Vec2f ComputeTextSize(const LoadedFont &font, const char *text, const size_t textLen, const float charHeight) {
	Vec2f result = fplZeroInit;
	if (text != nullptr) {
		float totalWidth = 0.0f;
		const char *p = text;
		while (*p) {
			size_t n = (p - text) + 1;
			if (n > textLen) break;
			uint32_t codePoint = (unsigned char)*p;
			if (codePoint >= font.minChar && codePoint <= font.maxChar) {
				uint32_t charIndex = codePoint - font.minChar;
				const LoadedFont::Glyph *glyph = font.glyphs + charIndex;
				Vec2f verts[] = {
					glyph->offset[0] * charHeight,
					glyph->offset[1] * charHeight,
					glyph->offset[2] * charHeight,
					glyph->offset[3] * charHeight,
				};
				Vec2f min = glyph->offset[0];
				Vec2f max = glyph->offset[0];
				for (int i = 1; i < 4; ++i) {
					min = V2fMin(min, verts[i]);
					max = V2fMax(max, verts[i]);
				}
				totalWidth += glyph->advance * charHeight;
			}
			++p;
		}
		result.x = totalWidth;
		result.y = charHeight;
	}
	return(result);
}

static Vec2f ComputeBoxOffset(const Vec2f &size, const HorizontalAlignment horizonzalAlign = HorizontalAlignment::Left, const VerticalAlignment verticalAlign = VerticalAlignment::Top) {
	Vec2f result = V2f(0, 0);
	if (verticalAlign == VerticalAlignment::Bottom) {
		result += V2f(0, -size.h);
	} else 	if (verticalAlign == VerticalAlignment::Middle) {
		result += V2f(0, -size.h * 0.5f);
	}
	if (horizonzalAlign == HorizontalAlignment::Right) {
		result += V2f(-size.w, 0);
	} else 	if (horizonzalAlign == HorizontalAlignment::Center) {
		result += V2f(-size.w * 0.5f, 0);
	}
	return(result);
}

static Vec2f ComputeTextOffset(const LoadedFont &font, const char *text, const size_t textLen, const float charHeight, const HorizontalAlignment horizonzalAlign = HorizontalAlignment::Left, const VerticalAlignment verticalAlign = VerticalAlignment::Top) {
	Vec2f size = ComputeTextSize(font, text, textLen, charHeight);
	Vec2f result = ComputeBoxOffset(size, horizonzalAlign, verticalAlign);
	return(result);
}

static void RenderTextQuads(const float x, const float y, const char *text, const size_t textLen, const float charHeight, const LoadedFont &font, const Vec4f &color) {
	if (text != nullptr) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, font.textureId);
		glColor4fv(&color.m[0]);
		glBegin(GL_QUADS);
		const char *p = text;
		Vec2f d = V2f(0, 0);
		Vec2f pos = V2f(x, y) + d;
		float scale = charHeight;
		while (*p) {
			uint32_t codePoint = (unsigned char)*p;
			if (codePoint >= font.minChar && codePoint <= font.maxChar) {
				uint32_t charIndex = codePoint - font.minChar;
				const LoadedFont::Glyph *glyph = font.glyphs + charIndex;

				Vec2f v0 = pos + glyph->offset[0] * scale;
				Vec2f v1 = pos + glyph->offset[1] * scale;
				Vec2f v2 = pos + glyph->offset[2] * scale;
				Vec2f v3 = pos + glyph->offset[3] * scale;

				glTexCoord2fv(&glyph->uv[0].m[0]); glVertex2fv(&v0.m[0]);
				glTexCoord2fv(&glyph->uv[1].m[0]); glVertex2fv(&v1.m[0]);
				glTexCoord2fv(&glyph->uv[2].m[0]); glVertex2fv(&v2.m[0]);
				glTexCoord2fv(&glyph->uv[3].m[0]); glVertex2fv(&v3.m[0]);

				pos += V2f(glyph->advance * scale, 0);
			}
			++p;
		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

static void RenderLine(const Vec2f &a, const Vec2f &b, const Vec4f &color, const float lineWidth = 1.0f) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();
	glLineWidth(1.0f);
}

static void RenderFilledQuad(const Vec2f &pos, const Vec2f &size, const Vec4f &color0, const Vec4f &color1, const BackgroundKind kind) {
	glBegin(GL_QUADS);
	if (kind == BackgroundKind::GradientHorizontal) {
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	} else if (kind == BackgroundKind::GradientVertical) {
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color1.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	} else {
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x, pos.y + size.h);
		glColor4fv(&color0.m[0]); glVertex2f(pos.x + size.w, pos.y + size.h);
	}
	glEnd();
}

static void RenderFilledQuad(const Vec2f &pos, const Vec2f &size, const Vec4f &color) {
	RenderFilledQuad(pos, size, color, V4fInit(0, 0, 0, 0), BackgroundKind::Solid);
}

static void RenderStrokedQuad(const Vec2f &pos, const Vec2f &size, const Vec4f &color, const float lineWidth = 1.0f) {
	glLineWidth(lineWidth);
	glColor4fv(&color.m[0]);
	glBegin(GL_LINE_LOOP);
	glVertex2f(pos.x + size.w, pos.y);
	glVertex2f(pos.x, pos.y);
	glVertex2f(pos.x, pos.y + size.h);
	glVertex2f(pos.x + size.w, pos.y + size.h);
	glEnd();
	glLineWidth(1.0f);
}

static const char *ResolveText(const SlideVariables &vars, const char *source, char *buffer, size_t maxBufferLen) {
	buffer[0] = 0;
	const char *result = buffer;
	const char *s = source;
	if (s != nullptr) {
		size_t bufIndex = 0;
		while (*s) {
			char c = *s;
			if (c == '%') {
				++s;
				size_t varLen = 0;
				const char *varName = s;
				while (*s && *s != '%') {
					++varLen;
					++s;
				}
				if (*s == '%') {
					++s;
					// % Escape
					if (varLen == 0) {
						buffer[bufIndex++] = '%';
					} else {
						size_t remainingBufLen = maxBufferLen - bufIndex;
						char *remainingStart = &buffer[bufIndex];
						if (strncmp("SLIDE_NUM", varName, varLen) == 0) {
							char *t = fplS32ToString(vars.slideNum, remainingStart, remainingBufLen);
							if (t != nullptr) {
								size_t addonChars = fplGetStringLength(remainingStart);
								bufIndex += addonChars;
							}
						} else if (strncmp("SLIDE_COUNT", varName, varLen) == 0) {
							char *t = fplS32ToString(vars.slideCount, remainingStart, remainingBufLen);
							if (t != nullptr) {
								size_t addonChars = fplGetStringLength(remainingStart);
								bufIndex += addonChars;
							}
						}
					}
				} else {
					// Unterminated
					break;
				}
			} else {
				buffer[bufIndex++] = c;
				++s;
			}
		}
		buffer[bufIndex] = 0;
	}
	return(result);
}

static void RenderBackground(const Vec2f &pos, const Vec2f &size, const Background &background) {
	if (background.kind != BackgroundKind::None) {
		RenderFilledQuad(pos, size, background.primaryColor, background.secondaryColor, background.kind);
	}
}

static void RenderLabel(const LoadedFont &font, const Label &label, const SlideVariables &vars) {
	static char tmpBuffer[4096]; // @REPLACE(tspaete): Not great using a static buffer here, find a better approach

	const LabelStyle &style = label.style;
	const char *text = ResolveText(vars, label.text, tmpBuffer, fplArrayCount(tmpBuffer));
	float charHeight = label.fontSize;
	size_t textLen = fplGetStringLength(label.text);
	Vec2f pos = label.pos;
	Vec2f size = ComputeTextSize(font, text, textLen, charHeight);
	Vec2f align = ComputeBoxOffset(size, label.hAlign, label.vAlign);
	Vec2f boxPos = pos + align;
	Vec2f textPos = boxPos + V2f(0, font.ascent * charHeight);

	// Background
	RenderBackground(boxPos, size, style.background);

	// Shadow
	if (style.drawShadow) {
		RenderTextQuads(textPos.x + style.shadowOffset.x, textPos.y + style.shadowOffset.y, text, textLen, charHeight, font, style.shadowColor);
	}

	// Foreground
	RenderTextQuads(textPos.x, textPos.y, text, textLen, charHeight, font, style.foregroundColor);

#if DRAW_TEXT_BOUNDS
	// Draw bounds
	RenderStrokedQuad(boxPos, size, V4fInit(1, 0, 0, 1), 1.0f);

	// Draw baseline
	Vec2f baseline = boxPos + V2f(0, size.h + font.descent * charHeight);
	Vec2f ascent = baseline + V2f(0, -font.ascent * charHeight);
	RenderLine(baseline, baseline + V2f(size.w, 0), V4fInit(0, 1, 0, 1), 2.0f);
	RenderLine(ascent, ascent + V2f(size.w, 0), V4fInit(0, 0, 1, 1), 2.0f);
#endif
}

static void RenderImageQuad(const LoadedImage &renderImage, const Vec2f &pos, const Vec2f &size, const Vec4f &color) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, renderImage.textureId);
	glColor4fv(&color.m[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(pos.x + size.w, pos.y);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(pos.x, pos.y);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(pos.x, pos.y + size.h);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(pos.x + size.w, pos.y + size.h);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

static void RenderImage(const LoadedImage &renderImage, const Image &image) {
	Vec2f pos = image.pos;
	Vec2f size = image.size;
	Vec2f align = ComputeBoxOffset(size, HorizontalAlignment::Left, VerticalAlignment::Top);
	Vec2f boxPos = pos + align;
	Vec2f imagePos = boxPos;
	const ImageStyle &style = image.style;

	// Background
	RenderBackground(boxPos, size, style.background);

	// Foreground
	RenderImageQuad(renderImage, imagePos, size, V4f(1, 1, 1, 1));

#if DRAW_IMAGE_BOUNDS
	// Draw bounds
	RenderStrokedQuad(boxPos, size, V4fInit(1, 0, 0, 1), 1.0f);
#endif
}

struct Viewport {
	int x, y;
	int w, h;
};

extern Viewport ComputeViewportByAspect(const Vec2i &screenSize, const float targetAspect) {
	int targetHeight = (int)(screenSize.w / targetAspect);
	Vec2i viewSize = V2iInit(screenSize.w, screenSize.h);
	Vec2i viewOffset = V2iInit(0, 0);
	if (targetHeight > screenSize.h) {
		viewSize.h = screenSize.h;
		viewSize.w = (int)(screenSize.h * targetAspect);
		viewOffset.x = (screenSize.w - viewSize.w) / 2;
	} else {
		viewSize.w = screenSize.w;
		viewSize.h = (int)(screenSize.w / targetAspect);
		viewOffset.y = (screenSize.h - viewSize.h) / 2;
	}
	Viewport result = { viewOffset.x, viewOffset.y, viewSize.w, viewSize.h };
	return(result);
}

static void UpdateFrame(App &app, const float dt) {
	PresentationState &state = app.state;
	Animation &anim = state.slideAnimation;
	anim.Update(dt);

	if (state.slideAnimation.IsActive()) {
		state.currentOffset = V2fLerp(state.startOffset, state.slideAnimation.currentAlpha, state.targetOffset);
	} else {
		state.currentOffset = state.targetOffset;
	}
}

static void RenderSlide(const Slide &slide, const Renderer &renderer) {
	float w = slide.size.w;
	float h = slide.size.h;
	Vec2f radius = V2f(w, h) * 0.5f;
	Vec2f center = radius;

	// CornerFlowerBlue = RGBAToLinearRaw(0x64, 0x95, 0xED, 0xFF)

	RenderFilledQuad(V2f(0, 0), V2f(w, h), slide.backgroundColor);

#if DRAW_SLIDE_CENTER
	RenderLine(center - V2f(radius.w, 0), center + V2f(radius.w, 0), V4fInit(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
	RenderLine(center - V2f(0, radius.h), center + V2f(0, radius.h), V4fInit(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
#endif

	auto it = slide.elements.GetConstIterator();
	for (const Element *element = it.Value(); it.HasNext(); element = it.MoveNext()) {
		switch (element->type) {
			case ElementType::Rect:
			{
				const Rect &rect = element->rect;
				RenderBackground(rect.pos, rect.size, rect.style.background);
			} break;

			case ElementType::Label:
			{
				const Label &label = element->label;
				const char *fontName = label.fontName;
				const LoadedFont *font = renderer.FindFont(fontName, label.fontSize);
				if (font != nullptr) {
					RenderLabel(*font, label, slide.vars);
				}
			} break;

			case ElementType::Image:
			{
				const Image &image = element->image;
				const char *imageName = image.imageName;
				const LoadedImage *renderImage = renderer.FindImage(imageName);
				if (renderImage != nullptr) {
					RenderImage(*renderImage, image);
				}
			} break;
		}
	}
}

static void RenderFrame(const App &app, const Vec2i &winSize) {
	const PresentationState &state = app.state;
	const Presentation &presentation = app.presentation;
	const Renderer &renderer = app.renderer;

	const LoadedFont *debugFont = app.renderer.debugFont;
	fplAssert(debugFont != nullptr);
	const float debugFontSize = 30.0f;

#if 1
	const Slide *activeSlide = state.activeSlide;
	if (activeSlide == nullptr) {
		float w = 1280.0;
		float h = 720.0f;

		glViewport(0, 0, winSize.w, winSize.h);

		Mat4f proj = Mat4OrthoRH(0.0f, w, h, 0.0f, -1.0f, 1.0f);
		glLoadMatrixf(&proj.m[0]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const char *text = "No slide found!";
		size_t textLen = fplGetStringLength(text);
		Vec2f size = ComputeTextSize(*debugFont, text, textLen, debugFontSize);
		Vec2f offset = ComputeBoxOffset(size, HorizontalAlignment::Center, VerticalAlignment::Middle);
		RenderTextQuads(w * 0.5f + offset.x, h * 0.5f + offset.y, text, textLen, debugFontSize, *debugFont, V4f(1, 0, 0, 1));
	} else {
		float w = activeSlide->size.w;
		float h = activeSlide->size.h;
		Vec2f center = V2f(w, h) * 0.5f;
		Mat4f proj = Mat4OrthoRH(0.0f, w, h, 0.0f, -1.0f, 1.0f);

		float aspect = w / h;
		Viewport viewport = ComputeViewportByAspect(winSize, aspect);
		glViewport(0, 0, winSize.w, winSize.h);
		glScissor(0, 0, winSize.w, winSize.h);
		//glViewport(viewport.x, viewport.y, viewport.w, viewport.h);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float zoom = 1.0f;
		Mat4f view = Mat4TranslationV2(V2f(w * 0.5f, h * 0.5f)) * Mat4ScaleV2(V2f(zoom, zoom));
		Vec2f zoomOffset = V2f(-w * 0.5f, -h * 0.5f);

		Vec2f slidePos = V2f(0, 0);
		auto it = presentation.slides.GetConstIterator();
		for (const Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
			Mat4f slideModel = Mat4TranslationV2(slidePos - state.currentOffset + zoomOffset);
			Mat4f slideMVP = proj * view * slideModel;
			glLoadMatrixf(&slideMVP.m[0]);
			RenderSlide(*slide, renderer);
			slidePos += V2f(slide->size.w, 0);
		}

#if DRAW_VIEW_CENTER
		glLoadMatrixf(&proj.m[0]);
		RenderLine(center + V2f(-w * 0.25f, 0), center + V2f(w * 0.25f, 0), V4f(1, 1, 1, 1));
		RenderLine(center + V2f(0, -h * 0.25f), center + V2f(0, h * 0.25f), V4f(1, 1, 1, 1));
#endif

	}
#endif

	CheckGLError();
	glFlush();
}

static void ReleaseApp(App &app) {
	app.presentation.Release();
	for (size_t imageIndex = 0; imageIndex < app.renderer.numImages; ++imageIndex) {
		LoadedImage *image = app.renderer.images + imageIndex;
		image->Release();
	}
	for (size_t fontIndex = 0; fontIndex < app.renderer.numFonts; ++fontIndex) {
		LoadedFont *font = app.renderer.fonts + fontIndex;
		font->Release();
	}
	app.strings.ReleaseAll();
}

static Rect2f AddHeaderAndFooter(Slide *slide, const char *normalFont, const float fontSize) {
	float w = slide->size.w;
	float h = slide->size.h;

	Vec2f logoSize = V2f(32, 32);

	float headerHeight = 24;
	Vec2f headerPadding = V2f(2, 2);

	LabelStyle lblStyle = {};
	lblStyle.shadowColor = V4fInit(0, 0, 0, 1);
	lblStyle.shadowOffset = V2f(1, 1);
	lblStyle.drawShadow = true;
	lblStyle.foregroundColor = V4fInit(1, 1, 1, 1);

	Rect *rectTop = slide->AddRect(V2f(0, 0), V2f(w, headerHeight));
	rectTop->style.background.primaryColor = RGBAToLinearRaw(119, 113, 197, 255);
	rectTop->style.background.secondaryColor = RGBAToLinearRaw(0, 0, 0, 255);
	rectTop->style.background.kind = BackgroundKind::GradientVertical;

	Label *fplLabelTop = slide->AddLabel("Final-Platform-Layer", rectTop->pos + headerPadding, normalFont, fontSize, HorizontalAlignment::Left, VerticalAlignment::Top, lblStyle);

	Image *fplLogo = slide->AddImage(rectTop->pos + V2f(w - logoSize.w, 0), logoSize, ImageResources::FPLLogo128x128.name);

	Rect *rectBottom = slide->AddRect(V2f(0, h - headerHeight), V2f(w, headerHeight));
	rectBottom->style.background.primaryColor = RGBAToLinearRaw(0, 0, 0, 255);
	rectBottom->style.background.secondaryColor = RGBAToLinearRaw(119, 113, 197, 255);
	rectBottom->style.background.kind = BackgroundKind::GradientVertical;

	Label *fplLabelBottomLeft = slide->AddLabel(slide->name, rectBottom->pos + V2f(headerPadding.x, rectBottom->size.h - headerPadding.y), normalFont, fontSize, HorizontalAlignment::Left, VerticalAlignment::Bottom, lblStyle);

	Label *fplLabelBottomRight = slide->AddLabel("Page %SLIDE_NUM% of %SLIDE_COUNT%", rectBottom->pos + V2f(w - headerPadding.x, rectBottom->size.h - headerPadding.y), normalFont, fontSize, HorizontalAlignment::Right, VerticalAlignment::Bottom, lblStyle);

	Rect2f result = R2fInit(V2f(0, headerHeight), V2f(w, h - headerHeight * 2));
	return(result);
}

static void UpdateSlideVariables(const Presentation &presentation, Slide &slide, const uint32_t slideNum) {
	slide.vars = {};
	slide.vars.slideCount = (uint32_t)presentation.slides.Count();
	slide.vars.slideNum = slideNum;
}

static void UpdatePresentationVariables(Presentation &presentation) {
	auto it = presentation.slides.GetIterator();
	for (Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
		UpdateSlideVariables(presentation, *slide, (uint32_t)(it.index + 1));
	}
}

static Vec2f GetSlidePositionForSlide(const Presentation &presentation, const uint32_t slideIndex, const Vec2f direction) {
	Vec2f result = V2f(0, 0);
	auto it = presentation.slides.GetConstIterator();
	for (const Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
		if (it.index == slideIndex) {
			break;
		}
		result += direction * V2fDot(slide->size, direction);
	}
	return(result);
}

static Slide *GetSlideFromIndex(Presentation &presentation, const uint32_t slideIndex) {
	Slide *result = nullptr;
	auto it = presentation.slides.GetIterator();
	for (Slide *slide = it.Value(); it.HasNext(); slide = it.MoveNext()) {
		if (it.index == slideIndex) {
			return(slide);
		}
	}
	return(nullptr);
}

static void ShowSlideshow(App &app, const uint32_t slideIndex, const bool withTransition) {
	size_t slideCount = app.presentation.slides.Count();
	if (slideCount > 0 && slideIndex < slideCount) {
		Slide *slide = GetSlideFromIndex(app.presentation, slideIndex);
		app.state.activeSlideIndex = slideIndex;
		app.state.activeSlide = slide;

		// Offset starts at the left of the first slide and goes to the middle to the left of the last slide
		Vec2f moveDir = V2f(1, 0);
		Vec2f targetSlidePos = GetSlidePositionForSlide(app.presentation, slideIndex, moveDir);

		if (withTransition) {
			app.state.targetOffset = targetSlidePos;
			app.state.startOffset = app.state.currentOffset;
			float duration;
			const float maxDuration = 1.0f;
			if (app.state.slideAnimation.IsActive()) {
				float remaining = fplMax(0, app.state.slideAnimation.duration - app.state.slideAnimation.currentTime);
				duration = fplMax(0, fplMin(maxDuration - remaining, maxDuration));
			} else {
				duration = maxDuration;
			}
			app.state.slideAnimation.ResetAndStart(duration, false, Easings::EaseInOutExpo);
		} else {
			app.state.slideAnimation.Stop();
			app.state.targetOffset = app.state.currentOffset = app.state.startOffset = targetSlidePos;
		}
	}
}

static void JumpToNextSlide(App &app) {
	PresentationState &state = app.state;
	size_t slideCount = app.presentation.slides.Count();
	if (slideCount > 0 && state.activeSlideIndex < (int32_t)(slideCount - 1)) {
		ShowSlideshow(app, state.activeSlideIndex + 1, true);
	}
}

static void JumpToPrevSlide(App &app) {
	PresentationState &state = app.state;
	size_t slideCount = app.presentation.slides.Count();
	if (slideCount > 0 && state.activeSlideIndex > 0) {
		ShowSlideshow(app, state.activeSlideIndex - 1, true);
	}
}

static Vec2f ComputeTextBlockSize(Renderer &renderer, Slide &slide, const char *text, const char *fontName, const float fontSize, const float lineHeight) {
	const LoadedFont *font = renderer.FindFont(fontName, fontSize);
	Vec2f result = V2f(0, 0);
	const char *p = text;
	const char *start = p;
	while (*p) {
		while (*p && *p != '\n') {
			++p;
		}
		const size_t len = p - start;

		Vec2f textSize = ComputeTextSize(*font, start, len, fontSize);
		result += V2f(0, lineHeight);
		result.w = fplMax(result.w, textSize.w);

		if (*p == 0)
			break;
		++p;
		start = p;
	}
	return(result);
}

static void AddTextBlock(Renderer &renderer, Slide &slide, const Vec2f &offset, const char *text, const char *fontName, const float fontSize, const float lineHeight, const LabelStyle &style, const HorizontalAlignment hAlign, const VerticalAlignment vAlign) {
	const LoadedFont *font = renderer.FindFont(fontName, fontSize);
	Vec2f pos = offset;
	const char *p = text;
	const char *start = p;
	while (*p) {
		while (*p && *p != '\n') {
			++p;
		}
		const size_t len = p - start;

		Vec2f textSize = ComputeTextSize(*font, start, len, fontSize);
		slide.AddLabel(String(start, len + 1), pos, fontName, fontSize, hAlign, vAlign, style);
		pos += V2f(0, lineHeight);

		if (*p == 0)
			break;
		++p;
		start = p;
	}
}

struct DefaultSlideSettings {
	LabelStyle normalStyle;
	LabelStyle titleStyle;

	Vec4f backgroundColor;

	const char *headerFontName;
	const char *normalFontName;
	const char *titleFontName;

	float headerFontSize;
	float normalFontSize;
	float titleFontSize;

	float normalLineHeight;
	float titleLineHeight;

	float padding;
};

static void AddSlideFromDefinition(Renderer &renderer, Presentation &presentation, const SlideDefinition &def, const DefaultSlideSettings &settings) {
	Slide *slide = presentation.AddSlide(presentation.size, def.name);
	slide->backgroundColor = settings.backgroundColor;

	Rect2f area = AddHeaderAndFooter(slide, settings.headerFontName, settings.headerFontSize);

	// Title
	slide->AddLabel(slide->name, area.pos + V2f(area.size.w * 0.5f, 0), settings.titleFontName, settings.titleFontSize, HorizontalAlignment::Center, VerticalAlignment::Top, settings.titleStyle);

	// Content
	{
		TextBlockDefinition block = def.content;

		const char *text = block.text;
		Vec2f blockSize = ComputeTextBlockSize(renderer, *slide, text, settings.normalFontName, settings.normalFontSize, settings.normalLineHeight);

		HorizontalAlignment textAlign = HorizontalAlignment::Left;
		VerticalAlignment vAlign = VerticalAlignment::Top;

		Vec2f blockPos = area.pos;
		if (block.hAlign == HorizontalAlignment::Center) {
			textAlign = HorizontalAlignment::Center;
			blockPos += V2f((area.size.w - blockSize.w) * 0.5f, 0);
		} else if (block.hAlign == HorizontalAlignment::Left) {
			blockPos += V2f(settings.padding, 0);
		} else if (block.hAlign == HorizontalAlignment::Right) {
			blockPos -= V2f(settings.padding, 0);
		}
		if (block.vAlign == VerticalAlignment::Middle) {
			blockPos += V2f(0, (area.size.h - blockSize.h) * 0.5f);
		} else if (block.vAlign == VerticalAlignment::Top) {
			blockPos += V2f(0, settings.titleLineHeight);
		}

#if 0
		Rect *rect = slide->AddRect(blockPos, blockSize);
		rect->style.background.kind = BackgroundKind::Solid;
		rect->style.background.primaryColor = V4f(1, 0, 1, 1);
#endif

		Vec2f textPos = blockPos;
		if (textAlign == HorizontalAlignment::Center) {
			textPos += V2fHadamard(V2f(1, 0), blockSize * 0.5f);
		} else if (textAlign == HorizontalAlignment::Right) {
			textPos += V2fHadamard(V2f(1, 0), blockSize);
		}
		AddTextBlock(renderer, *slide, textPos, text, settings.normalFontName, settings.normalFontSize, settings.normalLineHeight, settings.normalStyle, textAlign, vAlign);
	}
}

static void BuildFPLPresentation(Renderer &renderer, Presentation &presentation) {
	float slideWidth = 1280.0f;
	float slideHeight = 720.0f;
	Vec2f slideSize = V2f(slideWidth, slideHeight);
	presentation.size = slideSize;

	const char *headerFont = FontResources::Arimo.name;
	const char *normalFont = FontResources::Arimo.name;
	const char *consoleFont = FontResources::BitStreamVerySans.name;

	const float normalFontSize = 28.0f;
	const float titleFontSize = 50.0f;
	const float headerFontSize = 16.0f;

	const float normalLineScale = 1.25f;
	const float titleLineScale = 1.25f;

	const float normalLineHeight = normalLineScale * normalFontSize;
	const float titleLineHeight = titleLineScale * titleFontSize;

	const Vec4f backColor = V4f(0, 0, 0, 1);
	const Vec4f foreColor = V4f(1, 1, 1, 1);

	const float contentPadding = 20.0f;

	LabelStyle labelStyle = {};
	labelStyle.drawShadow = true;
	labelStyle.shadowColor = V4f(1.0f, 1.0f, 1.0f, 0.2f);
	labelStyle.shadowOffset = V2f(2, 1);
	labelStyle.foregroundColor = foreColor;

	DefaultSlideSettings slideSettings = {};
	slideSettings.backgroundColor = backColor;

	slideSettings.normalFontSize = normalFontSize;
	slideSettings.normalFontName = normalFont;
	slideSettings.normalLineHeight = normalLineHeight;
	slideSettings.normalStyle = labelStyle;
	slideSettings.titleFontSize = titleFontSize;
	slideSettings.titleFontName = normalFont;
	slideSettings.titleLineHeight = titleLineHeight;
	slideSettings.titleStyle = labelStyle;
	slideSettings.headerFontSize = headerFontSize;
	slideSettings.headerFontName = normalFont;
	slideSettings.padding = contentPadding;

#if 1
	size_t slideCount = fplArrayCount(FPLSlides);
	for (size_t slideIndex = 0; slideIndex < slideCount; ++slideIndex) {
		SlideDefinition def = FPLSlides[slideIndex];
		AddSlideFromDefinition(renderer, presentation, def, slideSettings);
	}
#endif

#if 0
	// Intro
	{
		Slide *slide = presentation.AddSlide(presentation.size, "Introduction");
		slide->backgroundColor = backColor;

		Rect2f area = AddHeaderAndFooter(slide, headerFont, headerFontSize);

		Vec2f center = area.pos + area.size * 0.5f;

		Vec2f pos;

		pos = center - V2f(0, normalLineHeight * 3 * 0.5f);
		slide->AddLabel("Introducing Final-Platform-Layer (FPL).", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("A lightweight Platform-Abstraction-Library written in C99.", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
		pos += V2f(0, normalLineHeight * 2);
		slide->AddLabel("Created by Torsten Spaete, a professional software engineer with 20+ years of experience.", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
	}

	// What is a Platform-Abstraction-Library
	{
		Slide *slide = presentation.AddSlide(presentation.size, "What is a Platform-Abstraction-Library");
		slide->backgroundColor = backColor;

		Rect2f area = AddHeaderAndFooter(slide, normalFont, headerFontSize);

		Vec2f topLeft = area.pos;
		Vec2f center = topLeft + area.size * 0.5f;

		Vec2f pos;

		// Title
		pos = topLeft + V2f(area.size.w * 0.5f, 0.0f);
		slide->AddLabel(slide->name, pos, normalFont, titleFontSize, HorizontalAlignment::Center, VerticalAlignment::Top, labelStyle);

		// Content
		pos = center - V2f(0, normalLineHeight * 4 * 0.5f);
		slide->AddLabel("A Platform-Abstraction-Library (or short PAL) is a library written in a low-level language - like C,", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("that abstracts low-level systems in a platform-independent way.", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
		pos += V2f(0, normalLineHeight * 2);
		slide->AddLabel("This has the advantage of not having to deal with tons of platform/compiler specific implementation details,", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("you have to deal with if you don�t use a PAL.", pos, normalFont, normalFontSize, HorizontalAlignment::Center, VerticalAlignment::Middle, labelStyle);
	}

	// What is FPL
	{
		Slide *slide = presentation.AddSlide(presentation.size, "What is FPL");
		slide->backgroundColor = backColor;

		Rect2f area = AddHeaderAndFooter(slide, normalFont, headerFontSize);

		Vec2f topLeft = area.pos;
		Vec2f center = topLeft + area.size * 0.5f;

		Vec2f pos;

		// Title
		pos = topLeft + V2f(area.size.w * 0.5f, 0.0f);
		slide->AddLabel(slide->name, pos, normalFont, titleFontSize, HorizontalAlignment::Center, VerticalAlignment::Top, labelStyle);

		// Content
		pos = topLeft + V2f(contentPadding, titleFontSize + normalFontSize);

		slide->AddLabel("FPL is an all-purpose / multimedia platform abstraction library,", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("providing a powerful and easy to use API, accessing low-level systems in a platform-independent way:", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight * 2);

		slide->AddLabel("- Platform detection (x86/x64/Arm, Win32/Linux/Unix, etc.)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Compiler detection (MSVC/GCC/Clang/Intel)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Macros (Debugbreak, Assertions, CPU-Features, Memory init etc.)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Dynamic library loading (.dll/.so)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Single window creation and handling (Win32/X11)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Event and input polling (Keyboard/Mouse/Gamepad)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Video initialization and output (Software, OpenGL, etc.)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Asyncronous audio playback (DirectSound, ALSA, etc.)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- IO (Console, Paths, Files, Directories, etc.)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Memory handling with or without alignment", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Multithreading (Atomics, Threads, Mutexes, Conditionals, etc.)", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Retrieving hardware informations", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- and many more", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
	}

	// Motivation
	{
		Slide *slide = presentation.AddSlide(presentation.size, "Motivation");
		slide->backgroundColor = backColor;

		Rect2f area = AddHeaderAndFooter(slide, normalFont, headerFontSize);

		Vec2f topLeft = area.pos;
		Vec2f center = topLeft + area.size * 0.5f;

		Vec2f pos;

		// Title
		pos = topLeft + V2f(area.size.w * 0.5f, 0.0f);
		slide->AddLabel(slide->name, pos, normalFont, titleFontSize, HorizontalAlignment::Center, VerticalAlignment::Top, labelStyle);

		// Content
		pos = topLeft + V2f(contentPadding, titleFontSize + normalFontSize);

		slide->AddLabel("C/C++ has very limited access to the underlying platform,", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("so you have to use third-party libraries to get access to low level systems.", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight * 2);

		slide->AddLabel("The pre-existing platform abstraction libraries have a lot of issues:", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Massive in file count and/or size", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Massive in number of translation units", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Massive in memory usage and number of allocations", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Some are built on top of third-party dependencies", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Without configuration and/or buildsystems you cant compile it", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Statically linking is madness or not supported at all", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Forces you to either static or runtime linking", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- It takes forever to compile", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- Including the full source is either impossible or extremely cumbersome", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- No control over the allocated memory, at max you can overwrite malloc/free", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
		pos += V2f(0, normalLineHeight);
		slide->AddLabel("- They are heavily bloated", pos, normalFont, normalFontSize, HorizontalAlignment::Left, VerticalAlignment::Top, labelStyle);
	}
#endif
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo | Presentation", settings.window.title, fplArrayCount(settings.window.title));
	settings.video.driver = fplVideoDriverType_OpenGL;
	settings.video.isVSync = true;
	settings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	if (fplPlatformInit(fplInitFlags_All, &settings)) {
		if (fglLoadOpenGL(true)) {
			glDisable(GL_DEPTH_TEST);

			glDisable(GL_CULL_FACE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_SCISSOR_TEST);

			glDisable(GL_TEXTURE_2D);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glMatrixMode(GL_MODELVIEW);

			glClearColor(0.1f, 0.2f, 0.3f, 1);

			App *appMemory = (App *)fplMemoryAllocate(sizeof(App));
			App &app = *appMemory;

			app.renderer.strings = &app.strings;
			app.presentation.strings = &app.strings;

			// First font is always the debug font
			app.renderer.debugFont = app.renderer.AddFontFromResource(FontResources::BitStreamVerySans, 16.0f);

			app.renderer.AddFontFromResource(FontResources::BitStreamVerySans, 32.0f);
			app.renderer.AddFontFromResource(FontResources::BitStreamVerySans, 48.0f);
			app.renderer.AddFontFromResource(FontResources::Arimo, 16.0f);
			app.renderer.AddFontFromResource(FontResources::Arimo, 32.0f);
			app.renderer.AddFontFromResource(FontResources::Arimo, 48.0f);

			app.renderer.AddImageFromResource(ImageResources::FPLLogo128x128);

#if 0
			app.renderer.AddFontFromFile("c:/windows/fonts/arial.ttf", "Arial", 24);
#endif

			BuildFPLPresentation(app.renderer, app.presentation);

			UpdatePresentationVariables(app.presentation);

			ShowSlideshow(app, 0, false);

			const float dt = 1.0f / 60.0f;

			while (fplWindowUpdate()) {
				fplEvent ev;
				while (fplPollEvent(&ev)) {
					if (ev.type == fplEventType_Keyboard) {
						if (ev.keyboard.type == fplKeyboardEventType_Button) {
							if (ev.keyboard.buttonState == fplButtonState_Release) {
								switch (ev.keyboard.mappedKey) {
									case fplKey_F:
									{
										if (!fplIsWindowFullscreen()) {
											fplEnableWindowFullscreen();
										} else {
											fplDisableWindowFullscreen();
										}
									} break;

									case fplKey_PageUp:
										JumpToPrevSlide(app);
										break;

									case fplKey_PageDown:
										JumpToNextSlide(app);
										break;
								}
							}
						}
					}
				}

				fplWindowSize winSize = fplZeroInit;
				fplGetWindowSize(&winSize);

				UpdateFrame(app, dt);

				RenderFrame(app, V2iInit(winSize.width, winSize.height));

				fplVideoFlip();
			}

			if (fplIsWindowFullscreen()) {
				fplDisableWindowFullscreen();
			}

			ReleaseApp(app);

			fplMemoryFree(appMemory);
		}
		fplPlatformRelease();
	}
}