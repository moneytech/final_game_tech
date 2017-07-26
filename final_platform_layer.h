/*
final_platform_layer.h - FPL -  v0.1 alpha
Open-Source Single-File Header-Library by Torsten Spaete

This library is designed to abstract the underlying platform to a very simple and easy to understand api.
The only dependencies are built-in operatoring system libraries and thats it - even the C-Runtime-Library is not required.

The main focus is game development, so the default settings will create a window and setup some rendering.

It is fully C-89 compatible but support C++ as well.

Some features can be fully compiled out as needed.

HOW TO USE:

- In one of your C or C++ translation units include this:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

- Provide the typical main entry point

int main(int argc, char **args) {
}

- Initialize the library and release it when you are done

fpl_Init(fpl_InitFlag_All);
fpl_Release();

USE CASE [OpenGL-Window]:

#define FPL_IMPLEMENTATION
#include "final_platform_layer.h"

int main(int argc, char **args) {
	if (fpl_Init(fpl_InitFlag_VideoOpenGL)) {
		while (fpl_WindowUpdate()) {
			glClear(GL_COLOR_BUFFER_BIT);
			fpl_WindowFlip();
		}
		fpl_Release();
	}
}

When using OpenGL you may need to link to the systems opengl library manually:

- Win32: opengl32.lib
- Linux: libGL.so

PREPROCESSOR OVERRIDES:

- FPL_API_AS_PRIVATE 0 or 1

- FPL_DEFAULT_WINDOW_WIDTH 1 or greater
- FPL_DEFAULT_WINDOW_HEIGHT 1 or greater

- FPL_ENABLE_ASSERTIONS 0 or 1

- FPL_ENABLE_WINDOW 0 or 1
- FPL_ENABLE_OPENGL 0 or 1
- FPL_ENABLE_C_RUNTIME_LIBRARY 0 or 1

FEATURES:

[x] Creating a fixed or resizeable window
[x] Handling window, keyboard, mouse events
[ ] Polling gamepad informations
[ ] Clipboard string reading and writing

[p] Creating a opengl rendering context
[ ] Creating modern opengl 3+ rendering context

[ ] Audio playback using OS native libraries

[x] Memory allocation and de-allocation with custom alignment support
[x] Atomic operations
[ ] String manipulation functions
[ ] Path, file and directory functions
[ ] Thread, mutex, condition handling

SUPPORTED ARCHITECTURES:

[x] x86
[x] x86_64

SUPPORTED COMPILERS:

[X] Compiles with MSVC
[ ] Compiles with GCC/G++
[ ] Compiles with Clang
[ ] Compiles with Intel C/C++ Compiler

SUPPORTED PLATFORMS:

[x] Win32
[ ] Linux (Planned)
[ ] Unix/Posix (Planned)
[ ] OSX (Not sure)

CODE-GUIDELINES:

- Must be compilable under C and C++ but never require any C++ features

- Everything must be prefixed with fpl_ or FPL_ but never with Fpl_ or fPl

- All internal types/structures/functions must be postfixed with _internal or _Internal or _INTERNAL

- Large names are prefered over short names

- Value by reference is passed by pointer only

- Returning or passing short structs by value is fine

- Dont rely on compilers, define/declare it explicitly always

- Preprocessor directives must be properly indented
	* Header and Implementation guards are treated as non-existing
	* There is no preprocessor directives allowed inside code blocks
	* Definitions must be formatted logically, first non directives than directives.

- Tab identations with windows \r\n linefeeds only

- All api definitions + definition must use function statement fpl_api

- Constants:
	* Simple defines in uppercase
	* Explicit type cast

	Good:
		#define FPL_MAX_EVENT_COUNT_INTERNAL (uint32_t)32768

	Bad:
		#define FPL_MAX_EVENT_COUNT_INTERNAL 32768
		const uint32_t FPL_MAX_EVENT_COUNT_INTERNAL = 32768;
		static const uint32_t fpl_MaxEventCount_Internal = 32768;

- Function pointers:
	* Prefix fpl_func_
	* Lowercase with underscore only

	Good:
		typedef int (*fpl_func_my_function_doing_somthing)(char a, int b, void *c);

	Bad:
		typedef int (*fpl_my_function_ptr)(char a, int b, void *c);
		typedef int (*fpl_myFunctionPtr)(char a, int b, void *c);
		typedef int (*fpl_MyFunctionPtr)(char a, int b, void *c);
		typedef int (*MyFunctionPtr)(char a, int b, void *c);

- Readonly primitive arguments are defined as const
	Good:
		void fpl_MyFunctionDoingSomething(const int width, const int height);
	Bad:
		void fpl_MyFunctionDoingSomething(int width, int height);

- Function statements like inline, extern, etc. may never be used directly and must use its pendant already defined:
	fpl_inline		(Inline compiler hint)
	fpl_api			(Public API definition)
	fpl_internal	(For internals only)
	fpl_globalvar	(For global variables only)

- Primitive type definitions:
	Good:
		typedef uint32_t fpl_bool32;

	Bad:
		typedef uint32_t fplBool32;

- Use existing standard types always
	* Unless there a type missing

	Good:
		typedef uint32_t fpl_bool32;

	Bad:
		typedef int32_t fpl_s32;

- Enums:
	* Enum values must be set always
	* Anonymous enums only
	* C-Style

	Good:
		enum {
			fpl_Key_None = 0,
			fpl_Key_Escape = 27,
		};
		typedef uint32_t fpl_key;

	Bad:
		typedef enum fpl_key {
			fpl_Key_Escape = 27,
		} fpl_key;

		typedef enum fpl_key {
			fpl_Key_None,
			fpl_Key_Escape,
		} fpl_key;

		enum fpl_key {
			fpl_Key_Escape = 27,
		};

- Structures:
	* Camel-case style:

	Good:
		typedef struct fpl_MyStructForStuff {
		};

	Bad:
		typedef struct fpl_mystructforstuff {
		};
		typedef struct fpl_my_struct_for_stuff {
		};

LICENSE:

MIT License

Copyright (c) 2017 Torsten Spaete
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

VERSION HISTORY:
v0.1 (2017-05-10) Initial version
*/

#ifndef FPL_INCLUDE_H
#define FPL_INCLUDE_H

// ****************************************************************************
//
// Header
//
// ****************************************************************************

//
// Platform detection
//
#if defined(_WIN32)
#	define FPL_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FPL_PLATFORM_LINUX
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FPL_PLATFORM_UNIX
#else
#	error "This platform/compiler is not supported!"
#endif

//
// Architecture detection (x86, x64)
// See: https://sourceforge.net/p/predef/wiki/Architectures/
//
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
#	define FPL_ARCH_X64	
#elif defined(_WIN32) || defined(__i386__) || defined(__X86__) || defined(_X86_)
#	define FPL_ARCH_X86
#else
#	error "This architecture/compiler is not supported!"
#endif

//
// Build configuration and compilers
// See: http://beefchunk.com/documentation/lang/c/pre-defined-c/precomp.html
// See: http://nadeausoftware.com/articles/2012/10/c_c_tip_how_detect_compiler_name_and_version_using_compiler_predefined_macros
//
#if defined(_MSC_VER)
#	define FPL_COMPILER_MSVC
#	if defined(_DEBUG) || (!defined(NDEBUG))
#		define FPL_DEBUG
#	else
#		define FPL_RELEASE
#	endif
#elif defined(__llvm__)
#	define FPL_COMPILER_LLVM
#elif defined(__clang__)
#	define FPL_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#	define FPL_COMPILER_GCC
#elif defined(__MINGW32__)
#	define FPL_COMPILER_MINGW
#elif defined(__INTEL_COMPILER) || defined(__ICC)
#	define FPL_COMPILER_INTEL
#else
#	define FPL_COMPILER_UNKNOWN
#endif

