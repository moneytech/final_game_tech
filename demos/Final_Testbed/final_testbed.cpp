/*
-------------------------------------------------------------------------------
Name:
	Final's Testbed

Description:
	Playground for testing out all my stuff.

Author:
	Torsten Spaete

License:
	Copyright (c) 2017-2019 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/
#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#define FINAL_FONTLOADER_IMPLEMENTATION
#define FINAL_FONTLOADER_BETTERQUALITY 1
#include <final_fontloader.h>

#define FINAL_GAMEPLATFORM_IMPLEMENTATION
#include <final_gameplatform.h>

#include <final_math.h>

#include <new> // placement new

#include "static_data.h"

struct FontAsset {
	LoadedFont data;
	TextureHandle texture;
};

enum class AssetType {
	None,
	Font
};

enum class AssetLoadState : int32_t {
	Failed = -1,
	Unloaded = 0,
	ToUpload,
	ToFree,
	Loaded
};

struct Asset {
	AssetType type;
	AssetLoadState loadState;
	union {
		FontAsset font;
	};
};

struct GameState {
	Asset debugFont;
	Viewport viewport;
	float angle;
	bool isExiting;
};

static bool Init(GameState &state) {
	state.debugFont.type = AssetType::Font;
	state.debugFont.loadState = AssetLoadState::Unloaded;
	size_t fontDataSize = fplArrayCount(fontDataArray);
	if (LoadFontFromMemory(fontDataArray, fontDataSize, 0, 36.0f, 32, 128, 512, 512, false, &state.debugFont.font.data)) {
		state.debugFont.loadState = AssetLoadState::ToUpload;
	}
	state.angle = 0.0f;
	return(true);
}

static void Kill(GameState *state) {
	ReleaseFont(&state->debugFont.font.data);
}

extern bool GameInit(GameMemory &gameMemory) {
	GameState *state = (GameState *)fmemPush(gameMemory.memory, sizeof(GameState), fmemPushFlags_Clear);
	gameMemory.game = state;
	if (!Init(*state)) {
		GameRelease(gameMemory);
		return(false);
	}
	return(true);
}

extern void GameRelease(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	if (state != nullptr) {
		Kill(state);
	}
}

extern bool IsGameExiting(GameMemory &gameMemory) {
	GameState *state = gameMemory.game;
	fplAssert(state != nullptr);
	return state->isExiting;
}

extern void GameInput(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	fplAssert(state != nullptr);
	state->viewport.x = 0;
	state->viewport.y = 0;
	state->viewport.w = input.windowSize.w;
	state->viewport.h = input.windowSize.h;
}

extern void GameUpdate(GameMemory &gameMemory, const Input &input) {
	if (!input.isActive) {
		return;
	}
	GameState *state = gameMemory.game;
	fplAssert(state != nullptr);
	state->angle += input.deltaTime * 0.1f;
}

static Rect2f ComputeAspectRect(Vec2f targetSize, Vec2f sourceSize, Ratio sourceRatio) {
	float aspect_ratio;
	if (sourceRatio.numerator == 0.0) {
		aspect_ratio = 0.0f;
	} else {
		aspect_ratio = (float)ComputeRatio(sourceRatio);
	}
	if (aspect_ratio <= 0.0f) {
		aspect_ratio = 1.0f;
	}
	aspect_ratio *= sourceSize.w / sourceSize.h;

	float height = targetSize.h;
	float width = height * aspect_ratio;
	if (width > targetSize.w) {
		width = targetSize.w;
		height = width / aspect_ratio;
	}

	float x = (targetSize.w - width) * 0.5f;
	float y = (targetSize.h - height) * 0.5f;

	Rect2f result = MakeRect(V2f(x, y), V2f(width, height));
	return(result);
}

extern void GameRender(GameMemory &gameMemory, const float alpha) {
	GameState *state = gameMemory.game;
	fplAssert(state != nullptr);
	RenderState &renderState = *gameMemory.render;

	if (state->debugFont.loadState == AssetLoadState::ToUpload) {
		fplAssert(state->debugFont.type == AssetType::Font);
		const LoadedFont &font = state->debugFont.font.data;
		PushTexture(renderState, &state->debugFont.font.texture, font.atlasAlphaBitmap, font.atlasWidth, font.atlasHeight, 1, TextureFilterType::Linear, TextureWrapMode::ClampToEdge, false, false);
	}

	PushViewport(renderState, state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
	PushClear(renderState, V4f(0.1f, 0.2f, 0.3f, 1), ClearFlags::Color);

	Vec2f viewSize = V2f(10, 6);

	Mat4f proj = Mat4OrthoRH(0.0f, viewSize.w, viewSize.h, 0.0f, 0.0f, 1.0f);
	Mat4f view = Mat4Translation(V2f(0, 0)) * Mat4Scale(V2f(1, 1));
	SetMatrix(renderState, proj * view);

#define DEMO_IMAGEFIT 1
#define DEMO_TEST 2

#define DEMO DEMO_IMAGEFIT

#if DEMO == DEMO_IMAGEFIT
	Vec2f maxSize = viewSize * 0.75f;
	Vec2f maxPos = (viewSize - maxSize) * 0.5f;

	Vec2f sourceImageSize = V2f(1000, 100);
	Ratio sourceImageAspect = MakeRatio(1, 1);
	float containerAspect = maxSize.w / maxSize.h;

	Rect2f imageRect = ComputeAspectRect(maxSize, sourceImageSize, sourceImageAspect);

	Vec2f imageSize = imageRect.size;

	Vec2f imageExt = imageSize * 0.5f;

	Vec2f imageCenter = maxPos + imageRect.pos + imageExt;

	PushRectangle(renderState, maxPos, maxSize, V4f(1, 1, 1, 1), false, 1.0f);

	PushRectangle(renderState, maxPos + imageRect.pos, imageSize, V4f(1, 0, 0, 1), false, 1.0f);

	float imageRot = state->angle;

	Mat4f initialTranslationMat = Mat4Translation(imageCenter);
	Mat4f imageRotMat = Mat4RotationZ(imageRot);
	Mat4f imageMat = initialTranslationMat * imageRotMat;

	Vec2f verts[] = {
		V2f(-imageExt.w, -imageExt.h),
		V2f(imageExt.w, -imageExt.h),
		V2f(imageExt.w, imageExt.h),
		V2f(-imageExt.w, imageExt.h),
	};

	Vec2f min = Vec4MultMat4(imageRotMat, V4f(verts[0])).xy;
	Vec2f max = min;
	for (int i = 1; i < fplArrayCount(verts); ++i) {
		Vec2f v = Vec4MultMat4(imageRotMat, V4f(verts[i])).xy;
		min = Vec2Min(min, v);
		max = Vec2Max(max, v);
	}

	Vec2f rotatedSize = max - min;

	float factor = 1.0f;
	float rotatedAspect = rotatedSize.w / rotatedSize.h;
	if (rotatedAspect > containerAspect) {
		factor = maxSize.w / rotatedSize.w;
	} else {
		factor = maxSize.h / rotatedSize.h;
	}

	Vec2f scaledSize = imageSize * factor;

	PushMatrix(renderState, imageMat);
	PushRectangle(renderState, -imageExt, imageSize, V4f(0, 1, 0, 1), false, 1.0f);
	PopMatrix(renderState);

	PushRectangle(renderState, imageCenter - rotatedSize * 0.5f, rotatedSize, V4f(0, 0, 1, 1), false, 1.0f);

	PushMatrix(renderState, imageMat);
	PushRectangle(renderState, -scaledSize * 0.5f, scaledSize, V4f(0, 1, 1, 1), false, 2.0f);
	PopMatrix(renderState);
#endif // DEMO_IMAGEFIT

#if DEMO == DEMO_TEST
	PushRectangleCenter(renderState, V2f(0, 0), V2f(w * 0.2f, h * 0.2f), V4f(1, 1, 1, 1), false, 1.0f);
	PushRectangle(renderState, V2f(0, 0), V2f(w * 0.25f, h * 0.25f), V4f(1, 1, 1, 1), true, 0.0f);

	Vec2f verts[] = {
		V2f(0.0f, h * 0.3f),
		V2f(-w * 0.3f, -h * 0.3f),
		V2f(w * 0.3f, -h * 0.3f),
	};
	PushVertices(renderState, verts, fplArrayCount(verts), true, V4f(0, 1, 1, 1), DrawMode::Lines, true, 1.0f);

	view = Mat4Translation(V2f(w * 0.25f, -h * 0.1f)) * Mat4Scale(V2f(0.5f, 0.5f));
	SetMatrix(renderState, proj * view);
	PushVertices(renderState, verts, fplArrayCount(verts), true, V4f(1, 0, 1, 1), DrawMode::Polygon, true, 1.0f);

	view = Mat4Translation(V2f(0, 0));
	SetMatrix(renderState, proj * view);
	PushText(renderState, "Hello", 5, &state->debugFont.font.data, &state->debugFont.font.texture, V2f(0, 0), h * 0.1f, 0.0f, 0.0f, V4f(1, 0, 0, 1));
#endif // DEMO_TEST
}

extern void GameUpdateAndRender(GameMemory &gameMemory, const Input &input, const float alpha) {
}

int main(int argc, char **argv) {
	GameConfiguration config = {};
	config.title = L"Final�s Testbed";
	config.hideMouseCursor = false;
	config.disableInactiveDetection = true;
	int result = GameMain(config);
	return(result);
}