//
// Default preprocessor overrides
//
#if !defined(FPL_DEFAULT_WINDOW_WIDTH)
#	define FPL_DEFAULT_WINDOW_WIDTH 800
#endif
#if !defined(FPL_DEFAULT_WINDOW_HEIGHT)
#	define FPL_DEFAULT_WINDOW_HEIGHT 600
#endif
#if !defined(FPL_ENABLE_ASSERTIONS)
#	if defined(FPL_DEBUG)
#		define FPL_ENABLE_ASSERTIONS 1
#	else
#		define FPL_ENABLE_ASSERTIONS 0
#	endif
#endif
#if !defined(FPL_ENABLE_OPENGL)
#	define FPL_ENABLE_OPENGL 1
#endif
#if !defined(FPL_ENABLE_WINDOW)
#	define FPL_ENABLE_WINDOW 1
#endif
#if !defined(FPL_ENABLE_C_RUNTIME_LIBRARY)
#	define FPL_ENABLE_C_RUNTIME_LIBRARY 1
#endif

//
// Types
//
#include <stdint.h>
typedef uint32_t fpl_bool32;
enum {
	fpl_false = 0,
	fpl_true = 1,
};
#define FPL_NULL 0

//
// Assertions
//
#if FPL_ENABLE_ASSERTIONS
#	define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
#	define FPL_STATICASSERT_INTERNAL(exp, line) \
		int fpl_static_assert_##line(int static_assert_failed[(exp)?1:-1])
#	define FPL_STATICASSERT(exp) \
		FPL_STATICASSERT_INTERNAL(exp, __LINE__)
#else
#	define FPL_ASSERT(exp)
#	define FPL_STATICASSERT(exp)
#endif

//
// Static/Inline/Extern/Internal
//
#define fpl_globalvar static
#define fpl_inline inline
#define fpl_internal static
#if FPL_API_AS_PRIVATE
#	define fpl_api static
#else
#	define fpl_api extern
#endif

//
// Macro functions
//
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FPL_OFFSETOF(type, field) ((type *)((void *)0)->field)

//
// Disabled C-Runtime-Support
//
// @TODO: This is a pain in the ass to get it compile without CRT, so we may require C-Runtime as a dependency always
#if	!FPL_ENABLE_C_RUNTIME_LIBRARY
#	if defined(FPL_COMPILER_MSVC)
#		ifdef __cplusplus
extern "C" {
#		endif // __cplusplus

	int _fltused;
#		ifdef __cplusplus
}
#		endif // __cplusplus
#	endif // defined(FPL_COMPILER_MSVC)
#endif // !FPL_ENABLE_C_RUNTIME_LIBRARY

//
// API
//
#ifdef __cplusplus
extern
"C" {
#endif
	typedef struct fpl_LibraryHandle {
		void *internalHandle;
		fpl_bool32 isValid;
	} fpl_LibraryHandle;

	typedef struct fpl_FileHandle {
		void *internalHandle;
		fpl_bool32 isValid;
	} fpl_FileHandle;

	enum {
		fpl_FileType_Binary = 0,
		fpl_FileType_Text = 1,
	};
	typedef uint32_t fpl_FileType;

	enum {
		fpl_FilePositionMode_Beginning,
		fpl_FilePositionMode_Current,
		fpl_FilePositionMode_End,
	};
	typedef uint32_t fpl_FilePositionMode;

	enum {
		fpl_InitFlag_None = 0,
		fpl_InitFlag_Window = 1 << 0,
		fpl_InitFlag_VideoOpenGL = 1 << 1,
		fpl_InitFlag_All = fpl_InitFlag_Window | fpl_InitFlag_VideoOpenGL,
	};
	typedef uint32_t fpl_InitFlag;

	// Atomics
	fpl_api void fpl_AtomicReadFence();
	fpl_api void fpl_AtomicWriteFence();
	fpl_api void fpl_AtomicReadWriteFence();
	fpl_api uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
	fpl_api uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
	fpl_api uint32_t fpl_AtomicAddU32(volatile uint32_t *value, const uint32_t addend);
	fpl_api uint64_t fpl_AtomicAddU64(volatile uint64_t *value, const uint64_t addend);
	fpl_api uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand);
	fpl_api uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand);

	// Core
	fpl_api fpl_bool32 fpl_Init(const fpl_InitFlag initFlags);
	fpl_api void fpl_Release();
	fpl_api fpl_LibraryHandle fpl_LoadLibrary(const char *libraryFilePath);
	fpl_api void *fpl_GetLibraryProcAddress(fpl_LibraryHandle *handle, const char *name);
	fpl_api void fpl_ReleaseLibrary(fpl_LibraryHandle *handle);

	// Console
	fpl_api void fpl_ConsoleOut(const char *text);
	fpl_api void fpl_ConsoleFormatOut(const char *format, ...);

	// Memory
	fpl_api void fpl_ClearMemory(void *mem, const size_t size);
	fpl_api void *fpl_AllocateMemory(const size_t size);
	fpl_api void fpl_FreeMemory(void *ptr);
	fpl_api void *fpl_AllocateAlignedMemory(const size_t size, const size_t alignment);
	fpl_api void fpl_FreeAlignedMemory(void *ptr);

	// Timing
	fpl_api double fpl_GetHighResolutionTimeInSeconds();

	// Strings
	fpl_api uint32_t fpl_GetStringLength(const char *str);
	fpl_api uint32_t fpl_GetWideStringLength(const wchar_t *str);
	fpl_api void fpl_CopyString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
	fpl_api void fpl_CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
	fpl_api void fpl_WideStringToOEMString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *oemDest, const uint32_t maxOemDestLen);
	fpl_api void fpl_WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
	fpl_api void fpl_OEMStringToWideString(const char *oemSource, const uint32_t oemSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
	fpl_api void fpl_UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);

	// Files, Directories and Paths
	fpl_api fpl_FileHandle fpl_OpenBinaryFile(const char *filePath);
	fpl_api fpl_FileHandle fpl_CreateBinaryFile(const char *filePath);
	fpl_api uint32_t fpl_ReadFileBlock32(fpl_FileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
	fpl_api uint32_t fpl_WriteFileBlock32(fpl_FileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize);
	fpl_api void fpl_SetFilePosition32(fpl_FileHandle *fileHandle, const uint32_t position, const fpl_FilePositionMode mode);
	fpl_api uint32_t fpl_GetFilePosition32(fpl_FileHandle *fileHandle);

	// @TODO: Add 64-bit file methods as well!

	fpl_api void fpl_CloseFile(fpl_FileHandle *fileHandle);

	fpl_api void fpl_GetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
	fpl_api char *fpl_ExtractFilePath(char *destPath, const uint32_t maxDestLen, const char *sourcePath);
	fpl_api char *fpl_ExtractFileExtension(char *destPath, const uint32_t maxDestLen, const char *sourcePath);
	fpl_api char *fpl_ExtractFileName(char *destPath, const uint32_t maxDestLen, const char *sourcePath);
	fpl_api char *fpl_ChangeFileExtension(char *destPath, const uint32_t maxDestLen, const char *sourcePath);
	fpl_api void fpl_CombinePath(char *destPath, const uint32_t maxDestPathLen, ...);

#if FPL_ENABLE_WINDOW
	// @NOTE: Based on MS Virtual-Key-Codes, mostly directly mappable to ASCII
	enum {
		fpl_Key_None = 0,

		// 0x07: Undefined

		fpl_Key_Backspace = 0x08,
		fpl_Key_Tab = 0x09,

		// 0x0A-0x0B: Reserved

		fpl_Key_Clear = 0x0C,
		fpl_Key_Enter = 0x0D,

		// 0x0E-0x0F: Undefined

		fpl_Key_Shift = 0x10,
		fpl_Key_Control = 0x11,
		fpl_Key_Alt = 0x12,
		fpl_Key_Pause = 0x13,
		fpl_Key_CapsLock = 0x14,

		// 0x15: IME-Keys
		// 0x16: Undefined
		// 0x17-0x19 IME-Keys
		// 0x1A: Undefined

		fpl_Key_Escape = 0x1B,

		// 0x1C - 0x1F: IME-Keys

		fpl_Key_Space = 0x20,
		fpl_Key_PageUp = 0x21,
		fpl_Key_PageDown = 0x22,
		fpl_Key_End = 0x23,
		fpl_Key_Home = 0x24,
		fpl_Key_Left = 0x25,
		fpl_Key_Up = 0x26,
		fpl_Key_Right = 0x27,
		fpl_Key_Down = 0x28,
		fpl_Key_Select = 0x29,
		fpl_Key_Print = 0x2A,
		fpl_Key_Execute = 0x2B,
		fpl_Key_Snapshot = 0x2C,
		fpl_Key_Insert = 0x2D,
		fpl_Key_Delete = 0x2E,
		fpl_Key_Help = 0x2F,

		fpl_Key_0 = 0x30,
		fpl_Key_1 = 0x31,
		fpl_Key_2 = 0x32,
		fpl_Key_3 = 0x33,
		fpl_Key_4 = 0x34,
		fpl_Key_5 = 0x35,
		fpl_Key_6 = 0x36,
		fpl_Key_7 = 0x37,
		fpl_Key_8 = 0x38,
		fpl_Key_9 = 0x39,

		// 0x3A-0x40: Undefined

		fpl_Key_A = 0x41,
		fpl_Key_B = 0x42,
		fpl_Key_C = 0x43,
		fpl_Key_D = 0x44,
		fpl_Key_E = 0x45,
		fpl_Key_F = 0x46,
		fpl_Key_G = 0x47,
		fpl_Key_H = 0x48,
		fpl_Key_I = 0x49,
		fpl_Key_J = 0x4A,
		fpl_Key_K = 0x4B,
		fpl_Key_L = 0x4C,
		fpl_Key_M = 0x4D,
		fpl_Key_N = 0x4E,
		fpl_Key_O = 0x4F,
		fpl_Key_P = 0x50,
		fpl_Key_Q = 0x51,
		fpl_Key_R = 0x52,
		fpl_Key_S = 0x53,
		fpl_Key_T = 0x54,
		fpl_Key_U = 0x55,
		fpl_Key_V = 0x56,
		fpl_Key_W = 0x57,
		fpl_Key_X = 0x58,
		fpl_Key_Y = 0x59,
		fpl_Key_Z = 0x5A,

		fpl_Key_LeftWin = 0x5B,
		fpl_Key_RightWin = 0x5C,
		fpl_Key_Apps = 0x5D,

		// 0x5E: Reserved

		fpl_Key_Sleep = 0x5F,
		fpl_Key_NumPad0 = 0x60,
		fpl_Key_NumPad1 = 0x61,
		fpl_Key_NumPad2 = 0x62,
		fpl_Key_NumPad3 = 0x63,
		fpl_Key_NumPad4 = 0x64,
		fpl_Key_NumPad5 = 0x65,
		fpl_Key_NumPad6 = 0x66,
		fpl_Key_NumPad7 = 0x67,
		fpl_Key_NumPad8 = 0x68,
		fpl_Key_NumPad9 = 0x69,
		fpl_Key_Multiply = 0x6A,
		fpl_Key_Add = 0x6B,
		fpl_Key_Separator = 0x6C,
		fpl_Key_Substract = 0x6D,
		fpl_Key_Decimal = 0x6E,
		fpl_Key_Divide = 0x6F,
		fpl_Key_F1 = 0x70,
		fpl_Key_F2 = 0x71,
		fpl_Key_F3 = 0x72,
		fpl_Key_F4 = 0x73,
		fpl_Key_F5 = 0x74,
		fpl_Key_F6 = 0x75,
		fpl_Key_F7 = 0x76,
		fpl_Key_F8 = 0x77,
		fpl_Key_F9 = 0x78,
		fpl_Key_F10 = 0x79,
		fpl_Key_F11 = 0x7A,
		fpl_Key_F12 = 0x7B,
		fpl_Key_F13 = 0x7C,
		fpl_Key_F14 = 0x7D,
		fpl_Key_F15 = 0x7E,
		fpl_Key_F16 = 0x7F,
		fpl_Key_F17 = 0x80,
		fpl_Key_F18 = 0x81,
		fpl_Key_F19 = 0x82,
		fpl_Key_F20 = 0x83,
		fpl_Key_F21 = 0x84,
		fpl_Key_F22 = 0x85,
		fpl_Key_F23 = 0x86,
		fpl_Key_F24 = 0x87,

		// 0x88-8F: Unassigned

		fpl_Key_NumLock = 0x90,
		fpl_Key_Scroll = 0x91,

		// 0x92-9x96: OEM specific
		// 0x97-0x9F: Unassigned

		fpl_Key_LeftShift = 0xA0,
		fpl_Key_RightShift = 0xA1,
		fpl_Key_LeftControl = 0xA2,
		fpl_Key_RightControl = 0xA3,
		fpl_Key_LeftAlt = 0xA4,
		fpl_Key_RightAlt = 0xA5,

		// 0xA6-0xFE: Dont care
	};

	typedef uint64_t fpl_Key;

	typedef struct fpl_WindowConfiguration {
		char windowTitle[128];
		uint32_t windowWidth;
		uint32_t windowHeight;
	} fpl_WindowConfiguration;

	typedef enum fpl_WindowEventType {
		fpl_WindowEventType_Resize = 1,
	} fpl_WindowEventType;

	typedef struct fpl_WindowEvent {
		fpl_WindowEventType type;
		int32_t width;
		int32_t height;
	} fpl_WindowEvent;

	typedef enum fpl_KeyboardEventType {
		fpl_KeyboardEventType_KeyDown = 1,
		fpl_KeyboardEventType_KeyUp,
		fpl_KeyboardEventType_Char,
	} fpl_KeyboardEventType;

	enum {
		fpl_KeyboardModifierType_None = 0,
		fpl_KeyboardModifierType_Alt = 1 << 0,
		fpl_KeyboardModifierType_Ctrl = 1 << 1,
		fpl_KeyboardModifierType_Shift = 1 << 2,
		fpl_KeyboardModifierType_Super = 1 << 3,
	};
	typedef uint32_t fpl_KeyboardModifierType;

	typedef struct fpl_KeyboardEvent {
		fpl_KeyboardEventType type;
		uint64_t keyCode;
		fpl_Key mappedKey;
		fpl_KeyboardModifierType modifiers;
	} fpl_KeyboardEvent;

	typedef enum fpl_MouseEventType {
		fpl_MouseEventType_Move = 1,
		fpl_MouseEventType_ButtonDown,
		fpl_MouseEventType_ButtonUp,
		fpl_MouseEventType_Wheel,
	} fpl_MouseEventType;

	typedef enum fpl_MouseButtonType {
		fpl_MouseButtonType_None = -1,
		fpl_MouseButtonType_Left = 0,
		fpl_MouseButtonType_Right,
		fpl_MouseButtonType_Middle,
	} fpl_MouseButtonType;

	typedef struct fpl_MouseEvent {
		fpl_MouseEventType type;
		fpl_MouseButtonType mouseButton;
		int32_t mouseX;
		int32_t mouseY;
		float wheelDelta;
		int32_t _padding;
	} fpl_MouseEvent;

	typedef enum fpl_EventType {
		fpl_EventType_Window = 1,
		fpl_EventType_Keyboard,
		fpl_EventType_Mouse,
	} fpl_EventType;

	typedef struct fpl_Event {
		fpl_EventType type;
		union {
			fpl_WindowEvent window;
			fpl_KeyboardEvent keyboard;
			fpl_MouseEvent mouse;
		};
	} fpl_Event;

	// Window
	fpl_api fpl_bool32 fpl_IsWindowRunning();
	fpl_api fpl_bool32 fpl_WindowUpdate();
	fpl_api void fpl_WindowFlip();
	fpl_api int32_t fpl_GetWindowWidth();
	fpl_api int32_t fpl_GetWindowHeight();
	fpl_api void fpl_ShowWindowMouseCursor(const fpl_bool32 value);

	// Events
	fpl_api fpl_bool32 fpl_PollEvent(fpl_Event *event);
#endif

#if defined(FPL_PLATFORM_WINDOWS)
	// @NOTE(final): Required for access "main" from the actual win32 entry point
	fpl_api int main(int argc, char **args);
#endif

#ifdef __cplusplus
}
#endif

#endif // FPL_INCLUDE_H

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)
#define FPL_IMPLEMENTED

// Internal types and functions
#define FPL_CLEARMEMORY_INTERNAL(type, mem, size, shift, mask) do { \
	type *dataBlock = (type *)mem; \
	type *dataBlockEnd = (type *)dataBlock + (size >> shift); \
	uint8_t *data8 = (uint8_t *)dataBlockEnd; \
	uint8_t *data8End = data8 + (size & mask); \
	while (dataBlock != dataBlockEnd) { \
		*dataBlock++ = 0; \
	} \
	while (data8 != data8End) { \
		*data8++ = 0; \
	} \
} while (0)

#define FPL_MAX_LAST_ERROR_STRING_LENGTH_INTERNAL (uint32_t)256
typedef struct fpl_ErrorState_Internal {
	char lastError[FPL_MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
} fpl_ErrorState_Internal;

#if FPL_ENABLE_WINDOW
#	define FPL_MAX_EVENT_COUNT_INTERNAL (uint32_t)32768
typedef struct fpl_EventQueue_Internal {
	fpl_Event events[FPL_MAX_EVENT_COUNT_INTERNAL];
	volatile uint32_t pollIndex;
	volatile uint32_t pushCount;
} fpl_EventQueue_Internal;

fpl_globalvar fpl_EventQueue_Internal *fpl_GlobalEventQueue_Internal = 0;

fpl_internal void fpl_PushEvent_Internal(fpl_Event *event) {
	fpl_EventQueue_Internal *eventQueue = fpl_GlobalEventQueue_Internal;
	FPL_ASSERT(eventQueue != NULL);
	if (eventQueue->pushCount < FPL_MAX_EVENT_COUNT_INTERNAL) {
		uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->pushCount, 1);
		FPL_ASSERT(eventIndex < FPL_MAX_EVENT_COUNT_INTERNAL);
		eventQueue->events[eventIndex] = *event;
	}
}
#endif

#define FPL_CLEARSTRUCT_INTERNAL(value) \
	fpl_ClearMemory(value, sizeof(*value))

//
// All Public String
//
fpl_api uint32_t fpl_GetStringLength(const char *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_api uint32_t fpl_GetWideStringLength(const wchar_t *str) {
	uint32_t result = 0;
	if (str) {
		while (*str++) {
			result++;
		}
	}
	return(result);
}

fpl_api void fpl_CopyString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(source && dest);
	FPL_ASSERT((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	*dest = 0;
}

fpl_api void fpl_CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
	FPL_ASSERT(source && dest);
	FPL_ASSERT((sourceLen + 1) <= maxDestLen);
	uint32_t index = 0;
	while (index++ < sourceLen) {
		*dest++ = *source++;
	}
	dest[sourceLen] = 0;
}

//
// All Public Memory
//
fpl_api void *fpl_AllocateAlignedMemory(const size_t size, const size_t alignment) {
	FPL_ASSERT(size > 0);
	FPL_ASSERT((alignment > 0) && !(alignment & (alignment - 1)));

	// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
	size_t newSize = sizeof(void *) + size + (alignment << 1);
	void *basePtr = fpl_AllocateMemory(newSize);
	fpl_ClearMemory(basePtr, newSize);

	// The resulting address starts after the stored base pointer
	void *alignedPtr = (void *)((uintptr_t)basePtr + sizeof(void *));

	// Move the resulting address to a aligned one when not aligned
	// @TODO(final): This seems to do nothing on a typical i7 machine, regardless of the target - test with other architectures!
	uintptr_t mask = alignment - 1;
	if ((alignment > 1) && (((uintptr_t)alignedPtr & mask) != 0)) {
		*(uintptr_t *)alignedPtr += ((uintptr_t)alignment - ((uintptr_t)alignedPtr & mask));
	}

	// Write the base pointer before the alignment pointer
	*(void **)((void *)((uintptr_t)alignedPtr - sizeof(void *))) = basePtr;

	return(alignedPtr);
}

fpl_api void fpl_FreeAlignedMemory(void *ptr) {
	FPL_ASSERT(ptr != FPL_NULL);

	// Free the base pointer which is stored to the left from the given pointer
	void *basePtr = (void *)((void **)((uintptr_t)ptr - sizeof(void *)));
	fpl_FreeMemory(basePtr);
}

fpl_api void fpl_ClearMemory(void *mem, const size_t size) {
	if (size % 8 == 0) {
		FPL_CLEARMEMORY_INTERNAL(uint64_t, mem, size, 3, 0x00000007);
	} else if (size % 4 == 0) {
		FPL_CLEARMEMORY_INTERNAL(uint32_t, mem, size, 2, 0x00000003);
	} else if (size % 2 == 0) {
		FPL_CLEARMEMORY_INTERNAL(uint16_t, mem, size, 1, 0x00000001);
	} else {
		uint8_t *data8 = (uint8_t *)mem;
		uint8_t *data8End = data8 + size;
		while (data8 != data8End) {
			*data8++ = 0;
		}
	}
}

//
// All Public Window
//
#if FPL_ENABLE_WINDOW
fpl_api fpl_bool32 fpl_PollEvent(fpl_Event *event) {
	fpl_bool32 result = fpl_false;
	fpl_EventQueue_Internal *eventQueue = fpl_GlobalEventQueue_Internal;
	FPL_ASSERT(eventQueue != NULL);
	if (eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
		uint32_t eventIndex = fpl_AtomicAddU32(&eventQueue->pollIndex, 1);
		*event = eventQueue->events[eventIndex];
		result = fpl_true;
	} else if (fpl_GlobalEventQueue_Internal->pushCount > 0) {
		fpl_AtomicExchangeU32(&eventQueue->pollIndex, 0);
		fpl_AtomicExchangeU32(&eventQueue->pushCount, 0);
	}
	return result;
}
#endif

//
// ----------------------------------------------------------------------------
// WIN32 Platform
// ----------------------------------------------------------------------------
//
#if defined(FPL_PLATFORM_WINDOWS)
#	include <intrin.h>
	// @NOTE(final): windef.h defines min/max macros defined in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#	define NOMINMAX
#	include <windows.h>
#	include <windowsx.h> // macros for window messages
#	include <varargs.h>  // va_start, va_end, va_list
#	include <stdio.h> // fprintf
#	pragma comment(lib, "kernel32.lib")
#	pragma comment(lib, "user32.lib")
#	if FPL_ENABLE_WINDOW
#		pragma comment(linker, "/subsystem:windows")
#		if FPL_ENABLE_OPENGL
#			include <gl\gl.h>
#			pragma comment( lib, "opengl32.lib" )
#		endif // FPL_ENABLE_OPENGL
#	else
#		pragma comment(linker, "/subsystem:console")
#	endif // FPL_ENABLE_WINDOW

#	define FPL_PATH_SEPARATOR '\\'

// Win32 internal functions
#	if defined(UNICODE)
typedef wchar_t fpl_win32_char_internal;
#	else
typedef char fpl_win32_char_internal;
#	endif // defined(UNICODE)

#	if FPL_ENABLE_WINDOW
typedef struct fpl_Win32WindowState_Internal {
	HWND windowHandle;
	fpl_win32_char_internal windowClass[256];
	HDC deviceContext;
	fpl_bool32 isCursorActive;
	HCURSOR defaultCursor;
	fpl_bool32 isRunning;
} fpl_Win32WindowState_Internal;
#	else
typedef void *fpl_Win32WindowState_Internal;
#	endif // FPL_ENABLE_WINDOW

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
typedef struct fpl_Win32OpenGLState_Internal {
	HGLRC renderingContext;
} fpl_Win32OpenGLState_Internal;
#	else
typedef void *fpl_Win32OpenGLState_Internal;
#	endif // FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL

typedef struct fpl_Win32State_Internal {
	fpl_bool32 isInitialized;
	HINSTANCE appInstance;
	LARGE_INTEGER performanceFrequency;
	fpl_Win32WindowState_Internal window;
	fpl_Win32OpenGLState_Internal opengl;
} fpl_Win32State_Internal;

fpl_globalvar fpl_Win32State_Internal fpl_GlobalWin32State_Internal = { 0 };

// @TODO: Dont overwrite defines like that, just have one path for unicode and one for ansi
#	undef WNDCLASSEX
#	undef RegisterClassEx
#	undef UnregisterClass
#	undef CreateWindowEx
#	undef DefWindowProc
#	undef GetWindowLongPtr
#	undef SetWindowLongPtr
#	undef DispatchMessage
#	if defined(UNICODE)
#		define WIN32_CLASSNAME L"FPLWindowClass"
#		define WIN32_UNNAMED_WINDOW L"Unnamed FPL Window"
#		define WNDCLASSEX WNDCLASSEXW
#		define RegisterClassEx RegisterClassExW
#		define UnregisterClass UnregisterClassW
#		define CreateWindowEx CreateWindowExW
#		define DefWindowProc DefWindowProcW
#		define GetWindowLongPtr GetWindowLongPtrW
#		define SetWindowLongPtr SetWindowLongPtrW
#		define PeekMessage PeekMessageW
#		define DispatchMessage DispatchMessageW
#		define fpl_Win32StringCopy fpl_CopyWideString
#		define fpl_Win32GetStringLength fpl_GetWideStringLength
#		define MapVirtualKey MapVirtualKeyW
#	else
#		define WIN32_CLASSNAME "FPLWindowClass"
#		define WIN32_UNNAMED_WINDOW "Unnamed FPL Window"
#		define WNDCLASSEX WNDCLASSEXA
#		define RegisterClassEx RegisterClassExA
#		define UnregisterClass UnregisterClassA
#		define CreateWindowEx CreateWindowExA
#		define DefWindowProc DefWindowProcA
#		define GetWindowLongPtr GetWindowLongPtrA
#		define SetWindowLongPtr SetWindowLongPtrA
#		define PeekMessage PeekMessageA
#		define DispatchMessage DispatchMessageA
#		define fpl_Win32StringCopy fpl_CopyString
#		define fpl_Win32GetStringLength fpl_GetStringLength
#		define MapVirtualKey MapVirtualKeyA
#	endif // defined(UNICODE)

//
// Win32 Public Intrinsics
//
#	if defined(FPL_COMPILER_MSVC)
fpl_api void fpl_AtomicReadFence() {
	_ReadBarrier();
}
fpl_api void fpl_AtomicWriteFence() {
	_WriteBarrier();
}
fpl_api void fpl_AtomicReadWriteFence() {
	_ReadWriteBarrier();
}
fpl_api uint32_t fpl_AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
	uint32_t result = _InterlockedExchange((volatile long *)target, value);
	return (result);
}
fpl_api uint64_t fpl_AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
	uint64_t result = InterlockedExchange64((volatile long long *)target, value);
	return (result);
}
fpl_api uint32_t fpl_AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
	uint32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
	return (result);
}
fpl_api uint64_t fpl_AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
	uint64_t result = InterlockedExchangeAdd64((volatile long long *)value, addend);
	return (result);
}
fpl_api uint32_t fpl_AtomicCompareExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand) {
	uint32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
	return (result);
}
fpl_api uint64_t fpl_AtomicCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand) {
	uint64_t result = InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
	return (result);
}
#	endif // defined(FPL_COMPILER_MSVC)

// Win32 public console
fpl_api void fpl_ConsoleOut(const char *text) {
	// @TODO: Implement this!
}

fpl_api void fpl_ConsoleFormatOut(const char *format, ...) {
	va_list vaList;
	va_start(vaList, format);

	// @TODO: Implement this!

	va_end(vaList);
}

//
// Win32 Public Memory
//
fpl_api void *fpl_AllocateMemory(const size_t size) {
	FPL_ASSERT(size > 0);
	void *result = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return(result);
}

fpl_api void fpl_FreeMemory(void *ptr) {
	FPL_ASSERT(ptr != NULL);
	VirtualFree(ptr, 0, MEM_FREE);
}

//
// Win32 Public File/Path IO
//
fpl_api fpl_FileHandle fpl_OpenBinaryFile(const char *filePath) {
	fpl_FileHandle result = { 0 };
	HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (win32FileHandle != INVALID_HANDLE_VALUE) {
		result.isValid = fpl_true;
		result.internalHandle = (void *)win32FileHandle;
	}
	return(result);
}

fpl_api fpl_FileHandle fpl_CreateBinaryFile(const char *filePath) {
	fpl_FileHandle result = { 0 };
	HANDLE win32FileHandle = CreateFileA(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (win32FileHandle != INVALID_HANDLE_VALUE) {
		result.isValid = fpl_true;
		result.internalHandle = (void *)win32FileHandle;
	}
	return(result);
}

fpl_api uint32_t fpl_ReadFileBlock32(fpl_FileHandle *fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
	uint32_t result = 0;
	if ((fileHandle != FPL_NULL) && (fileHandle->isValid)) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD bytesRead = 0;
		if (ReadFile(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, NULL) == TRUE) {
			result = bytesRead;
		}
	}
	return(result);
}

fpl_api uint32_t fpl_WriteFileBlock32(fpl_FileHandle *fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
	uint32_t result = 0;
	if ((fileHandle != FPL_NULL) && (fileHandle->isValid)) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD bytesWritten = 0;
		if (WriteFile(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, NULL) == TRUE) {
			result = bytesWritten;
		}
	}
	return(result);
}

fpl_api void fpl_SetFilePosition32(fpl_FileHandle *fileHandle, const uint32_t position, const fpl_FilePositionMode mode) {
	if ((fileHandle != FPL_NULL) && (fileHandle->isValid)) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD moveMethod = FILE_BEGIN;
		if (mode == fpl_FilePositionMode_Current) {
			moveMethod = FILE_CURRENT;
		} else if (mode == fpl_FilePositionMode_End) {
			moveMethod = FILE_END;
		}
		SetFilePointer(win32FileHandle, (LONG)position, NULL, moveMethod);
	}
}

fpl_api uint32_t fpl_GetFilePosition32(fpl_FileHandle *fileHandle) {
	uint32_t result = 0;
	if ((fileHandle != FPL_NULL) && (fileHandle->isValid)) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		DWORD filePosition = SetFilePointer(win32FileHandle, 0L, NULL, FILE_CURRENT);
		if (filePosition != INVALID_SET_FILE_POINTER) {
			result = filePosition;
		}
	}
	return(result);
}

fpl_api void fpl_CloseFile(fpl_FileHandle *fileHandle) {
	if ((fileHandle != FPL_NULL) && (fileHandle->isValid)) {
		FPL_ASSERT(fileHandle->internalHandle != INVALID_HANDLE_VALUE);
		HANDLE win32FileHandle = (void *)fileHandle->internalHandle;
		CloseHandle(win32FileHandle);
	}
	FPL_CLEARSTRUCT_INTERNAL(fileHandle);
}

#	if defined(UNICODE)
fpl_api void fpl_GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
	wchar_t modulePath[MAX_PATH + 1];
	GetModuleFileNameW(NULL, modulePath, MAX_PATH + 1);
	fpl_WideStringToOEMString(modulePath, fpl_GetWideStringLength(modulePath), destPath, maxDestLen);
}
#	else
fpl_api void fpl_GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
	FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
	char modulePath[MAX_PATH + 1];
	GetModuleFileNameA(NULL, modulePath, MAX_PATH + 1);
	fpl_CopyString(modulePath, fpl_GetStringLength(modulePath), destPath, maxDestLen);
}
#	endif

fpl_api void fpl_CombinePath(char *destPath, const uint32_t maxDestPathLen, ...) {
	// @TODO: Implement fpl_CombinePath!
}

fpl_api char *fpl_ExtractFilePath(char *destPath, const uint32_t maxDestLen, const char *sourcePath) {
	char *result = 0;
	if (sourcePath) {
		int copyLen = 0;
		char *chPtr = (char *)sourcePath;
		while (*chPtr) {
			if (*chPtr == FPL_PATH_SEPARATOR) {
				copyLen = (int)(chPtr - sourcePath);
			}
			++chPtr;
		}
		if (copyLen) {
			fpl_CopyString(sourcePath, copyLen, destPath, maxDestLen);
		}
	}
	return(result);
}

fpl_api char *fpl_ExtractFileExtension(char *destPath, const uint32_t maxDestLen, const char *sourcePath) {
	// @TODO: Implement fpl_ExtractFileExtension!
	return FPL_NULL;
}

fpl_api char *fpl_ExtractFileName(char *destPath, const uint32_t maxDestLen, const char *sourcePath) {
	// @TODO: Implement fpl_ExtractFileName!
	return FPL_NULL;
}

fpl_api char *fpl_ChangeFileExtension(char *destPath, const uint32_t maxDestLen, const char *sourcePath) {
	// @TODO: Implement fpl_ChangeFileExtension!
	return FPL_NULL;
}

//
// Win32 Public Timing
//
fpl_internal LARGE_INTEGER fpl_Win32GetWallClock_Internal() {
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return(result);
}
fpl_api double fpl_GetHighResolutionTimeInSeconds() {
	LARGE_INTEGER clock = fpl_Win32GetWallClock_Internal();
	double result = clock.QuadPart / (double)fpl_GlobalWin32State_Internal.performanceFrequency.QuadPart;
	return(result);
}

//
// Win32 Public String
//
fpl_api void fpl_WideStringToOEMString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *oemDest, const uint32_t maxOemDestLen) {
	uint32_t requiredSize = WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
	uint32_t requiredLen = requiredSize / sizeof(char);
	FPL_ASSERT(maxOemDestLen >= (requiredLen + 1));
	WideCharToMultiByte(CP_ACP, 0, wideSource, maxWideSourceLen, oemDest, maxOemDestLen, NULL, NULL);
	oemDest[requiredLen] = 0;
}
fpl_api void fpl_WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
	uint32_t requiredSize = WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, NULL, 0, NULL, NULL);
	uint32_t requiredLen = requiredSize / sizeof(char);
	FPL_ASSERT(maxUtf8DestLen >= (requiredSize + 1));
	WideCharToMultiByte(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, NULL, NULL);
	utf8Dest[requiredLen] = 0;
}
fpl_api void fpl_OEMStringToWideString(const char *oemSource, const uint32_t oemSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	uint32_t requiredSize = MultiByteToWideChar(CP_ACP, 0, oemSource, oemSourceLen, NULL, 0);
	uint32_t requiredLen = requiredSize / sizeof(wchar_t);
	FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
	MultiByteToWideChar(CP_ACP, 0, oemSource, oemSourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
}
fpl_api void fpl_UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
	uint32_t requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, NULL, 0);
	uint32_t requiredLen = requiredSize / sizeof(wchar_t);
	FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
	MultiByteToWideChar(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
	wideDest[requiredLen] = 0;
}

//
// Win32 Public Core
//
fpl_api fpl_LibraryHandle fpl_LoadLibrary(const char *libraryFilePath) {
	fpl_LibraryHandle result = { 0 };
	HMODULE libModule = LoadLibraryA(libraryFilePath);
	if (libModule != FPL_NULL) {
		result.internalHandle = (void *)libModule;
		result.isValid = fpl_true;
	}
	return(result);
}
fpl_api void *fpl_GetLibraryProcAddress(fpl_LibraryHandle *handle, const char *name) {
	void *result = FPL_NULL;
	if (handle != FPL_NULL) {
		if (handle->isValid) {
			FPL_ASSERT(handle->internalHandle != FPL_NULL);
			HMODULE libModule = (HMODULE)handle;
			result = GetProcAddress(libModule, name);
		}
	}
	return(result);
}
fpl_api void fpl_ReleaseLibrary(fpl_LibraryHandle *handle) {
	if (handle != FPL_NULL) {
		if (handle->isValid) {
			FPL_ASSERT(handle->internalHandle != FPL_NULL);
			HMODULE libModule = (HMODULE)handle->internalHandle;
			FreeLibrary(libModule);
		}
		FPL_CLEARSTRUCT_INTERNAL(handle);
	}
}

//
// Win32 Public Window
//
#	if FPL_ENABLE_WINDOW

#		if FPL_ENABLE_OPENGL
fpl_api void fpl_WindowFlip() {
	SwapBuffers(fpl_GlobalWin32State_Internal.window.deviceContext);
}
#		else
fpl_api void fpl_WindowFlip() {
}
#		endif // FPL_ENABLE_OPENGL

fpl_api int32_t fpl_GetWindowWidth() {
	int32_t result = 0;
	RECT windowRect;
	if (GetClientRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
		result = (windowRect.right - windowRect.left) + 1;
	}
	return(result);
}
fpl_api int32_t fpl_GetWindowHeight() {
	int32_t result = 0;
	RECT windowRect;
	if (GetClientRect(fpl_GlobalWin32State_Internal.window.windowHandle, &windowRect)) {
		result = (windowRect.bottom - windowRect.top) + 1;
	}
	return(result);
}

fpl_api void fpl_ShowWindowMouseCursor(const fpl_bool32 value) {
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	win32State->window.isCursorActive = value;
}

fpl_internal void fpl_Win32PushMouseEvent_Internal(const fpl_MouseEventType mouseEventType, const fpl_MouseButtonType mouseButton, const LPARAM lParam, const WPARAM wParam) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT_INTERNAL(&newEvent);
	newEvent.type = fpl_EventType_Mouse;
	newEvent.mouse.type = mouseEventType;
	newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
	newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
	newEvent.mouse.mouseButton = mouseButton;
	if (mouseEventType == fpl_MouseEventType_Wheel) {
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
	}
	fpl_PushEvent_Internal(&newEvent);
}

fpl_internal fpl_Key fpl_Win32MapVirtualKey_Internal(const uint64_t keyCode) {
	switch (keyCode) {
		case VK_BACK:
			return fpl_Key_Backspace;
		case VK_TAB:
			return fpl_Key_Tab;

		case VK_CLEAR:
			return fpl_Key_Clear;
		case VK_RETURN:
			return fpl_Key_Enter;

		case VK_SHIFT:
			return fpl_Key_Shift;
		case VK_CONTROL:
			return fpl_Key_Control;
		case VK_MENU:
			return fpl_Key_Alt;
		case VK_PAUSE:
			return fpl_Key_Pause;
		case VK_CAPITAL:
			return fpl_Key_CapsLock;

		case VK_ESCAPE:
			return fpl_Key_Escape;
		case VK_SPACE:
			return fpl_Key_Space;
		case VK_PRIOR:
			return fpl_Key_PageUp;
		case VK_NEXT:
			return fpl_Key_PageDown;
		case VK_END:
			return fpl_Key_End;
		case VK_HOME:
			return fpl_Key_Home;
		case VK_LEFT:
			return fpl_Key_Left;
		case VK_UP:
			return fpl_Key_Up;
		case VK_RIGHT:
			return fpl_Key_Right;
		case VK_DOWN:
			return fpl_Key_Down;
		case VK_SELECT:
			return fpl_Key_Select;
		case VK_PRINT:
			return fpl_Key_Print;
		case VK_EXECUTE:
			return fpl_Key_Execute;
		case VK_SNAPSHOT:
			return fpl_Key_Snapshot;
		case VK_INSERT:
			return fpl_Key_Insert;
		case VK_DELETE:
			return fpl_Key_Delete;
		case VK_HELP:
			return fpl_Key_Help;

		case 0x30:
			return fpl_Key_0;
		case 0x31:
			return fpl_Key_1;
		case 0x32:
			return fpl_Key_2;
		case 0x33:
			return fpl_Key_3;
		case 0x34:
			return fpl_Key_4;
		case 0x35:
			return fpl_Key_5;
		case 0x36:
			return fpl_Key_6;
		case 0x37:
			return fpl_Key_7;
		case 0x38:
			return fpl_Key_8;
		case 0x39:
			return fpl_Key_9;

		case 0x41:
			return fpl_Key_A;
		case 0x42:
			return fpl_Key_B;
		case 0x43:
			return fpl_Key_C;
		case 0x44:
			return fpl_Key_D;
		case 0x45:
			return fpl_Key_E;
		case 0x46:
			return fpl_Key_F;
		case 0x47:
			return fpl_Key_G;
		case 0x48:
			return fpl_Key_H;
		case 0x49:
			return fpl_Key_I;
		case 0x4A:
			return fpl_Key_J;
		case 0x4B:
			return fpl_Key_K;
		case 0x4C:
			return fpl_Key_L;
		case 0x4D:
			return fpl_Key_M;
		case 0x4E:
			return fpl_Key_N;
		case 0x4F:
			return fpl_Key_O;
		case 0x50:
			return fpl_Key_P;
		case 0x51:
			return fpl_Key_Q;
		case 0x52:
			return fpl_Key_R;
		case 0x53:
			return fpl_Key_S;
		case 0x54:
			return fpl_Key_T;
		case 0x55:
			return fpl_Key_U;
		case 0x56:
			return fpl_Key_V;
		case 0x57:
			return fpl_Key_W;
		case 0x58:
			return fpl_Key_X;
		case 0x59:
			return fpl_Key_Y;
		case 0x5A:
			return fpl_Key_Z;

		case VK_LWIN:
			return fpl_Key_LeftWin;
		case VK_RWIN:
			return fpl_Key_RightWin;
		case VK_APPS:
			return fpl_Key_Apps;

		case VK_SLEEP:
			return fpl_Key_Sleep;
		case VK_NUMPAD0:
			return fpl_Key_NumPad0;
		case VK_NUMPAD1:
			return fpl_Key_NumPad1;
		case VK_NUMPAD2:
			return fpl_Key_NumPad2;
		case VK_NUMPAD3:
			return fpl_Key_NumPad3;
		case VK_NUMPAD4:
			return fpl_Key_NumPad4;
		case VK_NUMPAD5:
			return fpl_Key_NumPad5;
		case VK_NUMPAD6:
			return fpl_Key_NumPad6;
		case VK_NUMPAD7:
			return fpl_Key_NumPad7;
		case VK_NUMPAD8:
			return fpl_Key_NumPad8;
		case VK_NUMPAD9:
			return fpl_Key_NumPad9;
		case VK_MULTIPLY:
			return fpl_Key_Multiply;
		case VK_ADD:
			return fpl_Key_Add;
		case VK_SEPARATOR:
			return fpl_Key_Separator;
		case VK_SUBTRACT:
			return fpl_Key_Substract;
		case VK_DECIMAL:
			return fpl_Key_Decimal;
		case VK_DIVIDE:
			return fpl_Key_Divide;
		case VK_F1:
			return fpl_Key_F1;
		case VK_F2:
			return fpl_Key_F2;
		case VK_F3:
			return fpl_Key_F3;
		case VK_F4:
			return fpl_Key_F4;
		case VK_F5:
			return fpl_Key_F5;
		case VK_F6:
			return fpl_Key_F6;
		case VK_F7:
			return fpl_Key_F7;
		case VK_F8:
			return fpl_Key_F8;
		case VK_F9:
			return fpl_Key_F9;
		case VK_F10:
			return fpl_Key_F10;
		case VK_F11:
			return fpl_Key_F11;
		case VK_F12:
			return fpl_Key_F12;
		case VK_F13:
			return fpl_Key_F13;
		case VK_F14:
			return fpl_Key_F14;
		case VK_F15:
			return fpl_Key_F15;
		case VK_F16:
			return fpl_Key_F16;
		case VK_F17:
			return fpl_Key_F17;
		case VK_F18:
			return fpl_Key_F18;
		case VK_F19:
			return fpl_Key_F19;
		case VK_F20:
			return fpl_Key_F20;
		case VK_F21:
			return fpl_Key_F21;
		case VK_F22:
			return fpl_Key_F22;
		case VK_F23:
			return fpl_Key_F23;
		case VK_F24:
			return fpl_Key_F24;

		case VK_LSHIFT:
			return fpl_Key_LeftShift;
		case VK_RSHIFT:
			return fpl_Key_RightShift;
		case VK_LCONTROL:
			return fpl_Key_LeftControl;
		case VK_RCONTROL:
			return fpl_Key_RightControl;
		case VK_LMENU:
			return fpl_Key_LeftAlt;
		case VK_RMENU:
			return fpl_Key_RightAlt;

		default:
			return fpl_Key_None;
	}
}

fpl_internal void fpl_Win32PushKeyboardEvent_Internal(const fpl_KeyboardEventType keyboardEventType, const uint64_t keyCode, const fpl_KeyboardModifierType modifiers, const fpl_bool32 isDown) {
	fpl_Event newEvent;
	FPL_CLEARSTRUCT_INTERNAL(&newEvent);
	newEvent.type = fpl_EventType_Keyboard;
	newEvent.keyboard.keyCode = keyCode;
	newEvent.keyboard.mappedKey = fpl_Win32MapVirtualKey_Internal(keyCode);
	newEvent.keyboard.type = keyboardEventType;
	newEvent.keyboard.modifiers = modifiers;
	fpl_PushEvent_Internal(&newEvent);
}

fpl_internal fpl_bool32 fpl_Win32IsKeyDown_Internal(const uint64_t keyCode) {
	fpl_bool32 result = GetAsyncKeyState((int)keyCode) & 0x8000;
	return(result);
}

LRESULT CALLBACK fpl_Win32MessageProc_Internal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	switch (msg) {
		case WM_DESTROY:
		case WM_CLOSE:
		{
			win32State->window.isRunning = fpl_false;
		} break;

		case WM_SIZE:
		{
			fpl_Event newEvent;
			FPL_CLEARSTRUCT_INTERNAL(&newEvent);
			newEvent.type = fpl_EventType_Window;
			newEvent.window.type = fpl_WindowEventType_Resize;
			newEvent.window.width = LOWORD(lParam);
			newEvent.window.height = HIWORD(lParam);
			fpl_PushEvent_Internal(&newEvent);
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint64_t keyCode = wParam;
			fpl_bool32 wasDown = ((lParam & (1 << 30)) != 0);
			fpl_bool32 isDown = ((lParam & (1 << 31)) == 0);

			fpl_bool32 altKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_MENU);
			fpl_bool32 shiftKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LSHIFT);
			fpl_bool32 ctrlKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LCONTROL);
			fpl_bool32 superKeyWasDown = fpl_Win32IsKeyDown_Internal(VK_LMENU);

			fpl_KeyboardEventType keyEventType = isDown ? fpl_KeyboardEventType_KeyDown : fpl_KeyboardEventType_KeyUp;
			fpl_KeyboardModifierType modifiers = fpl_KeyboardModifierType_None;
			if (altKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Alt;
			}
			if (shiftKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Shift;
			}
			if (ctrlKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Ctrl;
			}
			if (superKeyWasDown) {
				modifiers |= fpl_KeyboardModifierType_Super;
			}
			fpl_Win32PushKeyboardEvent_Internal(keyEventType, keyCode, modifiers, isDown);

			if (wasDown != isDown) {
				if (isDown) {
					if (keyCode == VK_F4 && altKeyWasDown) {
						win32State->window.isRunning = 0;
					}
				}
			}
			result = 1;
		} break;

		case WM_CHAR:
		{
			uint64_t keyCode = wParam;
			fpl_KeyboardModifierType modifiers = { 0 };
			fpl_Win32PushKeyboardEvent_Internal(fpl_KeyboardEventType_Char, keyCode, modifiers, 0);
			result = 1;
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_LBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Left, lParam, wParam);
			result = 1;
		} break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_RBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Right, lParam, wParam);
			result = 1;
		} break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			fpl_MouseEventType mouseEventType;
			if (msg == WM_MBUTTONDOWN) {
				mouseEventType = fpl_MouseEventType_ButtonDown;
			} else {
				mouseEventType = fpl_MouseEventType_ButtonUp;
			}
			fpl_Win32PushMouseEvent_Internal(mouseEventType, fpl_MouseButtonType_Middle, lParam, wParam);
			result = 1;
		} break;
		case WM_MOUSEMOVE:
		{
			fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType_Move, fpl_MouseButtonType_None, lParam, wParam);
			result = 1;
		} break;
		case WM_MOUSEWHEEL:
		{
			fpl_Win32PushMouseEvent_Internal(fpl_MouseEventType_Wheel, fpl_MouseButtonType_None, lParam, wParam);
			result = 1;
		} break;

		case WM_SETCURSOR:
		{
			if (win32State->window.isCursorActive) {
				SetCursor(win32State->window.defaultCursor);
			} else {
				SetCursor(NULL);
			}
			result = 1;
		} break;

		default:
			result = DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return (result);
}

fpl_api fpl_bool32 fpl_WindowUpdate() {
	fpl_bool32 result = fpl_false;
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	if (win32State->window.windowHandle != 0) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		result = win32State->window.isRunning;
	}
	return(result);
}

fpl_api fpl_bool32 fpl_IsWindowRunning() {
	fpl_bool32 result = fpl_GlobalWin32State_Internal.window.isRunning;
	return(result);
}
#	endif // FPL_ENABLE_WINDOW

#	if FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL
fpl_internal fpl_bool32 fpl_Win32CreateOpenGLContext_Internal(fpl_Win32State_Internal *win32State) {
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int pixelFormat = ChoosePixelFormat(win32State->window.deviceContext, &pfd);
	if (pixelFormat == 0) {
		// @TODO: Log error
		return fpl_false;
	}
	if (!SetPixelFormat(win32State->window.deviceContext, pixelFormat, &pfd)) {
		// @TODO: Log error
		return fpl_false;
	}

	win32State->opengl.renderingContext = wglCreateContext(win32State->window.deviceContext);
	if (!win32State->opengl.renderingContext) {
		// @TODO: Log error
		return fpl_false;
	}

	if (!wglMakeCurrent(win32State->window.deviceContext, win32State->opengl.renderingContext)) {
		// @TODO: Log error
		return fpl_false;
	}

	return fpl_true;
}

fpl_internal void fpl_Win32ReleaseOpenGLContext_Internal(fpl_Win32State_Internal *win32State) {
	if (win32State->opengl.renderingContext) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(win32State->opengl.renderingContext);
		win32State->opengl.renderingContext = NULL;
	}
}
#	else
#		define fpl_Win32CreateOpenGLContext_Internal(win32State) (fpl_true)
#		define fpl_Win32ReleaseOpenGLContext_Internal(win32State)
#	endif // FPL_ENABLE_WINDOW && FPL_ENABLE_OPENGL

#	if FPL_ENABLE_WINDOW
fpl_internal fpl_bool32 fpl_InitWindow_Internal(fpl_Win32State_Internal *win32State, const fpl_InitFlag initFlags) {
	// Register window class
	WNDCLASSEX windowClass;
	FPL_CLEARSTRUCT_INTERNAL(&windowClass);
	windowClass.hInstance = win32State->appInstance;
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowClass.lpszClassName = WIN32_CLASSNAME;
	windowClass.lpfnWndProc = fpl_Win32MessageProc_Internal;
	if (RegisterClassEx(&windowClass) == 0) {
		// @TODO: Log error
		return fpl_false;
	}
	fpl_Win32StringCopy(windowClass.lpszClassName, fpl_Win32GetStringLength(windowClass.lpszClassName), win32State->window.windowClass, FPL_ARRAYCOUNT(win32State->window.windowClass));

	// Create window
	win32State->window.windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, windowClass.lpszClassName, WIN32_UNNAMED_WINDOW, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, FPL_DEFAULT_WINDOW_WIDTH, FPL_DEFAULT_WINDOW_HEIGHT, NULL, NULL, windowClass.hInstance, NULL);
	if (win32State->window.windowHandle == NULL) {
		// @TODO: Log error
		return fpl_false;
	}

	// Get device context so we can swap the back and front buffer
	win32State->window.deviceContext = GetDC(win32State->window.windowHandle);
	if (win32State->window.deviceContext == NULL) {
		// @TODO: Log error
		return fpl_false;
	}

	// Create opengl rendering context if required
	if (initFlags & fpl_InitFlag_VideoOpenGL) {
		fpl_bool32 openglResult = fpl_Win32CreateOpenGLContext_Internal(win32State);
		if (!openglResult) {
			// @TODO: Log error
			return fpl_false;
		}
	}

	void *eventQueueMemory = fpl_AllocateAlignedMemory(sizeof(fpl_EventQueue_Internal), 16);
	fpl_GlobalEventQueue_Internal = (fpl_EventQueue_Internal *)eventQueueMemory;

	// Show window
	ShowWindow(win32State->window.windowHandle, SW_SHOW);
	UpdateWindow(win32State->window.windowHandle);

	// Cursor is visible at start
	win32State->window.defaultCursor = windowClass.hCursor;
	win32State->window.isCursorActive = fpl_true;
	win32State->window.isRunning = fpl_true;

	return fpl_true;
}

fpl_internal void fpl_ReleaseWindow_Internal(fpl_Win32State_Internal *win32State) {
	fpl_Win32ReleaseOpenGLContext_Internal(win32State);

	if (win32State->window.deviceContext != NULL) {
		ReleaseDC(win32State->window.windowHandle, win32State->window.deviceContext);
		win32State->window.deviceContext = NULL;
	}

	if (win32State->window.windowHandle != NULL) {
		DestroyWindow(win32State->window.windowHandle);
		win32State->window.windowHandle = NULL;
		UnregisterClass(win32State->window.windowClass, win32State->appInstance);
	}

	fpl_FreeAlignedMemory(fpl_GlobalEventQueue_Internal);
}
#	else
#		define fpl_InitWindow_Internal(win32State, initFlags) (fpl_true)
#		define fpl_ReleaseWindow_Internal(void)
#	endif // FPL_ENABLE_WINDOW

fpl_api fpl_bool32 fpl_Init(const fpl_InitFlag initFlags) {
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	FPL_ASSERT(!win32State->isInitialized);

	// Timing
	QueryPerformanceFrequency(&win32State->performanceFrequency);

	fpl_InitFlag usedInitFlags = initFlags;
	if (usedInitFlags & fpl_InitFlag_VideoOpenGL) {
		usedInitFlags |= fpl_InitFlag_Window;
	}

	if (usedInitFlags & fpl_InitFlag_Window) {
		if (!fpl_InitWindow_Internal(win32State, usedInitFlags)) {
			return fpl_false;
		}
	}

	win32State->isInitialized = fpl_true;

	return (fpl_true);
}

fpl_api void fpl_Release() {
	fpl_Win32State_Internal *win32State = &fpl_GlobalWin32State_Internal;
	FPL_ASSERT(win32State != NULL);
	FPL_ASSERT(win32State->isInitialized);
	fpl_ReleaseWindow_Internal(win32State);
	win32State->isInitialized = fpl_false;
}

//
// Win32 Entry-Point
// This is a bit ugly because:
// - Support for Window and Console application
// - Support for Unicode and Ansi
// - Support for disabled C-Runtime-Library
// @NOTE: If possible clean this up
//
#	if FPL_ENABLE_WINDOW

#		if defined(UNICODE)
int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO: Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#if			!FPL_ENABLE_C_RUNTIME_LIBRARY
void __stdcall WinMainCRTStartup() {
	int result = wWinMain(GetModuleHandleW(0), 0, 0, 0);
	ExitProcess(result);
}
#			endif // !FPL_ENABLE_C_RUNTIME_LIBRARY
#		else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl_GlobalWin32State_Internal.appInstance = appInstance;
	// @TODO: Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#if			!FPL_ENABLE_C_RUNTIME_LIBRARY
void __stdcall WinMainCRTStartup() {
	int result = WinMain(GetModuleHandleA(0), 0, 0, 0);
	ExitProcess(result);
}
#			endif // !FPL_ENABLE_C_RUNTIME_LIBRARY
#		endif // defined(UNICODE)

#	else

#if		!FPL_ENABLE_C_RUNTIME_LIBRARY
#			if defined(UNICODE)
void __stdcall mainCRTStartup() {
	// @TODO: Parse command line parameters
	int result = main(0, 0);
	ExitProcess(result);
}
#			else
void __stdcall mainCRTStartup() {
	// @TODO: Parse command line parameters
	int result = main(0, 0);
	ExitProcess(result);
}
#			endif
#		else
// The main() entry point is used directly
#		endif // !FPL_ENABLE_C_RUNTIME_LIBRARY	

#	endif // FPL_ENABLE_WINDOW

#elif defined(FPL_PLATFORM_LINUX) // FPL_PLATFORM_WINDOWS
	//
	// Linux platform implementation
	//
#	error "Please define at least the entry point for the linux platform!"
#elif defined(FPL_PLATFORM_UNIX) // FPL_PLATFORM_LINUX
	//
	// Unix platform implementation
	//
#	error "Please define at least the entry point for the unix platform!"
#else // defined(FPL_PLATFORM_UNIX)
#	error "Unsupported Platform!"
#endif // !defined(FPL_PLATFORM_UNIX)

#endif // defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)