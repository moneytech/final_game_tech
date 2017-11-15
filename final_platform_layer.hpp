/**
* @file final_platform_layer.hpp
* @version v0.4.0 alpha
* @author Torsten Spaete
* @brief Final Platform Layer (FPL) - A Open source C++ single file header platform abstraction layer library.
*
* This library is designed to abstract the underlying platform to a very simple and easy to use api.
* The only dependencies are built-in operatoring system libraries and the C++ runtime library.
*
* The main focus is game/simulation development, so the default settings will create a window and setup a opengl rendering context.
*
* @mainpage
* Summary of the Final Platform Layer (FPL) project.
* Please see @ref final_platform_layer.hpp for more details.
**/

/*
final_platform_layer.hpp
Open-Source C++ Single-File Header-Library by Torsten Spaete

# ABOUT

This library is designed to abstract the underlying platform to a very simple and easy to use api.
The only dependencies are built-in operatoring system libraries and the C runtime library.

The main focus is game development, so the default settings will create a window and setup a opengl rendering context.

It works very well with other libraries like for example:

- STB Libraries
- Standard C++ Library (STL)
- Glew/Glad
- Box2D
- GLM
- ImGUI
- etc.

# HOW TO USE

// In one of your C++ translation units include this:
#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

// You can then #include this file in any other C++ source or header file as you would with any other header file.

// Provide the typical main entry point
int main(int argc, char **args) {
}

// Initialize the library and release it when you are done
fpl::InitPlatform(fpl::InitFlags::All);
...
fpl::ReleasePlatform();

# EXAMPLES

## Simple OpenGL 1.x Triangle

#define FPL_IMPLEMENTATION
#include "final_platform_layer.hpp"

using namespace fpl;
using namespace fpl::memory;
using namespace fpl::window;

int main(int argc, char **args) {
	int result = 0;
	if (InitPlatform(InitFlags::Video)) {
		glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
		while (WindowUpdate()) {
			WindowSize windowArea = GetWindowArea();
			glViewport(0, 0, windowArea.width, windowArea.height);
			glClear(GL_COLOR_BUFFER_BIT);
			glBegin(GL_TRIANGLES);
			glVertex2f(0.0f, 0.5f);
			glVertex2f(-0.5f, -0.5f);
			glVertex2f(0.5f, -0.5f);
			glEnd();
			WindowFlip();
		}
		ReleasePlatform();
		result = 0;
	} else {
		result = -1;
	}
	return(result);
}

# HOW TO COMPILE

## Win32

	* Link to kernel32.lib
	* Link to user32.lib
	* Link to shell32.lib
	* Link to opengl32.lib (Only needed if you use opengl)

# OPTIONS

Define these options before including this file.

// #define FPL_API_AS_PRIVATE
	Define this to make all functions be private (static)

// #define FPL_NO_ASSERTIONS
	Define this to disable assertions.

// #define FPL_NO_C_ASSERT
	Define this to disable C runtime assert.

// #define FPL_NO_WINDOW
	Define this to disable window support entirely.

// #define FPL_NO_VIDEO_OPENGL
	Define this to disable opengl rendering support entirely.

// #define FPL_NO_VIDEO_SOFTWARE
	Define this to disable software rendering support entirely.

// #define FPL_NO_MULTIPLE_ERRORSTATES
	Define this to use a single error state for GetPlatformLastError() instead of multiple ones.

// #define FPL_NO_ERROR_IN_CONSOLE
	Define this to disable printing any platform specific errors to the console.

// #define FPL_AUTO_NAMESPACE
	Define this to include all required namespaces automatically.

# FEATURES

[x] Creating a fixed or resizeable window
[x] Handling window, keyboard, mouse events
[x] Enable/Disable fullscreen
[x] Full event buffering
[x] Polling gamepad informations
[x] Clipboard string reading and writing
[ ] Multi-monitor support for window and fullscreen toggling and creation

[x] Creating a 1.x opengl rendering context
[x] Creating a 3.x + opengl rendering context
[x] Software rendering context

[ ] Audio playback using OS native libraries

[x] Memory allocation and de-allocation with custom alignment support
[x] Atomic operations
[x] Path functions
[x] File functions
[x] Hardware functions
[x] String conversion functions
[x] Thread/Mutex/Signal handling

# SUPPORTED ARCHITECTURES

[x] x86
[x] x86_64

# SUPPORTED COMPILERS

[X] Compiles with MSVC
[ ] Compiles with MingW
[ ] Compiles with GCC/G++
[ ] Compiles with Clang
[ ] Compiles with Intel C/C++ Compiler

# SUPPORTED PLATFORMS

[x] Win32
[ ] Linux
[ ] Unix/Posix
[ ] OSX (Not sure)

# LICENSE

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

# VERSION HISTORY

- v0.4.0 alpha:
	* Changed: All FPL_ENABLE_ defines are internal now, the caller must use FPL_NO_ or FPL_YES_ respectivily.
	* Changed: AtomicCompareExchange* is now AtomicCompareAndExchange*
	* Changed: InitFlags::VideoOpenGL is now InitFlags::Video
	* Added: Software rendering support
	* Added: VideoDriverType enumeration for selecting the active video driver
	* Added: video::GetVideoBackBuffer with [Win32] implementation
	* Added: video::ResizeVideoBackBuffer with [Win32] implementation
	* Added: FPL_PETABYTES macro
	* Added: FPL_EXABYTES macro
	* Added: FPL_ZETTABYTES macro
	* Added: FPL_YOTTABYTES macro
	* Added: FPL_MIN macro
	* Added: FPL_MAX macro
	* Added: MutexCreate with [Win32] implementation
	* Added: MutexDestroy with [Win32] implementation
	* Added: MutexLock with [Win32] implementation
	* Added: MutexUnlock with [Win32] implementation
	* Added: SignalCreate with [Win32] implementation
	* Added: SignalDestroy with [Win32] implementation
	* Added: SignalWait with [Win32] implementation
	* Added: SignalWakeUp with [Win32] implementation
	* Added: GetClipboardAnsiText with [Win32] implementation
	* Added: GetClipboardWideText with [Win32] implementation
	* Added: SetClipboardText with [Win32] implementation for ansi and wide strings
	* Added [MSVC]: AtomicExchangeS32 (Signed integer)
	* Added [MSVC]: AtomicExchangeS64 (Signed integer)
	* Added [MSVC]: AtomicAddS32 (Signed integer)
	* Added [MSVC]: AtomicAddS64 (Signed integer)
	* Added [MSVC]: AtomicCompareExchangeS32 (Signed integer)
	* Added [MSVC]: AtomicCompareExchangeS64 (Signed integer)
	* Fixed [MSVC]: AtomicExchangeU32 was not using unsigned intrinsic
	* Fixed [MSVC]: AtomicExchangeU64 was not using unsigned intrinsic
	* Fixed [MSVC]: AtomicAddU32 was not using unsigned intrinsic
	* Fixed [MSVC]: AtomicAddU64 was not using unsigned intrinsic
	* Fixed [MSVC]: AtomicCompareExchangeU32 was not using unsigned intrinsic
	* Fixed [MSVC]: AtomicCompareExchangeU64 was not using unsigned intrinsic
	* Implemented [Win32]: GetProcessorCoreCount
	* Implemented [Win32]: Main thread infos
	* Performance [Win32]: GetProcessorName (3 loop iterations at max)
- v0.3.6 alpha:
	* Cleanup: All win32 functions are macro calls now (prepare for dynamic function loading)
	* Fixed: FPL_ENABLE_WINDOW was enabling window features even when it was deactivated by the caller
- v0.3.5 alpha:
	* Renamed: All memory/library/threading functions
	* Removed: FPL_ENABLE_PUSHMEMORY removed entirely
- v0.3.4 alpha:
	* Renamed: CopyFile/DeleteFile/All memory functions renamed (Stupid win32!)
	* Renamed: All internal opengl defines renamed, so that it wont conflict with other libraries
	* Fixed: [Win32] strings::All Wide conversions was not working properly
	* Removed: [Win32] Undefs for CopyFile
	* Changed: [Win32/OpenGL] Test for already included gl.h
- v0.3.3 alpha:
	* Basic threading creation and handling
	* Fixed strings::All Wide convertions was not working properly
- v0.3.2 alpha:
	* Introduced: Automatic namespace inclusion (FPL_AUTO_NAMESPACE)
	* Introduced: Push memory (FPL_ENABLE_PUSHMEMORY)
	* Signature changed for: ExtractFilePath/ChangeFileExtension (source first, destination second)
	* Window features not not compiled out anymore when FPL_ENABLE_WINDOW is 0
	* New overloaded CombinePath without any destination arguments
	* New: AllocateStackMemory function
	* Optional destination arguments for: GetExecutableFilePath/GetHomePath/ChangeFileExtension/CombinePath
	* Fixed strings::CopyAnsiString/CopyWideString was not returning the correct value
- v0.3.1 alpha:
	* All types/structs/fields/functions documented
	* [Win32] Fixed legacy opengl (GL_INVALID_OPERATION)
- v0.3.0 alpha:
	* Updated documentation a lot
	* [Win32] Support for WGL opengl profile selection
- v0.2.6 alpha:
	* Added memory::CopyMemory
	* Added fpl::GetLastError and fpl::GetLastErrorCount for proper error handling
	* Added files::CreateBinaryFile and files::OpenBinaryFile for wide file paths
	* Added basic support for creating a modern opengl rendering context, see VideoCompabilityProfile in VideoSettings
	* Added support for enabling opengl vsync through WGL
	* Returns char * for all paths:: get like functions
	* Returns char/wchar_t * for all strings:: functions
	* Fixed files::CreateBinaryFile was never able to overwrite the file.
	* Fixed #include was in some namespaces defined
	* Fixed files::ClearMemory was wrong
	* Replaced all const constants with constexpr
	* Removed template code / Replaced it with macros
- v0.2.5 alpha:
	* Added CreateDirectories
	* Returns char * for all path get like functions
	* Fixed CreateBinaryFile was never able to overwrite the file.
- v.2.4 alpha:
	* Changed to a doxygen + vc complaint documentation style
	* CopyFile2, DeleteFile2 and CloseFile2 are now CopyFile, DeleteFile, CloseFile
- v0.2.3 alpha:
	* Support for doxygen in documentations
- v0.2.2 alpha:
	* Added XInput support
- v0.2.1 alpha:
	* Changed a lot of pointer arguments to reference
	* Added gamepad event structures
- v0.2 alpha:
	* Dropped C support and moved to a more C++ ish api
	* Dropped no C-Runtime support
- v0.1 alpha:
	* Initial version

# TODO

	* Compress namespaces down to one for the api and one for implementation.
	* Change most assertions to normal comparisons and make it rock solid, so it wont crash for the most part. Returning nullptr or empty is much more preferred.
	* Make linking to opengl32 not required at all (Replace wgl and gdi calls with macro calls)
	* Make linking to user32 and shell32 not required at all (Load the functions directly from the libraries, but still require kernel32)
	* Feature completeness for Win32 (Clipboard, Audio)
	* Support/Test other compilers (MingW, Clang, Intel)
	* Linux/Unix platform support (Look out for .DS_Store and . files/folders, handle it properly)
	* REFERENCE.MD generation using doxygen

*/

#ifndef FPL_INCLUDE_HPP
#define FPL_INCLUDE_HPP

// ****************************************************************************
//
// Header
//
// ****************************************************************************

//
// Platform detection
//
#if defined(_WIN32)
#	define FPL_PLATFORM_WIN32
#	define FPL_PLATFORM_NAME "Win32"
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define FPL_PLATFORM_LINUX
#	define FPL_PLATFORM_NAME "Linux"
#elif defined(__unix__) || defined(_POSIX_VERSION)
#	define FPL_PLATFORM_UNIX
#	define FPL_PLATFORM_NAME "Unix"
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

	// Disable noexcept compiler warning for C++
#	pragma warning( disable : 4577 )

	//! Visual studio compiler detected
#	define FPL_COMPILER_MSVC
#	if defined(_DEBUG) || (!defined(NDEBUG))
		//! Debug mode detected
#		define FPL_DEBUG
#	else
		//! Non-debug (Release) mode detected
#		define FPL_RELEASE
#	endif
#elif defined(__llvm__)
	//! LLVM compiler detected
#	define FPL_COMPILER_LLVM
#elif defined(__clang__)
	//! CLANG compiler detected
#	define FPL_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
	//! GCC compiler detected
#	define FPL_COMPILER_GCC
#elif defined(__MINGW32__)
	//! MingW compiler detected
#	define FPL_COMPILER_MINGW
#elif defined(__INTEL_COMPILER) || defined(__ICC)
	//! Intel compiler detected
#	define FPL_COMPILER_INTEL
#else
	//! No compiler detected
#	define FPL_COMPILER_UNKNOWN
#endif

//
// Options & Feature detection
//
#if !defined(FPL_NO_ASSERTIONS)
#	if defined(FPL_DEBUG)
		//! Assertions enabled in debug mode by default
#		define FPL_ENABLE_ASSERTIONS
#	endif
#endif
#if defined(FPL_ENABLE_ASSERTIONS)
#	if !defined(FPL_NO_C_ASSERT)
		//! C-Runtime assertions enabled by default
#		define FPL_ENABLE_C_ASSERT
#	endif
#endif

#if !defined(FPL_NO_WINDOW)
	//! Window support enabled by default
#	define FPL_SUPPORT_WINDOW
#endif

#if !defined(FPL_NO_VIDEO_OPENGL)
	//! OpenGL support enabled by default
#	define FPL_SUPPORT_VIDEO_OPENGL
#endif
#if !defined(FPL_NO_VIDEO_SOFTWARE)
	//! Software rendering support enabled by default
#	define FPL_SUPPORT_VIDEO_SOFTWARE
#endif

#if !defined(FPL_SUPPORT_WINDOW)
#	if defined(FPL_SUPPORT_VIDEO_OPENGL)
#		undef FPL_SUPPORT_VIDEO_OPENGL
#	endif
#	if defined(FPL_SUPPORT_VIDEO_SOFTWARE)
#		undef FPL_SUPPORT_VIDEO_SOFTWARE
#	endif
#endif

#if defined(FPL_SUPPORT_WINDOW)
	//! Enable window
#	define FPL_ENABLE_WINDOW
#endif
#if defined(FPL_SUPPORT_VIDEO_OPENGL)
	//! Enable opengl hardware rendering
#	define FPL_ENABLE_VIDEO_OPENGL
#endif
#if defined(FPL_SUPPORT_VIDEO_SOFTWARE)
	//! Enable software rendering
#	define FPL_ENABLE_VIDEO_SOFTWARE
#endif

#if !defined(FPL_NO_ERROR_IN_CONSOLE)
	//! Write errors in console
#	define FPL_ENABLE_ERROR_IN_CONSOLE
#endif
#if !defined(FPL_NO_MULTIPLE_ERRORSTATES)
	//! Allow multiple error states
#	define FPL_ENABLE_MULTIPLE_ERRORSTATES
#endif
#if defined(FPL_AUTO_NAMESPACE)
	//! Expand namespaces
#	define FPL_ENABLE_AUTO_NAMESPACE
#endif

//
// Static/Inline/Extern/Internal
//
//! Global variable
#define fpl_globalvar static
//! Private/Internal function
#define fpl_internal static
#if defined(FPL_API_AS_PRIVATE)
	//! Private api call
#	define fpl_api static
#else
	//! Public api call
#	define fpl_api extern
#endif // defined(FPL_API_AS_PRIVATE)

//
// Assertions
//
#if defined(FPL_ENABLE_ASSERTIONS)
#	if defined(FPL_ENABLE_C_ASSERT)
#		include <assert.h>
		//! Runtime assert (C Runtime)
#		define FPL_ASSERT(exp) assert(exp)
		//! Compile error assert (C Runtime)
#		define FPL_STATICASSERT(exp) static_assert(exp, "static_assert")
#	else
		//! Runtime assert
#		define FPL_ASSERT(exp) if(!(exp)) {*(int *)0 = 0;}
		//! Compile error assert
#		define FPL_STATICASSERT_INTERNAL(exp, line) \
			int fpl_static_assert_##line(int static_assert_failed[(exp)?1:-1])
#		define FPL_STATICASSERT(exp) \
			FPL_STATICASSERT_INTERNAL(exp, __LINE__)
#	endif // defined(FPL_ENABLE_C_ASSERT)
#else
	//! Runtime assertions disabled
#	define FPL_ASSERT(exp)
	//! Compile time assertions disabled
#	define FPL_STATICASSERT(exp)
#endif // defined(FPL_ENABLE_ASSERTIONS)

//
// Macro functions
//
//! Returns the element count from a static array,
#define FPL_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

//! Returns the offset in bytes to a field in a structure
#define FPL_OFFSETOF(type, field) ((type *)((void *)0)->field)

//! Returns the smallest value
#define FPL_MIN(a, b) ((a) < (b)) ? (a) : (b)
//! Returns the biggest value
#define FPL_MAX(a, b) ((a) > (b)) ? (a) : (b)

//! Returns the number of bytes for the given kilobytes
#define FPL_KILOBYTES(value) (value * 1024)
//! Returns the number of bytes for the given megabytes
#define FPL_MEGABYTES(value) (FPL_KILOBYTES() * 1024)
//! Returns the number of bytes for the given gigabytes
#define FPL_GIGABYTES(value) (FPL_MEGABYTES() * 1024)
//! Returns the number of bytes for the given terabytes
#define FPL_TERABYTES(value) (FPL_GIGABYTES() * 1024)
//! Returns the number of bytes for the given petabytes
#define FPL_PETABYTES(value) (FPL_TERABYTES() * 1024)
//! Returns the number of bytes for the given exabytes
#define FPL_EXABYTES(value) (FPL_PETABYTES() * 1024)
//! Returns the number of bytes for the given exabytes
#define FPL_ZETTABYTES(value) (FPL_EXABYTES() * 1024)
//! Returns the number of bytes for the given exabytes
#define FPL_YOTTABYTES(value) (FPL_ZETTABYTES() * 1024)

//! Defines the operator overloads for a enum used as flags
#define FPL_ENUM_AS_FLAGS_OPERATORS_INTERNAL(type) \
		inline type operator | (type lhs, type rhs) { \
			return (type)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
		} \
		inline bool operator & (const type lhs, const type rhs) { \
			return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) == static_cast<uint32_t>(rhs); \
		} \
		inline type& operator |= (type &lhs, type rhs) { \
			lhs = (type)(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
			return lhs; \
		}

//
// Types
//
#include <stdint.h>

// ****************************************************************************
//
// API Declaration
//
// ****************************************************************************

//! Core namespace
namespace fpl {

	//! Atomic functions, like CompareAndExchange etc.
	namespace atomics {
		//! Insert a atomic read fence. (Complete current writes before future reads)
		fpl_api void AtomicReadFence();
		//! Insert a atomic write fence. (Complete current reads before future writes)
		fpl_api void AtomicWriteFence();
		//! Insert a atomic read/write fence. (Complete current reads and writes before future reads and writes)
		fpl_api void AtomicReadWriteFence();
		//! Replace a 32-bit unsigned integer with the given value atomically. Returns the target before the replacement.
		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value);
		//! Replace a 32-bit signed integer with the given value atomically. Returns the target before the replacement.
		fpl_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value);
		//! Replace a 64-bit unsigned integer with the given value atomically. Returns the target before the replacement.
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value);
		//! Replace a 64-bit signed integer with the given value atomically. Returns the target before the replacement.
		fpl_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value);
		//! Adds a 32-bit unsigned integer atomatically. Returns the value before the addition.
		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend);
		//! Adds a 32-bit signed integer atomatically. Returns the value before the addition.
		fpl_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend);
		//! Adds a 64-bit unsigned integer atomatically. Returns the value before the addition.
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend);
		//! Adds a 64-bit signed integer atomatically. Returns the value before the addition.
		fpl_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend);
		//! Compares a 32-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand);
		//! Compares a 32-bit signed integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t exchange, const int32_t comparand);
		//! Compares a 64-bit unsigned integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api uint64_t AtomicAndCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand);
		//! Compares a 64-bit signed integer with a comparand and exchange it when comparand and matches destination. Returns the dest before the exchange.
		fpl_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t exchange, const int64_t comparand);
	};

	//! Hardware functions, like GetProcessorCoreCount, GetProcessorName, etc.
	namespace hardware {
		//! Returns the total number of processor cores
		fpl_api uint32_t GetProcessorCoreCount();
		//! Returns the processor name/identifier. Result is written in the required destination buffer.
		fpl_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen);
	};

	//! Initialization flags (Window, Video, All, etc.)
	enum class InitFlags : uint32_t {
		//! No init flags
		None = 0,
		//! Create a single window
		Window = 1 << 0,
		//! Create a video context
		Video = 1 << 1,
		//! Default init flags for a window + video
		All = Window | Video,
	};
	//! Operator support for InitFlags
	FPL_ENUM_AS_FLAGS_OPERATORS_INTERNAL(InitFlags);

	//! Video driver type
	enum class VideoDriverType : uint32_t {
		//! No video driver
		None = 0,
		//! OpenGL
		OpenGL,
		//! Software
		Software,
	};

	//! Window settings (Size, Title etc.)
	struct WindowSettings {
		//! Window title
		char windowTitle[256];
		//! Window width in screen coordinates
		uint32_t windowWidth;
		//! Window height in screen coordinates
		uint32_t windowHeight;
		//! Fullscreen width in screen coordinates
		uint32_t fullscreenWidth;
		//! Fullscreen height in screen coordinates
		uint32_t fullscreenHeight;
		//! Is window resizable
		bool isResizable;
		//! Is window in fullscreen mode
		bool isFullscreen;

		WindowSettings() {
			windowTitle[0] = 0;
			windowWidth = 800;
			windowHeight = 600;
			fullscreenWidth = 0;
			fullscreenHeight = 0;
			isResizable = true;
			isFullscreen = false;
		}
	};

	//! Video compability profile
	enum class VideoCompabilityProfile {
		//! Use legacy context
		Legacy,
		//! Use core context with backwards compability
		Core,
		//! Use foward context without backwards compability
		Forward,
	};

	//! Video settings container (Compability Profile, Version, VSync, etc.)
	struct VideoSettings {
		//! Video driver type
		VideoDriverType driverType;
		//! Compability profile
		VideoCompabilityProfile profile;
		//! Desired major version
		uint32_t majorVersion;
		//! Desired minor version
		uint32_t minorVersion;
		//! Vertical syncronisation is wanted
		bool isVSync;
		//! Backbuffer size is automatically resized. Useable only for software rendering!
		bool isAutoSize;

		VideoSettings() {
			profile = VideoCompabilityProfile::Legacy;
			majorVersion = minorVersion = 0;
			isVSync = false;
			isAutoSize = true;
		#if defined(FPL_ENABLE_VIDEO_OPENGL)
			driverType = VideoDriverType::OpenGL;
		#elif defined(FPL_ENABLE_VIDEO_SOFTWARE)
			driverType = VideoDriverType::Software;
		#else
			driverType = VideoDriverType::None;
		#endif
		}
	};

	//! Settings container (Window, Video, etc)
	struct Settings {
		//! Window settings
		WindowSettings window;
		//! Video settings
		VideoSettings video;

		Settings() {
			window = WindowSettings();
			video = VideoSettings();
		}
	};

	//! Initialize the platform layer.
	fpl_api bool InitPlatform(const InitFlags initFlags, const Settings &initSettings = Settings());
	//! Releases the platform layer and resets all structures to zero.
	fpl_api void ReleasePlatform();
	//! Returns last error string
	fpl_api const char *GetPlatformLastError();
	//! Returns last error string from the given index
	fpl_api const char *GetPlatformLastError(const size_t index);
	//! Returns number of last errors
	fpl_api size_t GetPlatformLastErrorCount();
	//! Returns the current settings
	fpl_api const Settings &GetCurrentSettings();

	//! Dynamic library functions and types
	namespace library {
		//! Handle to a loaded dynamic library
		struct DynamicLibraryHandle {
			//! Internal library handle
			void *internalHandle;
			//! Library opened successfully
			bool isValid;
		};

		//! Loads a dynamic library and returns the loaded handle for it.
		fpl_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath);
		//! Returns the dynamic library procedure address for the given procedure name.
		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name);
		//! Releases the loaded library and resets the handle to zero.
		fpl_api void DynamicLibraryUnload(DynamicLibraryHandle &handle);
	};

	//! Console functions
	namespace console {
		//! Writes the given text to the default console output
		fpl_api void ConsoleOut(const char *text);
		//! Writes the given formatted text to the default console output 
		fpl_api void ConsoleFormatOut(const char *format, ...);
		//! Writes the given text to the console error output
		fpl_api void ConsoleError(const char *text);
		//! Writes the given formatted text to the console error output
		fpl_api void ConsoleFormatError(const char *format, ...);
	};

	//! Threading functions
	namespace threading {
		//! Thread state type
		enum class ThreadState {
			//! Thread is stopped
			Stopped,
			//! Thread is still running
			Running,
			// Thread is suspended
			Suspended,
			// Thread is in waiting state until it gets waked up
			Waiting,
		};

		struct ThreadContext;
		//! Run function type definition for CreateThread
		typedef void (run_thread_function)(const ThreadContext &context, void *data);

		//! Stores all required information for a thread, like its handle and its state, etc.
		struct ThreadContext {
			//! The identifier of the thread
			uint64_t id;
			//! The stored run function
			run_thread_function *runFunc;
			//! The user data passed to the run function
			void *data;
			//! The internal handle
			void *internalHandle;
			//! The current state
			ThreadState state;
		};

		//! Mutex context
		struct ThreadMutex {
			//! The internal handle
			void *internalHandle;
			//! Is it valid
			bool isValid;
		};

		//! Signal context
		struct ThreadSignal {
			//! The internal handle
			void *internalHandle;
			//! Is it valid
			bool isValid;
		};

		//! Create a thread and return the context for it. When @autoStart is set to true, it will start immediatly. 
		fpl_api ThreadContext &ThreadCreate(run_thread_function *runFunc, void *data, const bool autoStart = true);
		//! Let the current thread sleep for the number of given milliseconds
		fpl_api void ThreadSleep(const uint32_t milliseconds);
		//! Suspend the given thread, so any user code will get be freezed at the current point. Returns true when the thread was successfully suspended.
		fpl_api bool ThreadSuspend(ThreadContext &context);
		//! Resume a suspended thread, so any user code will continue to run. Returns true when the thread was successfully resumed.
		fpl_api bool ThreadResume(ThreadContext &context);
		//! Stop the given thread and release all underlying resources.
		fpl_api void ThreadStop(ThreadContext &context);
		//! Wait until the given thread is done running. When @maxMilliseconds is set to @UINT32_MAX it will wait infinitly.
		fpl_api void ThreadWaitForSingle(const ThreadContext &context, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Wait until all given threads are done running. When @maxMilliseconds is set to @UINT32_MAX it will wait infinitly.
		fpl_api void ThreadWaitForMultiple(const ThreadContext *contexts, const uint32_t count, const uint32_t maxMilliseconds = UINT32_MAX);

		//! Creates a mutex
		fpl_api ThreadMutex MutexCreate();
		//! Destroys the given mutex
		fpl_api void MutexDestroy(ThreadMutex &mutex);
		//! Locks the given mutex and waits until it gets unlocked. When @maxMilliseconds is set to @UINT32_MAX it will wait infinitly.
		fpl_api void MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Unlocks the given mutex
		fpl_api void MutexUnlock(ThreadMutex &mutex);

		//! Creates a signal
		fpl_api ThreadSignal SignalCreate();
		//! Destroys the given signal
		fpl_api void SignalDestroy(ThreadSignal &signal);
		//! Waits until the given signal are waked up. When @maxMilliseconds is set to @UINT32_MAX it will wait infinitly.
		fpl_api bool SignalWait(ThreadSignal &signal, const uint32_t maxMilliseconds = UINT32_MAX);
		//! Wakes up the given signal
		fpl_api bool SignalWakeUp(ThreadSignal &signal);
	};

	//! Memory allocation functions
	namespace memory {
		//! Clears the given memory by the given size to zero.
		fpl_api void MemoryClear(void *mem, const size_t size);
		//! Copies the given source memory with its length to the target memory
		fpl_api void MemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem);
		//! Allocates memory from the operating system by the given size. The memory is guaranteed to be initialized by zero.
		fpl_api void *MemoryAllocate(const size_t size);
		//! Releases the memory allocated from the operating system.
		fpl_api void MemoryFree(void *ptr);
		//! Allocates memory on the current stack. Use this very carefully, it will be released then the current scope goes out of scope!
		fpl_api void *MemoryStackAllocate(const size_t size);
		//! Allocates aligned memory from the operating system by the given alignment. The memory is guaranteed to be initialized by zero.
		fpl_api void *MemoryAlignedAllocate(const size_t size, const size_t alignment);
		//! Releases aligned memory from the operating system.
		fpl_api void MemoryAlignedFree(void *ptr);
	};

	//! Timing and measurement functions
	namespace timings {
		//! Returns the current system clock in seconds with the highest precision.
		fpl_api double GetHighResolutionTimeInSeconds();
	};

	//! String functions
	namespace strings {
		//! Returns the number of characters of the given 8-bit Ansi string. Null terminator is not included.
		fpl_api uint32_t GetAnsiStringLength(const char *str);
		//! Returns the number of characters of the given 16-bit Wide string. Null terminator is not included.
		fpl_api uint32_t GetWideStringLength(const wchar_t *str);
		//! Copies the given 8-bit source Ansi string with length into a destination Ansi string. Does not allocate any memory.
		fpl_api char *CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen);
		//! Copies the given 8-bit source Ansi string into a destination Ansi string. Does not allocate any memory.
		fpl_api char *CopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen);
		//! Copies the given 16-bit source Wide string with length into a destination Wide string. Does not allocate any memory.
		fpl_api wchar_t *CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen);
		//! Copies the given 16-bit source Wide string into a destination Wide string. Does not allocate any memory.
		fpl_api wchar_t *CopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen);
		//! Converts the given 16-bit source Wide string with length in a 8-bit Ansi string. Does not allocate any memory.
		fpl_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen);
		//! Converts the given 16-bit Wide string in a 8-bit UTF-8 string. Does not allocate any memory.
		fpl_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen);
		//! Converts the given 8-bit Ansi string in a 16-bit Wide string. Does not allocate any memory.
		fpl_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
		//! Converts the given 8-bit UTF-8 string in a 16-bit Wide string. Does not allocate any memory.
		fpl_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen);
	};

	//! Files & directory functions and types
	namespace files {
		//! Handle to a loaded/created file
		struct FileHandle {
			//! Internal file handle
			void *internalHandle;
			//! File opened successfully
			bool isValid;
		};

		//! File position mode (Beginning, Current, End)
		enum class FilePositionMode {
			//! Starts from the beginning
			Beginning = 0,
			//! Starts from the current position
			Current = 1,
			//! Starts from the end
			End = 2,
		};

		//! File entry type (File, Directory, etc.)
		enum class FileEntryType {
			//! Unknown entry type
			Unknown = 0,
			//! Entry is a file
			File = 1,
			//! Entry is a directory
			Directory = 2,
		};

		//! File attribute flags (Normal, Readonly, Hidden, etc.)
		enum class FileAttributeFlags : uint32_t {
			//! No attributes
			None = 0,
			//! Normal
			Normal = 1 << 0,
			//! Readonly
			ReadOnly = 1 << 1,
			//! Hidden
			Hidden = 1 << 2,
			//! Archive
			Archive = 1 << 3,
			//! System
			System = 1 << 4,
		};
		//! Operator support for FileAttributeFlags
		FPL_ENUM_AS_FLAGS_OPERATORS_INTERNAL(FileAttributeFlags);

		//! Maximum length of a file entry path
		constexpr uint32_t MAX_FILEENTRY_PATH_LENGTH = 1024;

		//! Entry for storing current file informations (path, type, attributes, etc.)
		struct FileEntry {
			//! Entry type
			FileEntryType type;
			//! File attributes
			FileAttributeFlags attributes;
			//! File path
			char path[MAX_FILEENTRY_PATH_LENGTH];
			//! Internal file handle
			void *internalHandle;
		};

		//! Opens a binary file from a ansi string path for reading and returns the handle of it.
		fpl_api FileHandle OpenBinaryFile(const char *filePath);
		//! Opens a binary file from a wide string path for reading and returns the handle of it.
		fpl_api FileHandle OpenBinaryFile(const wchar_t *filePath);
		//! Creates a binary file to a ansi string path and returns the handle of it.
		fpl_api FileHandle CreateBinaryFile(const char *filePath);
		//! Creates a binary file to a wide string path and returns the handle of it.
		fpl_api FileHandle CreateBinaryFile(const wchar_t *filePath);
		//! Reads a block from the given file handle and returns the number of bytes readed. Operation is limited to a 2 GB byte boundary.
		fpl_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize);
		//! Writes a block to the given file handle and returns the number of bytes written. Operation is limited to a 2 GB byte boundary.
		fpl_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize);
		//! Sets the current file position by the given position, depending on the mode its absolute or relative. Position is limited to a 2 GB byte boundary.
		fpl_api void SetFilePosition32(const FileHandle &fileHandle, const uint32_t position, const FilePositionMode mode);
		//! Returns the current file position. Position is limited to a 2 GB byte boundary.
		fpl_api uint32_t GetFilePosition32(const FileHandle &fileHandle);
		//! Closes the given file handle and resets the handle to zero.
		fpl_api void CloseFile(FileHandle &fileHandle);

		// @TODO(final): Add 64-bit file operations as well!

		//! Returns the 32-bit file size in bytes for the given file. Its limited to files < 2 GB.
		fpl_api uint32_t GetFileSize32(const char *filePath);
		//! Returns the 32-bit file size in bytes for a opened file. Its limited to files < 2 GB.
		fpl_api uint32_t GetFileSize32(const FileHandle &fileHandle);
		//! Returns true when the given file path physically exists.
		fpl_api bool FileExists(const char *filePath);
		//! Copies the given source file to the target path and returns truwe when copy was successful. Target path must include the full path to the file. When overwrite is set, the target file path will always be overwritten.
		fpl_api bool CopyAFile(const char *sourceFilePath, const char *targetFilePath, const bool overwrite);
		//! Deletes the given file without confirmation and returns true when the deletion was successful.
		fpl_api bool DeleteAFile(const char *filePath);

		//! Creates all the directories in the path when not exists.
		fpl_api bool CreateDirectories(const char *path);
		//! Returns true when the given directory path physically exists.
		fpl_api bool DirectoryExists(const char *path);
		//! Deletes the given directory without confirmation and returns true when the deletion was successful. When recursive is set, all files and folders in sub-directories will be deleted as well.
		fpl_api bool RemoveEmptyDirectory(const char *path);
		//! Iterates through files / directories in the given directory. The path must contain the filter as well. Returns true when there was a first entry found.
		fpl_api bool ListFilesBegin(const char *pathAndFilter, FileEntry *firstEntry);
		//! Get next file entry from iterating through files / directories. Returns false when there is no entry found.
		fpl_api bool ListFilesNext(FileEntry *nextEntry);
		//! Releases opened resources from iterating through files / directories.
		fpl_api void ListFilesEnd(FileEntry *lastEntry);
	};

	//! Directory and paths functions
	namespace paths {
		//! Returns the full path to this executable, including the executable file name. Result is written in the required destination buffer.
		fpl_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen);
		//! Returns the full path to your home directory. Result is written in the required destination buffer.
		fpl_api char *GetHomePath(char *destPath, const uint32_t maxDestLen);
		//! Returns the path from the given source path. Result is written in the required destination buffer.
		fpl_api char *ExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen);
		//! Returns the file extension from the given source path.
		fpl_api char *ExtractFileExtension(const char *sourcePath);
		//! Returns the file name including the file extension from the given source path.
		fpl_api char *ExtractFileName(const char *sourcePath);
		//! Changes the file extension on the given source path and writes the result into the destination path. Returns the pointer of the destination path. Result is written in the required destination buffer.
		fpl_api char *ChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen);
		//! Combines all included path by the systems path separator. Returns the pointer of the destination path. Result is written in the required destination buffer.
		fpl_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...);
	};

#if defined(FPL_ENABLE_WINDOW)
	//! Window based functions and types
	namespace window {
		//! Mapped key (Based on MS Virtual-Key-Codes, mostly directly mapped from ASCII)
		enum class Key {
			Key_None = 0,

			// 0x07: Undefined

			Key_Backspace = 0x08,
			Key_Tab = 0x09,

			// 0x0A-0x0B: Reserved

			Key_Clear = 0x0C,
			Key_Enter = 0x0D,

			// 0x0E-0x0F: Undefined

			Key_Shift = 0x10,
			Key_Control = 0x11,
			Key_Alt = 0x12,
			Key_Pause = 0x13,
			Key_CapsLock = 0x14,

			// 0x15: IME-Keys
			// 0x16: Undefined
			// 0x17-0x19 IME-Keys
			// 0x1A: Undefined

			Key_Escape = 0x1B,

			// 0x1C - 0x1F: IME-Keys

			Key_Space = 0x20,
			Key_PageUp = 0x21,
			Key_PageDown = 0x22,
			Key_End = 0x23,
			Key_Home = 0x24,
			Key_Left = 0x25,
			Key_Up = 0x26,
			Key_Right = 0x27,
			Key_Down = 0x28,
			Key_Select = 0x29,
			Key_Print = 0x2A,
			Key_Execute = 0x2B,
			Key_Snapshot = 0x2C,
			Key_Insert = 0x2D,
			Key_Delete = 0x2E,
			Key_Help = 0x2F,

			Key_0 = 0x30,
			Key_1 = 0x31,
			Key_2 = 0x32,
			Key_3 = 0x33,
			Key_4 = 0x34,
			Key_5 = 0x35,
			Key_6 = 0x36,
			Key_7 = 0x37,
			Key_8 = 0x38,
			Key_9 = 0x39,

			// 0x3A-0x40: Undefined

			Key_A = 0x41,
			Key_B = 0x42,
			Key_C = 0x43,
			Key_D = 0x44,
			Key_E = 0x45,
			Key_F = 0x46,
			Key_G = 0x47,
			Key_H = 0x48,
			Key_I = 0x49,
			Key_J = 0x4A,
			Key_K = 0x4B,
			Key_L = 0x4C,
			Key_M = 0x4D,
			Key_N = 0x4E,
			Key_O = 0x4F,
			Key_P = 0x50,
			Key_Q = 0x51,
			Key_R = 0x52,
			Key_S = 0x53,
			Key_T = 0x54,
			Key_U = 0x55,
			Key_V = 0x56,
			Key_W = 0x57,
			Key_X = 0x58,
			Key_Y = 0x59,
			Key_Z = 0x5A,

			Key_LeftWin = 0x5B,
			Key_RightWin = 0x5C,
			Key_Apps = 0x5D,

			// 0x5E: Reserved

			Key_Sleep = 0x5F,
			Key_NumPad0 = 0x60,
			Key_NumPad1 = 0x61,
			Key_NumPad2 = 0x62,
			Key_NumPad3 = 0x63,
			Key_NumPad4 = 0x64,
			Key_NumPad5 = 0x65,
			Key_NumPad6 = 0x66,
			Key_NumPad7 = 0x67,
			Key_NumPad8 = 0x68,
			Key_NumPad9 = 0x69,
			Key_Multiply = 0x6A,
			Key_Add = 0x6B,
			Key_Separator = 0x6C,
			Key_Substract = 0x6D,
			Key_Decimal = 0x6E,
			Key_Divide = 0x6F,
			Key_F1 = 0x70,
			Key_F2 = 0x71,
			Key_F3 = 0x72,
			Key_F4 = 0x73,
			Key_F5 = 0x74,
			Key_F6 = 0x75,
			Key_F7 = 0x76,
			Key_F8 = 0x77,
			Key_F9 = 0x78,
			Key_F10 = 0x79,
			Key_F11 = 0x7A,
			Key_F12 = 0x7B,
			Key_F13 = 0x7C,
			Key_F14 = 0x7D,
			Key_F15 = 0x7E,
			Key_F16 = 0x7F,
			Key_F17 = 0x80,
			Key_F18 = 0x81,
			Key_F19 = 0x82,
			Key_F20 = 0x83,
			Key_F21 = 0x84,
			Key_F22 = 0x85,
			Key_F23 = 0x86,
			Key_F24 = 0x87,

			// 0x88-8F: Unassigned

			Key_NumLock = 0x90,
			Key_Scroll = 0x91,

			// 0x92-9x96: OEM specific
			// 0x97-0x9F: Unassigned

			Key_LeftShift = 0xA0,
			Key_RightShift = 0xA1,
			Key_LeftControl = 0xA2,
			Key_RightControl = 0xA3,
			Key_LeftAlt = 0xA4,
			Key_RightAlt = 0xA5,

			// 0xA6-0xFE: Dont care
		};

		//! Window event type (Resized, PositionChanged, etc.)
		enum class WindowEventType {
			//! Window has been resized
			Resized = 1,
			//! Window got focus
			GotFocus = 2,
			//! Window lost focus
			LostFocus = 3,
		};

		//! Window event data (Size, Position, etc.)
		struct WindowEvent {
			//! Window event type
			WindowEventType type;
			//! Window width in screen coordinates
			uint32_t width;
			//! Window height in screen coordinates
			uint32_t height;
		};

		//! Keyboard event type (KeyDown, KeyUp, Char, ...)
		enum class KeyboardEventType {
			//! Key is down
			KeyDown = 1,
			//! Key was released
			KeyUp = 2,
			//! Character was entered
			Char = 3,
		};

		//! Keyboard modifier flags (Alt, Ctrl, ...)
		enum class KeyboardModifierFlags : uint32_t {
			//! No modifiers
			None = 0,
			//! Alt key is down
			Alt = 1 << 0,
			//! Ctrl key is down
			Ctrl = 1 << 1,
			//! Shift key is down
			Shift = 1 << 2,
			//! Super key is down
			Super = 1 << 3,
		};
		//! Operator support for KeyboardModifierFlags
		FPL_ENUM_AS_FLAGS_OPERATORS_INTERNAL(KeyboardModifierFlags);

		//! Keyboard event data (Type, Keycode, Mapped key, etc.)
		struct KeyboardEvent {
			//! Keyboard event type
			KeyboardEventType type;
			//! Raw key code
			uint64_t keyCode;
			//! Mapped key
			Key mappedKey;
			//! Keyboard modifiers
			KeyboardModifierFlags modifiers;
		};

		//! Mouse event type (Move, ButtonDown, ...)
		enum class MouseEventType {
			//! Mouse position has been changed
			Move = 1,
			//! Mouse button is down
			ButtonDown = 2,
			//! Mouse button was released
			ButtonUp = 3,
			//! Mouse wheel up/down
			Wheel = 4,
		};

		//! Mouse button type (Left, Right, ...)
		enum class MouseButtonType : int32_t {
			//! No mouse button
			None = -1,
			//! Left mouse button
			Left = 0,
			//! Right mouse button
			Right = 1,
			//! Middle mouse button
			Middle = 2,
		};

		//! Mouse event data (Type, Button, Position, etc.)
		struct MouseEvent {
			//! Mouse event type
			MouseEventType type;
			//! Mouse button
			MouseButtonType mouseButton;
			//! Mouse X-Position
			int32_t mouseX;
			//! Mouse Y-Position
			int32_t mouseY;
			//! Mouse wheel delta
			float wheelDelta;
		};

		//! Gamepad event type (Connected, Disconnected, StateChanged, etc.)
		enum class GamepadEventType {
			//! No gamepad event
			None = 0,
			//! Gamepad connected
			Connected = 1,
			//! Gamepad disconnected
			Disconnected = 2,
			//! Gamepad state updated
			StateChanged = 3,
		};

		//! Gamepad button (IsDown, etc.)
		struct GamepadButton {
			//! Is button down
			bool isDown;
		};

		//! Gamepad state data
		struct GamepadState {
			union {
				struct {
					//! Digital button up
					GamepadButton dpadUp;
					//! Digital button right
					GamepadButton dpadRight;
					//! Digital button down
					GamepadButton dpadDown;
					//! Digital button left
					GamepadButton dpadLeft;

					//! Action button A
					GamepadButton actionA;
					//! Action button B
					GamepadButton actionB;
					//! Action button X
					GamepadButton actionX;
					//! Action button Y
					GamepadButton actionY;

					//! Start button
					GamepadButton start;
					//! Back button
					GamepadButton back;

					//! Analog left thumb button
					GamepadButton leftThumb;
					//! Analog right thumb button
					GamepadButton rightThumb;

					//! Left shoulder button
					GamepadButton leftShoulder;
					//! Right shoulder button
					GamepadButton rightShoulder;
				};

				//! All gamepad buttons
				GamepadButton buttons[14];
			};

			//! Analog left thumb X in range (-1.0 to 1.0f)
			float leftStickX;
			//! Analog left thumb Y in range (-1.0 to 1.0f)
			float leftStickY;
			//! Analog right thumb X in range (-1.0 to 1.0f)
			float rightStickX;
			//! Analog right thumb Y in range (-1.0 to 1.0f)
			float rightStickY;

			//! Analog left trigger in range (-1.0 to 1.0f)
			float leftTrigger;
			//! Analog right trigger in range (-1.0 to 1.0f)
			float rightTrigger;
		};

		//! Gamepad event data (Type, Device, State, etc.)
		struct GamepadEvent {
			//! Gamepad event type
			GamepadEventType type;
			//! Gamepad device index
			uint32_t deviceIndex;
			//! Full gamepad state
			GamepadState state;
		};

		//! Event type (Window, Keyboard, Mouse, ...)
		enum class EventType {
			//! Window event
			Window = 1,
			//! Keyboard event
			Keyboard = 2,
			//! Mouse event
			Mouse = 3,
			//! Gamepad event
			Gamepad = 4,
		};

		//! Event data (Type, Window, Keyboard, Mouse, etc.)
		struct Event {
			//! Event type
			EventType type;
			union {
				//! Window event data
				WindowEvent window;
				//! Keyboard event data
				KeyboardEvent keyboard;
				//! Mouse event data
				MouseEvent mouse;
				//! Gamepad event data
				GamepadEvent gamepad;
			};
		};

		//! Window size in screen coordinates
		struct WindowSize {
			//! Width in screen coordinates
			uint32_t width;
			//! Height in screen coordinates
			uint32_t height;
		};

		//! Window position in screen coordinates
		struct WindowPosition {
			//! Left position in screen coordinates
			int32_t left;
			//! Top position in screen coordinates
			int32_t top;
		};

		//
		// Window
		//

		//! Returns true when the window is active.
		fpl_api bool IsWindowRunning();
		//! Processes the message queue of the window.
		fpl_api bool WindowUpdate();
		//! Forces the window to redraw or swap the back/front buffer.
		fpl_api void WindowFlip();
		//! Enables or disables the window cursor.
		fpl_api void SetWindowCursorEnabled(const bool value);
		//! Returns the inner window area.
		fpl_api WindowSize GetWindowArea();
		//! Resizes the window to fit the inner area to the given size.
		fpl_api void SetWindowArea(const uint32_t width, const uint32_t height);
		//! Returns true when the window is resizable.
		fpl_api bool IsWindowResizable();
		//! Enables or disables the ability to resize the window.
		fpl_api void SetWindowResizeable(const bool value);
		//! Enables or disables fullscreen mode
		fpl_api void SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth = 0, const uint32_t fullscreenHeight = 0, const uint32_t refreshRate = 0);
		//! Returns true when the window is in fullscreen mode
		fpl_api bool IsWindowFullscreen();
		//! Returns the absolute window position.
		fpl_api WindowPosition GetWindowPosition();
		//! Sets the window absolut position to the given coordinates.
		fpl_api void SetWindowPosition(const int32_t left, const int32_t top);
		//! Sets the window title
		fpl_api void SetWindowTitle(const char *title);

		//
		// Events
		//

		//! Gets and removes the top event from the internal queue and fills out the "event" argument. Returns false when there are no events left, otherwise true.
		fpl_api bool PollWindowEvent(Event &ev);

		//
		// Clipboard
		//

		//! Returns the current clipboard ansi text
		fpl_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen);
		//! Returns the current clipboard wide text
		fpl_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen);
		//! Overwrites the current clipboard ansi text with the given one.
		fpl_api bool SetClipboardText(const char *ansiSource);
		//! Overwrites the current clipboard wide text with the given one.
		fpl_api bool SetClipboardText(const wchar_t *wideSource);
	};

	//! Video context access
	namespace video {
		//! Video backbuffer container. Use this for accessing the pixels directly. Use with care!
		struct VideoBackBuffer {
			//! The 32-bit pixel top-down array, format: 0xAABBGGRR. Do not modify before @WindowUpdate
			uint32_t *pixels;
			//! The width of the backbuffer in pixels. Do not modify, it will be set automatically.
			uint32_t width;
			//! The height of the backbuffer in pixels. Do not modify, it will be set automatically.
			uint32_t height;
			//! The size of one scanline. Do not modify, it will be set automatically.
			size_t stride;
		};

		//! Returns the pointer to the video software context.
		fpl_api VideoBackBuffer *GetVideoBackBuffer();
		//! Resizes the current video backbuffer
		fpl_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height);
	};
#endif // defined(FPL_ENABLE_WINDOW)
};

//
// Platform specific defines/includes and forward declarations
//
#if defined(FPL_PLATFORM_WIN32)
// @NOTE(final): Unfortunatly we have to include windows header here, because we want the user have access to the opengl header.
// @NOTE(final): windef.h defines min/max macros defined in lowerspace, this will break for example std::min/max so we have to tell the header we dont want this!
#	if !defined(NOMINMAX)
#		define NOMINMAX
#	endif
// @NOTE(final): For now we dont want any network, com or gdi stuff at all, maybe later how knows.
#	if !defined(WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN 1
#	endif
#	include <Windows.h> // Win32 api
#	if defined(FPL_ENABLE_VIDEO_OPENGL)
#		if !defined(GL_VERSION_1_1)
#			include <gl\gl.h> // wglMakeCurrent, wglCreateContext, etc.
#		endif
#	endif // FPL_ENABLE_VIDEO_OPENGL

// @NOTE(final): Required for access "main" from the actual win32 entry point
fpl_api int main(int argc, char **args);
#endif

// Expand all namespaces if the callers wants this
#if defined(FPL_ENABLE_AUTO_NAMESPACE)
using namespace fpl;
#	if defined(FPL_ENABLE_WINDOW)
using namespace fpl::window;
using namespace fpl::video;
#	endif
using namespace fpl::atomics;
using namespace fpl::hardware;
using namespace fpl::memory;
using namespace fpl::timings;
using namespace fpl::paths;
using namespace fpl::files;
using namespace fpl::library;
using namespace fpl::strings;
using namespace fpl::console;
using namespace fpl::threading;
#endif

#endif // FPL_INCLUDE_HPP

// ****************************************************************************
//
// Implementation
//
// ****************************************************************************
#if defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)
#	define FPL_IMPLEMENTED

// 
// Non-Platform specific includes
//
#include <stdarg.h>  // va_start, va_end, va_list, va_arg
#include <stdio.h> // fprintf, vfprintf, vsprintf
#include <new> // placement new

//
// Compiler specific includes
//
#if defined(FPL_COMPILER_MSVC)
#	include <intrin.h>
#endif 

//
// Internal macros
//
#define FPL_CLEARMEM_INTERNAL(T, memory, size, shift, mask) \
	do { \
		size_t clearedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *dataBlock = static_cast<T *>(memory); \
			T *dataBlockEnd = static_cast<T *>(memory) + (size >> shift); \
			while (dataBlock != dataBlockEnd) { \
				*dataBlock++ = 0; \
				clearedBytes += sizeof(T); \
			} \
		} \
		uint8_t *data8 = (uint8_t *)memory + clearedBytes; \
		uint8_t *data8End = (uint8_t *)memory + size; \
		while (data8 != data8End) { \
			*data8++ = 0; \
		} \
	} while (0);

#define FPL_COPYMEM_INTERNAL(T, source, sourceSize, dest, shift, mask) \
	do { \
		size_t copiedBytes = 0; \
		if (sizeof(T) > sizeof(uint8_t)) { \
			T *sourceDataBlock = static_cast<T *>(source); \
			T *sourceDataBlockEnd = static_cast<T *>(source) + (sourceSize >> shift); \
			T *destDataBlock = static_cast<T *>(dest); \
			while (sourceDataBlock != sourceDataBlockEnd) { \
				*destDataBlock++ = *sourceDataBlock++; \
				copiedBytes += sizeof(T); \
			} \
		} \
		uint8_t *sourceData8 = (uint8_t *)source + copiedBytes; \
		uint8_t *sourceData8End = (uint8_t *)source + sourceSize; \
		uint8_t *destData8 = (uint8_t *)dest + copiedBytes; \
		while (sourceData8 != sourceData8End) { \
			*destData8++ = *sourceData8++; \
		} \
	} while (0);

namespace fpl {
	//
	// Platform constants
	//
#	if defined(FPL_PLATFORM_WIN32)
	constexpr char PATH_SEPARATOR = '\\';
	constexpr char FILE_EXT_SEPARATOR = '.';
#	else
	constexpr char PATH_SEPARATOR = '/';
	constexpr char FILE_EXT_SEPARATOR = '.';
#	endif

	//
	// Internal types and functions
	//
	constexpr uint32_t MAX_LAST_ERROR_STRING_LENGTH_INTERNAL = 1024;

#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
	// @NOTE(final): 1024 errors are more than enough
	constexpr size_t MAX_ERRORSTATE_COUNT_INTERNAL = 1024;
	struct ErrorState_Internal {
		char errors[MAX_ERRORSTATE_COUNT_INTERNAL][MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
		size_t count;
	};

	fpl_globalvar ErrorState_Internal *globalLastError_Internal = nullptr;

	fpl_internal void PushError_Internal_Formatted(const char *format, va_list &argList) {
		ErrorState_Internal *state = globalLastError_Internal;
		if (state != nullptr) {
			FPL_ASSERT(format != nullptr);
			char buffer[MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
			vsprintf_s(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
			uint32_t messageLen = strings::GetAnsiStringLength(buffer);
			FPL_ASSERT(state->count < MAX_ERRORSTATE_COUNT_INTERNAL);
			size_t errorIndex = state->count++;
			strings::CopyAnsiString(buffer, messageLen, state->errors[errorIndex], MAX_LAST_ERROR_STRING_LENGTH_INTERNAL);
		#if defined(FPL_ENABLE_ERROR_IN_CONSOLE)
			console::ConsoleError(buffer);
		#endif
		}
	}
#else
	struct ErrorState_Internal {
		char error[MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
	};

	fpl_globalvar ErrorState_Internal *globalLastError_Internal = nullptr;

	fpl_internal void PushError_Internal_Formatted(const char *format, va_list &argList) {
		ErrorState_Internal *state = globalLastError_Internal;
		if (state != nullptr) {
			FPL_ASSERT(format != nullptr);
			char buffer[MAX_LAST_ERROR_STRING_LENGTH_INTERNAL];
			vsprintf_s(buffer, FPL_ARRAYCOUNT(buffer), format, argList);
			uint32_t messageLen = strings::GetAnsiStringLength(buffer);
			strings::CopyAnsiString(buffer, messageLen, state->error, MAX_LAST_ERROR_STRING_LENGTH_INTERNAL);
		#if FPL_ENABLE_ERROR_IN_CONSOLE
			console::ConsoleError(buffer);
		#endif
		}
	}
#endif

	fpl_internal void PushError_Internal(const char *format, ...) {
		va_list valist;
		va_start(valist, format);
		PushError_Internal_Formatted(format, valist);
		va_end(valist);
	}

	namespace threading {
		constexpr uint32_t MAX_THREAD_COUNT = 64;
		struct ThreadState_Internal {
			ThreadContext mainThread;
			ThreadContext threads[MAX_THREAD_COUNT];
			uint32_t threadCount;
		};

		static ThreadState_Internal globalThreadState_Internal = {};

		static ThreadContext *GetThreadContext_Internal(const uint64_t id) {
			// @NOTE: We have just a few handful of max threads, so looping over all it not that slow.
			ThreadContext *result = nullptr;
			for (uint32_t index = 0; index < globalThreadState_Internal.threadCount; ++index) {
				ThreadContext *thread = globalThreadState_Internal.threads + index;
				if (thread->id == id) {
					result = thread;
					break;
				}
			}
			return(result);
		}
	}

	//
	// All Public String
	//
	namespace strings {
		fpl_api uint32_t GetAnsiStringLength(const char *str) {
			uint32_t result = 0;
			if (str != nullptr) {
				while (*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_api uint32_t GetWideStringLength(const wchar_t *str) {
			uint32_t result = 0;
			if (str != nullptr) {
				while (*str++) {
					result++;
				}
			}
			return(result);
		}

		fpl_api char *CopyAnsiString(const char *source, const uint32_t sourceLen, char *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(source && dest);
			FPL_ASSERT((sourceLen + 1) <= maxDestLen);
			char *result = dest;
			uint32_t index = 0;
			while (index++ < sourceLen) {
				*dest++ = *source++;
			}
			*dest = 0;
			return(result);
		}

		fpl_api char *CopyAnsiString(const char *source, char *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(source);
			uint32_t sourceLen = GetAnsiStringLength(source);
			char *result = CopyAnsiString(source, sourceLen, dest, maxDestLen);
			return(result);
		}

		fpl_api wchar_t *CopyWideString(const wchar_t *source, const uint32_t sourceLen, wchar_t *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(source && dest);
			FPL_ASSERT((sourceLen + 1) <= maxDestLen);
			wchar_t *result = dest;
			uint32_t index = 0;
			while (index++ < sourceLen) {
				*dest++ = *source++;
			}
			*dest = 0;
			return(result);
		}

		fpl_api wchar_t *CopyWideString(const wchar_t *source, wchar_t *dest, const uint32_t maxDestLen) {
			FPL_ASSERT(source);
			uint32_t sourceLen = GetWideStringLength(source);
			wchar_t *result = CopyWideString(source, sourceLen, dest, maxDestLen);
			return(result);
		}
	}

	//
	// All Public Memory
	//
	namespace memory {
		fpl_api void *MemoryAlignedAllocate(const size_t size, const size_t alignment) {
			FPL_ASSERT(size > 0);
			FPL_ASSERT((alignment > 0) && !(alignment & (alignment - 1)));

			// Allocate empty memory to hold a size of a pointer + the actual size + alignment padding 
			size_t newSize = sizeof(void *) + size + (alignment << 1);
			void *basePtr = MemoryAllocate(newSize);

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

		fpl_api void MemoryAlignedFree(void *ptr) {
			FPL_ASSERT(ptr != nullptr);

			// Free the base pointer which is stored to the left from the given pointer
			void *basePtr = (void *)((void **)((uintptr_t)ptr - sizeof(void *)));
			MemoryFree(basePtr);
		}

		constexpr size_t MemShift64 = 3;
		constexpr size_t MemMask64 = 0x00000007;
		constexpr size_t MemShift32 = 2;
		constexpr size_t MemMask32 = 0x00000003;
		constexpr size_t MemShift16 = 1;
		constexpr size_t MemMask16 = 0x00000001;

		fpl_api void MemoryClear(void *mem, const size_t size) {
			if (size % 8 == 0) {
				FPL_CLEARMEM_INTERNAL(uint64_t, mem, size, MemShift64, MemMask64);
			} else if (size % 4 == 0) {
				FPL_CLEARMEM_INTERNAL(uint32_t, mem, size, MemShift32, MemMask32);
			} else if (size % 2 == 0) {
				FPL_CLEARMEM_INTERNAL(uint16_t, mem, size, MemShift16, MemMask16);
			} else {
				FPL_CLEARMEM_INTERNAL(uint8_t, mem, size, 0, 0);
			}
		}

		fpl_api void MemoryCopy(void *sourceMem, const size_t sourceSize, void *targetMem) {
			if (sourceSize % 8 == 0) {
				FPL_COPYMEM_INTERNAL(uint64_t, sourceMem, sourceSize, targetMem, MemShift64, MemMask64);
			} else if (sourceSize % 4 == 0) {
				FPL_COPYMEM_INTERNAL(uint32_t, sourceMem, sourceSize, targetMem, MemShift32, MemMask32);
			} else if (sourceSize % 2 == 0) {
				FPL_COPYMEM_INTERNAL(uint16_t, sourceMem, sourceSize, targetMem, MemShift32, MemMask32);
			} else {
				FPL_COPYMEM_INTERNAL(uint8_t, sourceMem, sourceSize, targetMem, 0, 0);
			}
		}
	}

	//
	// All Public Path, Directories
	//
	namespace paths {
		fpl_api char *ExtractFilePath(const char *sourcePath, char *destPath, const uint32_t maxDestLen) {
			uint32_t sourceLen = strings::GetAnsiStringLength(sourcePath);
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (sourceLen + 1));

			char *result = nullptr;
			if (sourcePath) {
				int copyLen = 0;
				char *chPtr = (char *)sourcePath;
				while (*chPtr) {
					if (*chPtr == PATH_SEPARATOR) {
						copyLen = (int)(chPtr - sourcePath);
					}
					++chPtr;
				}
				if (copyLen) {
					result = strings::CopyAnsiString(sourcePath, copyLen, destPath, maxDestLen);
				}
			}
			return(result);
		}

		fpl_api char *ExtractFileExtension(const char *sourcePath) {
			char *result = (char *)nullptr;
			if (sourcePath != nullptr) {
				char *filename = ExtractFileName(sourcePath);
				if (filename) {
					char *chPtr = filename;
					char *firstSeparatorPtr = (char *)nullptr;
					while (*chPtr) {
						if (*chPtr == FILE_EXT_SEPARATOR) {
							firstSeparatorPtr = chPtr;
							break;
						}
						++chPtr;
					}
					if (firstSeparatorPtr != nullptr) {
						result = firstSeparatorPtr;
					}
				}
			}
			return(result);
		}

		fpl_api char *ExtractFileName(const char *sourcePath) {
			char *result = (char *)nullptr;
			if (sourcePath) {
				result = (char *)sourcePath;
				char *chPtr = (char *)sourcePath;
				char *lastPtr = (char *)nullptr;
				while (*chPtr) {
					if (*chPtr == PATH_SEPARATOR) {
						lastPtr = chPtr;
					}
					++chPtr;
				}
				if (lastPtr != nullptr) {
					result = lastPtr + 1;
				}
			}
			return(result);
		}

		fpl_api char *ChangeFileExtension(const char *filePath, const char *newFileExtension, char *destPath, const uint32_t maxDestLen) {
			uint32_t pathLen = strings::GetAnsiStringLength(filePath);
			uint32_t extLen = strings::GetAnsiStringLength(newFileExtension);

			// @NOTE: Put one additional character for the extension which may not contain a dot
			uint32_t sourceLen = pathLen + (extLen + 1);
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (sourceLen + 1));

			char *result = (char *)nullptr;
			if (filePath != nullptr) {
				// Find last path
				char *chPtr = (char *)filePath;
				char *lastPathSeparatorPtr = (char *)nullptr;
				while (*chPtr) {
					if (*chPtr == PATH_SEPARATOR) {
						lastPathSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				// Find last ext separator
				if (lastPathSeparatorPtr != nullptr) {
					chPtr = lastPathSeparatorPtr + 1;
				} else {
					chPtr = (char *)filePath;
				}
				char *lastExtSeparatorPtr = (char *)nullptr;
				while (*chPtr) {
					if (*chPtr == FILE_EXT_SEPARATOR) {
						lastExtSeparatorPtr = chPtr;
					}
					++chPtr;
				}

				uint32_t copyLen;
				if (lastExtSeparatorPtr != nullptr) {
					copyLen = (uint32_t)((uintptr_t)lastExtSeparatorPtr - (uintptr_t)filePath);
				} else {
					copyLen = pathLen;
				}

				// Copy parts
				strings::CopyAnsiString(filePath, copyLen, destPath, maxDestLen);
				char *destExtPtr = destPath + copyLen;
				strings::CopyAnsiString(newFileExtension, extLen, destExtPtr, maxDestLen - copyLen);
				result = destPath;
			}
			return(result);
		}

		fpl_api char *CombinePath(char *destPath, const uint32_t maxDestPathLen, const uint32_t pathCount, ...) {
			uint32_t curDestPosition = 0;
			char *currentDestPtr = destPath;
			va_list vargs;
			va_start(vargs, pathCount);
			for (uint32_t pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
				char *path = va_arg(vargs, char *);
				uint32_t pathLen = strings::GetAnsiStringLength(path);
				bool requireSeparator = pathIndex < (pathCount - 1);
				uint32_t requiredPathLen = requireSeparator ? pathLen + 1 : pathLen;
				FPL_ASSERT(curDestPosition + requiredPathLen <= maxDestPathLen);
				strings::CopyAnsiString(path, pathLen, currentDestPtr, maxDestPathLen - curDestPosition);
				currentDestPtr += pathLen;
				if (requireSeparator) {
					*currentDestPtr++ = PATH_SEPARATOR;
				}
				curDestPosition += requiredPathLen;
			}
			*currentDestPtr = 0;
			va_end(vargs);
			return destPath;
		}

	}

	//
	// Compiler Intrinsics (These are platform/compiler dependent)
	//
#if defined(FPL_COMPILER_MSVC)
	namespace atomics {
		fpl_api void AtomicReadFence() {
			_ReadBarrier();
		}
		fpl_api void AtomicWriteFence() {
			_WriteBarrier();
		}
		fpl_api void AtomicReadWriteFence() {
			_ReadWriteBarrier();
		}
		fpl_api uint32_t AtomicExchangeU32(volatile uint32_t *target, const uint32_t value) {
			FPL_ASSERT(target != nullptr);
			uint32_t result = _InterlockedExchange((volatile unsigned long *)target, value);
			return (result);
		}
		fpl_api int32_t AtomicExchangeS32(volatile int32_t *target, const int32_t value) {
			FPL_ASSERT(target != nullptr);
			int32_t result = _InterlockedExchange((volatile long *)target, value);
			return (result);
		}
		fpl_api uint64_t AtomicExchangeU64(volatile uint64_t *target, const uint64_t value) {
			FPL_ASSERT(target != nullptr);
			uint64_t result = _InterlockedExchange((volatile unsigned __int64 *)target, value);
			return (result);
		}
		fpl_api int64_t AtomicExchangeS64(volatile int64_t *target, const int64_t value) {
			FPL_ASSERT(target != nullptr);
			int64_t result = _InterlockedExchange64((volatile long long *)target, value);
			return (result);
		}
		fpl_api uint32_t AtomicAddU32(volatile uint32_t *value, const uint32_t addend) {
			FPL_ASSERT(value != nullptr);
			uint32_t result = _InterlockedExchangeAdd((volatile unsigned long *)value, addend);
			return (result);
		}
		fpl_api int32_t AtomicAddS32(volatile int32_t *value, const int32_t addend) {
			FPL_ASSERT(value != nullptr);
			int32_t result = _InterlockedExchangeAdd((volatile long *)value, addend);
			return (result);
		}
		fpl_api uint64_t AtomicAddU64(volatile uint64_t *value, const uint64_t addend) {
			FPL_ASSERT(value != nullptr);
			uint64_t result = _InterlockedExchangeAdd((volatile unsigned __int64 *)value, addend);
			return (result);
		}
		fpl_api int64_t AtomicAddS64(volatile int64_t *value, const int64_t addend) {
			FPL_ASSERT(value != nullptr);
			int64_t result = _InterlockedExchangeAdd64((volatile long long *)value, addend);
			return (result);
		}
		fpl_api uint32_t AtomicCompareAndExchangeU32(volatile uint32_t *dest, const uint32_t exchange, const uint32_t comparand) {
			FPL_ASSERT(dest != nullptr);
			uint32_t result = _InterlockedCompareExchange((volatile unsigned long *)dest, exchange, comparand);
			return (result);
		}
		fpl_api int32_t AtomicCompareAndExchangeS32(volatile int32_t *dest, const int32_t exchange, const int32_t comparand) {
			FPL_ASSERT(dest != nullptr);
			int32_t result = _InterlockedCompareExchange((volatile long *)dest, exchange, comparand);
			return (result);
		}
		fpl_api uint64_t AtomicAndCompareExchangeU64(volatile uint64_t *dest, const uint64_t exchange, const uint64_t comparand) {
			FPL_ASSERT(dest != nullptr);
			uint64_t result = _InterlockedCompareExchange((volatile unsigned __int64 *)dest, exchange, comparand);
			return (result);
		}
		fpl_api int64_t AtomicCompareAndExchangeS64(volatile int64_t *dest, const int64_t exchange, const int64_t comparand) {
			FPL_ASSERT(dest != nullptr);
			int64_t result = _InterlockedCompareExchange64((volatile long long *)dest, exchange, comparand);
			return (result);
		}
	}

	namespace hardware {
		fpl_api char *GetProcessorName(char *destBuffer, const uint32_t maxDestBufferLen) {
			constexpr uint32_t cpuBrandBufferSize = 0x40;

			FPL_ASSERT(destBuffer != nullptr);
			FPL_ASSERT(maxDestBufferLen >= (cpuBrandBufferSize + 1));

			int cpuInfo[4] = { -1 };
			char cpuBrandBuffer[cpuBrandBufferSize] = {};
			__cpuid(cpuInfo, 0x80000000);
			uint32_t extendedIds = cpuInfo[0];

			// Get the information associated with each extended ID. Interpret CPU brand string.
			uint32_t max = FPL_MIN(extendedIds, 0x80000004);
			for (uint32_t i = 0x80000002; i <= max; ++i) {
				__cpuid(cpuInfo, i);
				uint32_t shift = (i - 0x80000002) << 4;
				memory::MemoryCopy(cpuInfo, sizeof(cpuInfo), cpuBrandBuffer + shift);
			}

			// Copy result back to the dest buffer
			uint32_t sourceLen = strings::GetAnsiStringLength(cpuBrandBuffer);
			FPL_ASSERT(sourceLen <= maxDestBufferLen);
			strings::CopyAnsiString(cpuBrandBuffer, sourceLen, destBuffer, maxDestBufferLen);

			return(destBuffer);
		}
	}

#else
#error "Unsupported compiler for intrinsics"
#endif // defined(FPL_COMPILER_MSVC)
}

//
// ----------------------------------------------------------------------------
// WIN32 Platform
// ----------------------------------------------------------------------------
//
#if defined(FPL_PLATFORM_WIN32)

#	include <windowsx.h>	// Macros for window messages
#	include <shlobj.h>		// SHGetFolderPath
#	include <xinput.h>		// XInputGetState

#	define FPL_WIN32_GET_DEVICE_CAPS GetDeviceCaps
#	define FPL_WIN32_SWAP_BUFFERS SwapBuffers
#	define FPL_WIN32_RELEASE_DC ReleaseDC
#	define FPL_WIN32_GET_DC GetDC
#	define FPL_WIN32_BEGIN_PAINT BeginPaint
#	define FPL_WIN32_END_PAINT EndPaint
#	define FPL_WIN32_STRETCH_DIBITS StretchDIBits
#	define FPL_WIN32_CREATE_DIBSECTION CreateDIBSection
#	define FPL_WIN32_DELETE_OBJECT DeleteObject
#	define FPL_WIN32_CHOOSE_PIXEL_FORMAT ChoosePixelFormat
#	define FPL_WIN32_SET_PIXEL_FORMAT SetPixelFormat
#	define FPL_WIN32_DESCRIPE_PIXEL_FORMAT DescribePixelFormat

#	define FPL_WIN32_WGL_CREATE_CONTEXT wglCreateContext
#	define FPL_WIN32_WGL_MAKE_CURRENT wglMakeCurrent
#	define FPL_WIN32_WGL_DELETE_CONTEXT wglDeleteContext

#	define FPL_WIN32_SHOW_WINDOW ShowWindow
#	define FPL_WIN32_UPDATE_WINDOW UpdateWindow
#	define FPL_WIN32_DESTROY_WINDOW DestroyWindow
#	define FPL_WIN32_SET_WINDOW_POS SetWindowPos
#	define FPL_WIN32_GET_WINDOW_PLACEMENT GetWindowPlacement
#	define FPL_WIN32_SET_WINDOW_PLACEMENT SetWindowPlacement
#	define FPL_WIN32_GET_ASYNC_KEY_STATE GetAsyncKeyState
#	define FPL_WIN32_GET_CLIENT_RECT GetClientRect
#	define FPL_WIN32_GET_WINDOW_RECT GetWindowRect
#	define FPL_WIN32_ADJUST_WINDOW_RECT AdjustWindowRect
#	define FPL_WIN32_SET_CURSOR SetCursor
#	define FPL_WIN32_GET_CURSOR GetCursor
#	define FPL_WIN32_CHANGE_DISPLAY_SETTINGS_A ChangeDisplaySettingsA
#	define FPL_WIN32_ENUM_DISPLAY_SETTINGS_A EnumDisplaySettingsA
#	define FPL_WIN32_TRANSLATE_MESSAGE TranslateMessage
#	define FPL_WIN32_SET_WINDOW_TEXT_A SetWindowTextA
#	define FPL_WIN32_SET_WINDOW_TEXT_W SetWindowTextW

#	define FPL_WIN32_VIRTUAL_ALLOC VirtualAlloc
#	define FPL_WIN32_VIRTUAL_FREE VirtualFree
#	define FPL_WIN32_LOAD_LIBRARY LoadLibraryA
#	define FPL_WIN32_FREE_LIBRARY FreeLibrary
#	define FPL_WIN32_GET_PROC_ADDRESS GetProcAddress

#	define FPL_WIN32_QUERY_PERFORMANCE_COUNTER QueryPerformanceCounter
#	define FPL_WIN32_QUERY_PERFORMANCE_FREQUENCY QueryPerformanceFrequency
#	define FPL_WIN32_CREATE_THREAD CreateThread
#	define FPL_WIN32_SUSPEND_THREAD SuspendThread
#	define FPL_WIN32_RESUME_THREAD ResumeThread
#	define FPL_WIN32_TERMINATE_THREAD TerminateThread
#	define FPL_WIN32_WAIT_FOR_SINGLE_OBJECT WaitForSingleObject
#	define FPL_WIN32_WAIT_FOR_MULTIPLE_OBJECTS WaitForMultipleObjects
#	define FPL_WIN32_CREATE_EVENT_A CreateEventA
#	define FPL_WIN32_SET_EVENT SetEvent
#	define FPL_WIN32_SLEEP Sleep
#	define FPL_WIN32_GET_SYSTEM_INFO GetSystemInfo

#	define FPL_WIN32_CREATE_FILE_A CreateFileA
#	define FPL_WIN32_CREATE_FILE_W CreateFileW
#	define FPL_WIN32_READ_FILE ReadFile
#	define FPL_WIN32_WRITE_FILE WriteFile
#	define FPL_WIN32_SET_FILE_POINTER SetFilePointer
#	define FPL_WIN32_CLOSE_HANDLE CloseHandle
#	define FPL_WIN32_GET_FILE_SIZE GetFileSize
#	define FPL_WIN32_FIND_FIRST_FILE_A FindFirstFileA
#	define FPL_WIN32_FIND_FIRST_FILE_W FindFirstFileW
#	define FPL_WIN32_FIND_NEXT_FILE_A FindNextFileA
#	define FPL_WIN32_FIND_NEXT_FILE_W FindNextFileW
#	define FPL_WIN32_FIND_CLOSE FindClose
#	define FPL_WIN32_COPY_FILE_A CopyFileA
#	define FPL_WIN32_COPY_FILE_W CopyFileW
#	define FPL_WIN32_DELETE_FILE_A DeleteFileA
#	define FPL_WIN32_DELETE_FILE_W DeleteFileW
#	define FPL_WIN32_CREATE_DIRECTORY_A CreateDirectoryA
#	define FPL_WIN32_CREATE_DIRECTORY_W CreateDirectoryW
#	define FPL_WIN32_REMOVE_DIRECTORY_A RemoveDirectoryA
#	define FPL_WIN32_REMOVE_DIRECTORY_W RemoveDirectoryW

#	define FPL_WIN32_WIDE_CHAR_TO_MULTI_BYTE WideCharToMultiByte
#	define FPL_WIN32_MULTI_BYTE_TO_WIDE_CHAR MultiByteToWideChar

#	if defined(UNICODE)
#		define FPL_WIN32_CLASSNAME L"FPLWindowClassW"
#		define FPL_WIN32_UNNAMED_WINDOW L"Unnamed FPL Unicode Window"
#		define FPL_WIN32_STRINGCOPY strings::CopyWideString
#		define FPL_WIN32_GETSTRINGLENGTH strings::GetWideStringLength
#		define FPL_WIN32_ANSI_TO_STRING strings::AnsiStringToWideString
#		define FPL_WIN32_WNDCLASSEX WNDCLASSEXW
#		define FPL_WIN32_REGISTER_CLASS_EX RegisterClassExW
#		define FPL_WIN32_UNREGISTER_CLASS UnregisterClassW
#		define FPL_WIN32_CREATE_WINDOW_EX CreateWindowExW
#		define FPL_WIN32_DEF_WINDOW_PROC DefWindowProcW
#		define FPL_WIN32_GET_WINDOW_LONG_PTR GetWindowLongPtrW
#		define FPL_WIN32_SET_WINDOW_LONG_PTR SetWindowLongPtrW
#		define FPL_WIN32_GET_WINDOW_LONG GetWindowLongW
#		define FPL_WIN32_SET_WINDOW_LONG SetWindowLongW
#		define FPL_WIN32_PEEK_MESSAGE PeekMessageW
#		define FPL_WIN32_POST_MESSAGE PostMessageW
#		define FPL_WIN32_DISPATCH_MESSAGE DispatchMessageW
#		define FPL_WIN32_MAP_VIRTUAL_KEY MapVirtualKeyW
#		define FPL_WIN32_SET_WINDOW_TEXT SetWindowTextW

#		define FPL_WIN32_GET_MODULE_FILENAME GetModuleFileNameW
#		define FPL_WIN32_SH_GET_FOLDER_PATH SHGetFolderPathW
#	else
#		define FPL_WIN32_CLASSNAME "FPLWindowClassA"
#		define FPL_WIN32_UNNAMED_WINDOW "Unnamed FPL Ansi Window"
#		define FPL_WIN32_STRINGCOPY strings::CopyAnsiString
#		define FPL_WIN32_GETSTRINGLENGTH strings::GetAnsiStringLength
#		define FPL_WIN32_ANSI_TO_STRING strings::CopyAnsiString
#		define FPL_WIN32_WNDCLASSEX WNDCLASSEXA
#		define FPL_WIN32_REGISTER_CLASS_EX RegisterClassExA
#		define FPL_WIN32_UNREGISTER_CLASS UnregisterClassA
#		define FPL_WIN32_CREATE_WINDOW_EX CreateWindowExA
#		define FPL_WIN32_DEF_WINDOW_PROC DefWindowProcA
#		define FPL_WIN32_GET_WINDOW_LONG_PTR GetWindowLongPtrA
#		define FPL_WIN32_SET_WINDOW_LONG_PTR SetWindowLongPtrA
#		define FPL_WIN32_GET_WINDOW_LONG GetWindowLongA
#		define FPL_WIN32_SET_WINDOW_LONG SetWindowLongA
#		define FPL_WIN32_PEEK_MESSAGE PeekMessageA
#		define FPL_WIN32_POST_MESSAGE PostMessageA
#		define FPL_WIN32_DISPATCH_MESSAGE DispatchMessageA
#		define FPL_WIN32_MAP_VIRTUAL_KEY MapVirtualKeyA
#		define FPL_WIN32_SET_WINDOW_TEXT SetWindowTextA

#		define FPL_WIN32_GET_MODULE_FILENAME GetModuleFileNameA
#		define FPL_WIN32_SH_GET_FOLDER_PATH SHGetFolderPathA
#	endif // defined(UNICODE)

//
// XInputGetState
//
#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef XINPUT_GET_STATE(xinput_get_state);
XINPUT_GET_STATE(Win32XInputGetStateStub) {
	return(ERROR_DEVICE_NOT_CONNECTED);
}

//
// WGL non-legacy types
//
#undef WGL_CHOOSE_PIXEL_FORMAT_ARB
#define WGL_CHOOSE_PIXEL_FORMAT_ARB(name) BOOL WINAPI name(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats)
typedef WGL_CHOOSE_PIXEL_FORMAT_ARB(wgl_choose_pixel_format_arb);

#undef WGL_CREATE_CONTEXT_ATTRIBS_ARB
#define WGL_CREATE_CONTEXT_ATTRIBS_ARB(name) HGLRC WINAPI name(HDC hDC, HGLRC hShareContext, const int *attribList)
typedef WGL_CREATE_CONTEXT_ATTRIBS_ARB(wgl_create_context_attribs_arb);

#undef WGL_SWAP_INTERVAL_EXT
#define WGL_SWAP_INTERVAL_EXT(name) BOOL WINAPI name(int interval)
typedef WGL_SWAP_INTERVAL_EXT(wgl_swap_interval_ext);

#if !defined(FPL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
#	define FPL_GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#	define FPL_GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#	define FPL_GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#	define FPL_GL_CONTEXT_FLAG_NO_ERROR_BIT 0x00000008
#	define FPL_GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#	define FPL_GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

#if !defined(FPL_WGL_CONTEXT_DEBUG_BIT_ARB)
#	define FPL_WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#	define FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#	define FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#	define FPL_WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#	define FPL_WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#	define FPL_WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#	define FPL_WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#	define FPL_WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#	define FPL_WGL_CONTEXT_FLAGS_ARB 0x2094
#endif

#if !defined(FPL_WGL_DRAW_TO_WINDOW_ARB)
#define FPL_WGL_DRAW_TO_WINDOW_ARB 0x2001
#define FPL_WGL_ACCELERATION_ARB 0x2003
#define FPL_WGL_SWAP_METHOD_ARB 0x2007
#define FPL_WGL_SUPPORT_OPENGL_ARB 0x2010
#define FPL_WGL_DOUBLE_BUFFER_ARB 0x2011
#define FPL_WGL_PIXEL_TYPE_ARB 0x2013
#define FPL_WGL_COLOR_BITS_ARB 0x2014
#define FPL_WGL_DEPTH_BITS_ARB 0x2022
#define FPL_WGL_STENCIL_BITS_ARB 0x2023
#define FPL_WGL_FULL_ACCELERATION_ARB 0x2027
#define FPL_WGL_SWAP_EXCHANGE_ARB 0x2028
#define FPL_WGL_TYPE_RGBA_ARB 0x202B
#endif

struct wgl_extensions {
	wgl_choose_pixel_format_arb *choosePixelFormatArb;
	wgl_create_context_attribs_arb *createContextAttribsArb;
	wgl_swap_interval_ext *swapIntervalExt;
};

struct win32_functions {
	xinput_get_state *xinputGetState = Win32XInputGetStateStub;
};

fpl_globalvar wgl_extensions globalWGLExtensions = {};
fpl_globalvar win32_functions globalWin32Functions = {};

namespace fpl {

	// Win32 internal functions
#	if defined(UNICODE)
	typedef wchar_t win32_char_internal;
#	else
	typedef char win32_char_internal;
#	endif // defined(UNICODE)

#	if defined(FPL_ENABLE_WINDOW)
	struct Win32WindowState_Internal {
		win32_char_internal windowClass[256];
		HWND windowHandle;
		HDC deviceContext;
		HCURSOR defaultCursor;
		WINDOWPLACEMENT lastWindowPlacement;
		uint32_t lastWindowWidth;
		uint32_t lastWindowHeight;
		bool isRunning;
		bool isCursorActive;
	};

	struct Win32XInputState_Internal {
		HMODULE xinputLibrary;
		bool isConnected[XUSER_MAX_COUNT];
	};

#	else
	typedef void *Win32WindowState_Internal;
	typedef void *Win32XInputState_Internal;
#	endif

#	if defined(FPL_ENABLE_VIDEO_OPENGL)
	struct Win32VideoOpenGLState_Internal {
		HGLRC renderingContext;
	};
#	else
	typedef void *Win32VideoOpenGLState_Internal;
#	endif // defined(FPL_ENABLE_VIDEO_OPENGL)

#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
	struct Win32VideoSoftwareState_Internal {
		video::VideoBackBuffer context;
		BITMAPINFO bitmapInfo;
		HBITMAP deviceBitmap;
	};
#	else
	typedef void *Win32VideoSoftwareState_Internal;
#	endif // defined(FPL_ENABLE_VIDEO_SOFTWARE)

	struct Win32VideoState_Internal {
		Win32VideoOpenGLState_Internal opengl;
		Win32VideoSoftwareState_Internal software;
	};

	struct Win32State_Internal {
		bool isInitialized;
		HINSTANCE appInstance;
		LARGE_INTEGER performanceFrequency;
		InitFlags initFlags;
		Settings initSettings;
		Settings currentSettings;
		VideoDriverType videoDriverType;
		Win32WindowState_Internal window;
		Win32VideoState_Internal video;
		Win32XInputState_Internal xinput;
	};

	fpl_globalvar Win32State_Internal *globalWin32State_Internal = nullptr;
	fpl_globalvar HINSTANCE globalWin32AppInstance_Internal = nullptr;

	// Win32 public hardware
	namespace hardware {
		fpl_api uint32_t GetProcessorCoreCount() {
			SYSTEM_INFO sysInfo = {};
			FPL_WIN32_GET_SYSTEM_INFO(&sysInfo);
			// @NOTE(final): For now this returns the number of logical processors, which is the actual core count in most cases.
			uint32_t result = sysInfo.dwNumberOfProcessors;
			return(result);
		}
	}

	// Win32 public console
	namespace console {
		fpl_api void ConsoleOut(const char *text) {
			FPL_ASSERT(text != nullptr);
			fprintf(stdout, "%s", text);
		}
		fpl_api void ConsoleFormatOut(const char *format, ...) {
			FPL_ASSERT(format != nullptr);
			va_list vaList;
			va_start(vaList, format);
			vfprintf(stdout, format, vaList);
			va_end(vaList);
		}
		fpl_api void ConsoleError(const char *text) {
			FPL_ASSERT(text != nullptr);
			fprintf(stderr, "%s", text);
		}
		fpl_api void ConsoleFormatError(const char *format, ...) {
			FPL_ASSERT(format != nullptr);
			va_list vaList;
			va_start(vaList, format);
			vfprintf(stderr, format, vaList);
			va_end(vaList);
		}
	}

	//
	// Win32 Public Threading
	//
	namespace threading {

		fpl_internal DWORD WINAPI Win32ThreadProc_Internal(void *data) {
			DWORD result = 0;
			ThreadContext *context = (ThreadContext *)data;
			FPL_ASSERT(context != nullptr);
			if (context->runFunc != nullptr) {
				context->runFunc(*context, context->data);
			}
			return(result);
		}

		fpl_api ThreadContext &ThreadCreate(run_thread_function *runFunc, void *data, const bool autoStart) {
			FPL_ASSERT(globalThreadState_Internal.threadCount < MAX_THREAD_COUNT);
			ThreadContext *result = &globalThreadState_Internal.threads[globalThreadState_Internal.threadCount++];
			*result = {};

			DWORD creationFlags = 0;
			if (!autoStart) {
				creationFlags |= CREATE_SUSPENDED;
			}
			DWORD threadId = 0;
			HANDLE handle = FPL_WIN32_CREATE_THREAD(nullptr, 0, Win32ThreadProc_Internal, result, creationFlags, &threadId);
			result->id = threadId;
			result->internalHandle = (void *)handle;
			result->runFunc = runFunc;
			result->data = data;
			if (autoStart) {
				result->state = ThreadState::Running;
			} else {
				result->state = ThreadState::Stopped;
			}
			return(*result);
		}

		fpl_api void ThreadSleep(const uint32_t milliseconds) {
			FPL_WIN32_SLEEP((DWORD)milliseconds);
		}

		fpl_api bool ThreadSuspend(ThreadContext &context) {
			FPL_ASSERT(context.internalHandle != nullptr);
			HANDLE handle = (HANDLE)context.internalHandle;
			DWORD err = FPL_WIN32_SUSPEND_THREAD(handle);
			bool result = err != -1;
			if (result) {
				context.state = ThreadState::Suspended;
			}
			return(result);
		}

		fpl_api bool ThreadResume(ThreadContext &context) {
			FPL_ASSERT(context.internalHandle != nullptr);
			HANDLE handle = (HANDLE)context.internalHandle;
			DWORD err = FPL_WIN32_RESUME_THREAD(handle);
			bool result = err != -1;
			if (result) {
				context.state = ThreadState::Running;
			}
			return(result);
		}

		fpl_api void ThreadStop(ThreadContext &context) {
			FPL_ASSERT(context.internalHandle != nullptr);
			HANDLE handle = (HANDLE)context.internalHandle;
			FPL_WIN32_TERMINATE_THREAD(handle, 0);
			context = {};

			// @TODO(final): Decrease thread count and move thread to the end (Use a free list or something)
		}

		fpl_api void ThreadWaitForSingle(const ThreadContext &context, const uint32_t maxMilliseconds) {
			FPL_ASSERT(context.id > 0);
			FPL_ASSERT(context.internalHandle != nullptr);
			HANDLE handle = (HANDLE)context.internalHandle;
			FPL_WIN32_WAIT_FOR_SINGLE_OBJECT(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
		}

		fpl_api void ThreadWaitForMultiple(const ThreadContext *contexts, const uint32_t count, const uint32_t maxMilliseconds) {
			FPL_ASSERT(contexts != nullptr);
			FPL_ASSERT(count <= MAX_THREAD_COUNT);
			HANDLE threadHandles[MAX_THREAD_COUNT];
			for (uint32_t index = 0; index < count; ++index) {
				const ThreadContext &context = contexts[index];
				FPL_ASSERT(context.id > 0);
				FPL_ASSERT(context.internalHandle != nullptr);
				HANDLE handle = (HANDLE)context.internalHandle;
				threadHandles[index] = handle;
			}
			FPL_WIN32_WAIT_FOR_MULTIPLE_OBJECTS(count, threadHandles, TRUE, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
		}

		fpl_api ThreadMutex MutexCreate() {
			ThreadMutex result = {};
			HANDLE handle = FPL_WIN32_CREATE_EVENT_A(nullptr, FALSE, TRUE, nullptr);
			if (handle != nullptr) {
				result.isValid = true;
				result.internalHandle = (void *)handle;
			}
			return(result);
		}

		fpl_api void MutexDestroy(ThreadMutex &mutex) {
			FPL_ASSERT(mutex.isValid);
			FPL_ASSERT(mutex.internalHandle != nullptr);
			HANDLE handle = (HANDLE)mutex.internalHandle;
			FPL_WIN32_CLOSE_HANDLE(handle);
			mutex = {};
		}

		fpl_api void MutexLock(ThreadMutex &mutex, const uint32_t maxMilliseconds) {
			FPL_ASSERT(mutex.isValid);
			FPL_ASSERT(mutex.internalHandle != nullptr);
			HANDLE handle = (HANDLE)mutex.internalHandle;
			FPL_WIN32_WAIT_FOR_SINGLE_OBJECT(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE);
		}

		fpl_api void MutexUnlock(ThreadMutex &mutex) {
			FPL_ASSERT(mutex.isValid);
			FPL_ASSERT(mutex.internalHandle != nullptr);
			HANDLE handle = (HANDLE)mutex.internalHandle;
			FPL_WIN32_SET_EVENT(handle);
		}

		fpl_api ThreadSignal SignalCreate() {
			ThreadSignal result = {};
			HANDLE handle = FPL_WIN32_CREATE_EVENT_A(nullptr, FALSE, FALSE, nullptr);
			if (handle != nullptr) {
				result.isValid = true;
				result.internalHandle = (void *)handle;
			}
			return(result);
		}

		fpl_api void SignalDestroy(ThreadSignal &signal) {
			FPL_ASSERT(signal.isValid);
			FPL_ASSERT(signal.internalHandle != nullptr);
			HANDLE handle = (HANDLE)signal.internalHandle;
			FPL_WIN32_CLOSE_HANDLE(handle);
			signal = {};
		}

		fpl_api bool SignalWait(ThreadSignal &signal, const uint32_t maxMilliseconds) {
			FPL_ASSERT(signal.isValid);
			FPL_ASSERT(signal.internalHandle != nullptr);
			HANDLE handle = (HANDLE)signal.internalHandle;
			bool result = (FPL_WIN32_WAIT_FOR_SINGLE_OBJECT(handle, maxMilliseconds < UINT32_MAX ? maxMilliseconds : INFINITE) == WAIT_OBJECT_0);
			return(result);
		}

		fpl_api bool SignalWakeUp(ThreadSignal &signal) {
			FPL_ASSERT(signal.isValid);
			FPL_ASSERT(signal.internalHandle != nullptr);
			HANDLE handle = (HANDLE)signal.internalHandle;
			bool result = FPL_WIN32_SET_EVENT(handle) == TRUE;
			return(result);
		}
	}

	//
	// Win32 Public Memory
	//
	namespace memory {
		fpl_api void *MemoryAllocate(const size_t size) {
			FPL_ASSERT(size > 0);
			void *result = FPL_WIN32_VIRTUAL_ALLOC(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			return(result);
		}

		fpl_api void MemoryFree(void *ptr) {
			FPL_ASSERT(ptr != nullptr);
			FPL_WIN32_VIRTUAL_FREE(ptr, 0, MEM_FREE);
		}

		fpl_api void *MemoryStackAllocate(const size_t size) {
			FPL_ASSERT(size > 0);
			// @TODO: Is this safe not to include <malloc.h>?
			void *result = _malloca(size);
			return(result);
		}
	}

	//
	// Win32 Public File
	//
	namespace files {
		fpl_api FileHandle OpenBinaryFile(const char *filePath) {
			FPL_ASSERT(filePath != nullptr);
			FileHandle result = {};
			HANDLE win32FileHandle = FPL_WIN32_CREATE_FILE_A(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				result.isValid = true;
				result.internalHandle = (void *)win32FileHandle;
			}
			return(result);
		}
		fpl_api FileHandle OpenBinaryFile(const wchar_t *filePath) {
			FPL_ASSERT(filePath != nullptr);
			FileHandle result = {};
			HANDLE win32FileHandle = FPL_WIN32_CREATE_FILE_W(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				result.isValid = true;
				result.internalHandle = (void *)win32FileHandle;
			}
			return(result);
		}

		fpl_api FileHandle CreateBinaryFile(const char *filePath) {
			FPL_ASSERT(filePath != nullptr);
			FileHandle result = {};
			HANDLE win32FileHandle = FPL_WIN32_CREATE_FILE_A(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				result.isValid = true;
				result.internalHandle = (void *)win32FileHandle;
			}
			return(result);
		}
		fpl_api FileHandle CreateBinaryFile(const wchar_t *filePath) {
			FPL_ASSERT(filePath != nullptr);
			FileHandle result = {};
			HANDLE win32FileHandle = FPL_WIN32_CREATE_FILE_W(filePath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				result.isValid = true;
				result.internalHandle = (void *)win32FileHandle;
			}
			return(result);
		}

		fpl_api uint32_t ReadFileBlock32(const FileHandle &fileHandle, const uint32_t sizeToRead, void *targetBuffer, const uint32_t maxTargetBufferSize) {
			FPL_ASSERT(targetBuffer != nullptr);
			FPL_ASSERT(sizeToRead > 0);
			uint32_t result = 0;
			if (fileHandle.isValid) {
				FPL_ASSERT(fileHandle.internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle;
				DWORD bytesRead = 0;
				if (FPL_WIN32_READ_FILE(win32FileHandle, targetBuffer, (DWORD)sizeToRead, &bytesRead, nullptr) == TRUE) {
					result = bytesRead;
				}
			}
			return(result);
		}

		fpl_api uint32_t WriteFileBlock32(const FileHandle &fileHandle, void *sourceBuffer, const uint32_t sourceSize) {
			FPL_ASSERT(sourceBuffer != nullptr);
			FPL_ASSERT(sourceSize > 0);
			uint32_t result = 0;
			if (fileHandle.isValid) {
				FPL_ASSERT(fileHandle.internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle;
				DWORD bytesWritten = 0;
				if (FPL_WIN32_WRITE_FILE(win32FileHandle, sourceBuffer, (DWORD)sourceSize, &bytesWritten, nullptr) == TRUE) {
					result = bytesWritten;
				}
			}
			return(result);
		}

		fpl_api void SetFilePosition32(const FileHandle &fileHandle, const uint32_t position, const FilePositionMode mode) {
			if (fileHandle.isValid) {
				FPL_ASSERT(fileHandle.internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle;
				DWORD moveMethod = FILE_BEGIN;
				if (mode == FilePositionMode::Current) {
					moveMethod = FILE_CURRENT;
				} else if (mode == FilePositionMode::End) {
					moveMethod = FILE_END;
				}
				FPL_WIN32_SET_FILE_POINTER(win32FileHandle, (LONG)position, nullptr, moveMethod);
			}
		}

		fpl_api uint32_t GetFilePosition32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if (fileHandle.isValid) {
				FPL_ASSERT(fileHandle.internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle;
				DWORD filePosition = FPL_WIN32_SET_FILE_POINTER(win32FileHandle, 0L, nullptr, FILE_CURRENT);
				if (filePosition != INVALID_SET_FILE_POINTER) {
					result = filePosition;
				}
			}
			return(result);
		}

		fpl_api void CloseFile(FileHandle &fileHandle) {
			if (fileHandle.isValid) {
				FPL_ASSERT(fileHandle.internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle;
				FPL_WIN32_CLOSE_HANDLE(win32FileHandle);
			}
			fileHandle = {};
		}

		fpl_api uint32_t GetFileSize32(const char *filePath) {
			uint32_t result = 0;
			HANDLE win32FileHandle = FPL_WIN32_CREATE_FILE_A(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (win32FileHandle != INVALID_HANDLE_VALUE) {
				DWORD fileSize = FPL_WIN32_GET_FILE_SIZE(win32FileHandle, nullptr);
				result = fileSize;
				FPL_WIN32_CLOSE_HANDLE(win32FileHandle);
			}
			return(result);
		}

		fpl_api uint32_t GetFileSize32(const FileHandle &fileHandle) {
			uint32_t result = 0;
			if (fileHandle.isValid) {
				FPL_ASSERT(fileHandle.internalHandle != INVALID_HANDLE_VALUE);
				HANDLE win32FileHandle = (void *)fileHandle.internalHandle;
				DWORD fileSize = FPL_WIN32_GET_FILE_SIZE(win32FileHandle, nullptr);
				result = fileSize;
			}
			return(result);
		}

		fpl_api bool FileExists(const char *filePath) {
			bool result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FPL_WIN32_FIND_FIRST_FILE_A(filePath, &findData);
			if (searchHandle != INVALID_HANDLE_VALUE) {
				result = !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
				FPL_WIN32_FIND_CLOSE(searchHandle);
			}
			return(result);
		}

		fpl_api bool CopyAFile(const char *sourceFilePath, const char *targetFilePath, const bool overwrite) {
			bool result = (FPL_WIN32_COPY_FILE_A(sourceFilePath, targetFilePath, !overwrite) == TRUE);
			return(result);
		}

		fpl_api bool DeleteAFile(const char *filePath) {
			bool result = (FPL_WIN32_DELETE_FILE_A(filePath) == TRUE);
			return(result);
		}

		fpl_api bool DirectoryExists(const char *path) {
			bool result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FPL_WIN32_FIND_FIRST_FILE_A(path, &findData);
			if (searchHandle != INVALID_HANDLE_VALUE) {
				result = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
				FPL_WIN32_CLOSE_HANDLE(searchHandle);
			}
			return(result);
		}

		fpl_api bool CreateDirectories(const char *path) {
			BOOL result = FPL_WIN32_CREATE_DIRECTORY_A(path, nullptr);
			return(result > 0 ? 1 : 0);
		}
		fpl_api bool RemoveEmptyDirectory(const char *path) {
			bool result = (FPL_WIN32_REMOVE_DIRECTORY_A(path) == TRUE);
			return(result);
		}
		fpl_internal void Win32FillFileEntry_Internal(WIN32_FIND_DATAA *findData, FileEntry *entry) {
			using namespace strings;

			CopyAnsiString(findData->cFileName, GetAnsiStringLength(findData->cFileName), entry->path, FPL_ARRAYCOUNT(entry->path));

			entry->type = FileEntryType::Unknown;
			if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				entry->type = FileEntryType::Directory;
			} else if (
				(findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ||
				(findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)) {
				entry->type = FileEntryType::File;
			}

			entry->attributes = FileAttributeFlags::None;
			if (findData->dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
				entry->attributes = FileAttributeFlags::Normal;
			} else {
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
					entry->attributes |= FileAttributeFlags::Hidden;
				}
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
					entry->attributes |= FileAttributeFlags::ReadOnly;
				}
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
					entry->attributes |= FileAttributeFlags::Archive;
				}
				if (findData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
					entry->attributes |= FileAttributeFlags::System;
				}
			}
		}
		fpl_api bool ListFilesBegin(const char *pathAndFilter, FileEntry *firstEntry) {
			FPL_ASSERT(pathAndFilter != nullptr);
			FPL_ASSERT(firstEntry != nullptr);
			bool result = false;
			WIN32_FIND_DATAA findData;
			HANDLE searchHandle = FPL_WIN32_FIND_FIRST_FILE_A(pathAndFilter, &findData);
			*firstEntry = {};
			if (searchHandle != INVALID_HANDLE_VALUE) {
				firstEntry->internalHandle = (void *)searchHandle;
				Win32FillFileEntry_Internal(&findData, firstEntry);
				result = true;
			}
			return(result);
		}
		fpl_api bool ListFilesNext(FileEntry *nextEntry) {
			FPL_ASSERT(nextEntry != nullptr);
			bool result = false;
			if (nextEntry->internalHandle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = (HANDLE)nextEntry->internalHandle;
				WIN32_FIND_DATAA findData;
				if (FPL_WIN32_FIND_NEXT_FILE_A(searchHandle, &findData)) {
					Win32FillFileEntry_Internal(&findData, nextEntry);
					result = true;
				}
			}
			return(result);
		}
		fpl_api void ListFilesEnd(FileEntry *lastEntry) {
			FPL_ASSERT(lastEntry != nullptr);
			if (lastEntry->internalHandle != INVALID_HANDLE_VALUE) {
				HANDLE searchHandle = (HANDLE)lastEntry->internalHandle;
				FPL_WIN32_FIND_CLOSE(searchHandle);
			}
			*lastEntry = {};
		}
	}

	//
	// Win32 Public Path/Directories
	//
	namespace paths {
	#	if defined(UNICODE)
		fpl_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			wchar_t modulePath[MAX_PATH];
			FPL_WIN32_GET_MODULE_FILENAME(nullptr, modulePath, MAX_PATH);
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			strings::WideStringToAnsiString(modulePath, strings::GetWideStringLength(modulePath), destPath, maxDestLen);
			return(destPath);
		}
	#	else
		fpl_api char *GetExecutableFilePath(char *destPath, const uint32_t maxDestLen) {
			char modulePath[MAX_PATH];
			FPL_WIN32_GET_MODULE_FILENAME(nullptr, modulePath, MAX_PATH);
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			strings::CopyAnsiString(modulePath, strings::GetAnsiStringLength(modulePath), destPath, maxDestLen);
			return(destPath);
		}
	#	endif

	#	if defined(UNICODE)
		fpl_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			wchar_t homePath[MAX_PATH];
			FPL_WIN32_SH_GET_FOLDER_PATH(nullptr, CSIDL_PROFILE, nullptr, 0, homePath);
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			strings::WideStringToAnsiString(homePath, strings::GetWideStringLength(homePath), destPath, maxDestLen);
			return(destPath);
		}
	#else
		fpl_api char *GetHomePath(char *destPath, const uint32_t maxDestLen) {
			char homePath[MAX_PATH];
			FPL_WIN32_SH_GET_FOLDER_PATH(nullptr, CSIDL_PROFILE, nullptr, 0, homePath);
			FPL_ASSERT(destPath != nullptr);
			FPL_ASSERT(maxDestLen >= (MAX_PATH + 1));
			strings::CopyAnsiString(homePath, strings::GetAnsiStringLength(homePath), destPath, maxDestLen);
			return(destPath);
		}
	#endif
	}

	//
	// Win32 Public Timing
	//
	namespace timings {
		fpl_api double GetHighResolutionTimeInSeconds() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			LARGE_INTEGER time;
			FPL_WIN32_QUERY_PERFORMANCE_COUNTER(&time);
			double result = time.QuadPart / (double)globalWin32State_Internal->performanceFrequency.QuadPart;
			return(result);
		}
	};

	//
	// Win32 Public String
	//
	namespace strings {
		fpl_api char *WideStringToAnsiString(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *ansiDest, const uint32_t maxAnsiDestLen) {
			uint32_t requiredLen = FPL_WIN32_WIDE_CHAR_TO_MULTI_BYTE(CP_ACP, 0, wideSource, maxWideSourceLen, nullptr, 0, nullptr, nullptr);
			FPL_ASSERT(maxAnsiDestLen >= (requiredLen + 1));
			FPL_WIN32_WIDE_CHAR_TO_MULTI_BYTE(CP_ACP, 0, wideSource, maxWideSourceLen, ansiDest, maxAnsiDestLen, nullptr, nullptr);
			ansiDest[requiredLen] = 0;
			return(ansiDest);
		}
		fpl_api char *WideStringToUTF8String(const wchar_t *wideSource, const uint32_t maxWideSourceLen, char *utf8Dest, const uint32_t maxUtf8DestLen) {
			uint32_t requiredLen = FPL_WIN32_WIDE_CHAR_TO_MULTI_BYTE(CP_UTF8, 0, wideSource, maxWideSourceLen, nullptr, 0, nullptr, nullptr);
			FPL_ASSERT(maxUtf8DestLen >= (requiredLen + 1));
			FPL_WIN32_WIDE_CHAR_TO_MULTI_BYTE(CP_UTF8, 0, wideSource, maxWideSourceLen, utf8Dest, maxUtf8DestLen, nullptr, nullptr);
			utf8Dest[requiredLen] = 0;
			return(utf8Dest);
		}
		fpl_api wchar_t *AnsiStringToWideString(const char *ansiSource, const uint32_t ansiSourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			uint32_t requiredLen = FPL_WIN32_MULTI_BYTE_TO_WIDE_CHAR(CP_ACP, 0, ansiSource, ansiSourceLen, nullptr, 0);
			FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
			FPL_WIN32_MULTI_BYTE_TO_WIDE_CHAR(CP_ACP, 0, ansiSource, ansiSourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
		fpl_api wchar_t *UTF8StringToWideString(const char *utf8Source, const uint32_t utf8SourceLen, wchar_t *wideDest, const uint32_t maxWideDestLen) {
			uint32_t requiredLen = FPL_WIN32_MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, 0, utf8Source, utf8SourceLen, nullptr, 0);
			FPL_ASSERT(maxWideDestLen >= (requiredLen + 1));
			FPL_WIN32_MULTI_BYTE_TO_WIDE_CHAR(CP_UTF8, 0, utf8Source, utf8SourceLen, wideDest, maxWideDestLen);
			wideDest[requiredLen] = 0;
			return(wideDest);
		}
	}

	//
	// Win32 Public Core
	//
	namespace library {
		fpl_api DynamicLibraryHandle DynamicLibraryLoad(const char *libraryFilePath) {
			FPL_ASSERT(libraryFilePath != nullptr);
			DynamicLibraryHandle result = {};
			HMODULE libModule = FPL_WIN32_LOAD_LIBRARY(libraryFilePath);
			if (libModule != nullptr) {
				result.internalHandle = (void *)libModule;
				result.isValid = true;
			}
			return(result);
		}
		fpl_api void *GetDynamicLibraryProc(const DynamicLibraryHandle &handle, const char *name) {
			void *result = nullptr;
			if (handle.isValid) {
				FPL_ASSERT(handle.internalHandle != nullptr);
				HMODULE libModule = (HMODULE)handle.internalHandle;
				result = (void *)FPL_WIN32_GET_PROC_ADDRESS(libModule, name);
			}
			return(result);
		}
		fpl_api void DynamicLibraryUnload(DynamicLibraryHandle &handle) {
			if (handle.isValid) {
				FPL_ASSERT(handle.internalHandle != nullptr);
				HMODULE libModule = (HMODULE)handle.internalHandle;
				FPL_WIN32_FREE_LIBRARY(libModule);
			}
			handle = {};
		}
	}



	//
	// Win32 Public Video
	//
#	if defined(FPL_ENABLE_WINDOW)
	namespace video {

	#	if defined(FPL_ENABLE_VIDEO_OPENGL)
		fpl_internal bool Win32CreateVideoOpenGL_Internal(Win32State_Internal &win32State, const VideoSettings &videoSettings) {
			HDC deviceContext = win32State.window.deviceContext;
			HWND handle = win32State.window.windowHandle;

			PIXELFORMATDESCRIPTOR pfd = {};
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			pfd.cAlphaBits = 8;
			pfd.iLayerType = PFD_MAIN_PLANE;

			int pixelFormat = FPL_WIN32_CHOOSE_PIXEL_FORMAT(deviceContext, &pfd);
			if (!pixelFormat) {
				PushError_Internal("[Win32] Failed choosing RGBA Legacy Pixelformat for Color/Depth/Alpha (%d,%d,%d) and DC '%x'\n", pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
				return false;
			}

			if (!FPL_WIN32_SET_PIXEL_FORMAT(deviceContext, pixelFormat, &pfd)) {
				PushError_Internal("[Win32] Failed setting RGBA Pixelformat '%d' for Color/Depth/Alpha (%d,%d,%d and DC '%x')\n", pixelFormat, pfd.cColorBits, pfd.cDepthBits, pfd.cAlphaBits, deviceContext);
				return false;
			}

			FPL_WIN32_DESCRIPE_PIXEL_FORMAT(deviceContext, pixelFormat, sizeof(pfd), &pfd);

			HGLRC legacyRenderingContext = FPL_WIN32_WGL_CREATE_CONTEXT(deviceContext);
			if (!legacyRenderingContext) {
				PushError_Internal("[Win32] Failed creating Legacy OpenGL Rendering Context for DC '%x')\n", deviceContext);
				return false;
			}

			if (!FPL_WIN32_WGL_MAKE_CURRENT(deviceContext, legacyRenderingContext)) {
				PushError_Internal("[Win32] Failed activating Legacy OpenGL Rendering Context for DC '%x' and RC '%x')\n", deviceContext, legacyRenderingContext);
				FPL_WIN32_WGL_DELETE_CONTEXT(legacyRenderingContext);
				return false;
			}

			// Load extensions
			globalWGLExtensions.choosePixelFormatArb = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
			globalWGLExtensions.createContextAttribsArb = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
			globalWGLExtensions.swapIntervalExt = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");

			FPL_WIN32_WGL_MAKE_CURRENT(nullptr, nullptr);

			HGLRC activeRenderingContext;
			if (videoSettings.profile != VideoCompabilityProfile::Legacy) {
				// @NOTE(final): This is only available in OpenGL 3.0+
				if (!(videoSettings.majorVersion >= 3 && videoSettings.minorVersion >= 0)) {
					PushError_Internal("[Win32] You have not specified the 'majorVersion' and 'minorVersion' in the VideoSettings!\n");
					return false;
				}

				if (!globalWGLExtensions.choosePixelFormatArb) {
					PushError_Internal("[Win32] wglChoosePixelFormatARB is not available, please select a different video profile!\n");
					return false;
				}
				if (!globalWGLExtensions.createContextAttribsArb) {
					PushError_Internal("[Win32] wglCreateContextAttribsARB is not available, please select a different video profile!\n");
					return false;
				}

				int contextAttribIndex = 0;
				int contextAttribList[20 + 1] = {};
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MAJOR_VERSION_ARB;
				contextAttribList[contextAttribIndex++] = (int)videoSettings.majorVersion;
				contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_MINOR_VERSION_ARB;
				contextAttribList[contextAttribIndex++] = (int)videoSettings.minorVersion;
				if (videoSettings.profile == VideoCompabilityProfile::Core) {
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_PROFILE_MASK_ARB;
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
				} else {
					FPL_ASSERT(videoSettings.profile == VideoCompabilityProfile::Forward);
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FLAGS_ARB;
					contextAttribList[contextAttribIndex++] = FPL_WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
				}

				HGLRC newContext = globalWGLExtensions.createContextAttribsArb(deviceContext, 0, contextAttribList);
				if (newContext) {
					if (!FPL_WIN32_WGL_MAKE_CURRENT(deviceContext, newContext)) {
						PushError_Internal("[Win32] Warning: Failed activating Modern OpenGL Rendering Context for version (%llu.%llu) and DC '%x') -> Fallback to legacy context.\n", videoSettings.majorVersion, videoSettings.minorVersion, deviceContext);

						FPL_WIN32_WGL_DELETE_CONTEXT(newContext);
						newContext = nullptr;

						// Fallback to legacy context
						FPL_WIN32_WGL_MAKE_CURRENT(deviceContext, legacyRenderingContext);
						activeRenderingContext = legacyRenderingContext;
					} else {
						// Destroy legacy rendering context
						FPL_WIN32_WGL_DELETE_CONTEXT(legacyRenderingContext);
						legacyRenderingContext = nullptr;
						activeRenderingContext = newContext;
					}
				} else {
					PushError_Internal("[Win32] Warning: Failed creating Modern OpenGL Rendering Context for version (%llu.%llu) and DC '%x') -> Fallback to legacy context.\n", videoSettings.majorVersion, videoSettings.minorVersion, deviceContext);

					// Fallback to legacy context
					FPL_WIN32_WGL_MAKE_CURRENT(deviceContext, legacyRenderingContext);
					activeRenderingContext = legacyRenderingContext;
				}
			} else {
				FPL_WIN32_WGL_MAKE_CURRENT(deviceContext, legacyRenderingContext);
				activeRenderingContext = legacyRenderingContext;
			}

			FPL_ASSERT(activeRenderingContext != nullptr);

			win32State.video.opengl.renderingContext = activeRenderingContext;

			// Set vertical syncronisation if available
			if (globalWGLExtensions.swapIntervalExt != nullptr) {
				int swapInterval = videoSettings.isVSync ? 1 : 0;
				globalWGLExtensions.swapIntervalExt(swapInterval);
			}

			return true;
		}

		fpl_internal void Win32ReleaseVideoOpenGLContext_Internal(Win32State_Internal &win32State) {
			if (win32State.video.opengl.renderingContext) {
				globalWGLExtensions = {};
				FPL_WIN32_WGL_MAKE_CURRENT(nullptr, nullptr);
				FPL_WIN32_WGL_DELETE_CONTEXT(win32State.video.opengl.renderingContext);
				win32State.video.opengl.renderingContext = nullptr;
			}
		}
	#	endif // defined(FPL_ENABLE_VIDEO_OPENGL)

	#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
		fpl_internal void Win32ReleaseVideoSoftwareContext_Internal(Win32State_Internal &win32State) {
			Win32VideoSoftwareState_Internal &software = win32State.video.software;

			FPL_ASSERT(software.deviceBitmap != nullptr);
			FPL_WIN32_DELETE_OBJECT(software.deviceBitmap);

			FPL_ASSERT(software.context.pixels != nullptr);
			memory::MemoryAlignedFree(software.context.pixels);

			software = {};
		}

		fpl_internal bool Win32CreateVideoSoftware_Internal(Win32State_Internal &win32State, const uint32_t width, const uint32_t height) {
			Win32VideoSoftwareState_Internal &software = win32State.video.software;

			// Allocate memory/fill context
			software = {};
			software.context.width = width;
			software.context.height = height;
			software.context.stride = software.context.width * sizeof(uint32_t);
			size_t size = software.context.stride * software.context.height;
			software.context.pixels = (uint32_t *)memory::MemoryAlignedAllocate(size, 16);

			// Clear to black by default
			// @NOTE(final): Its a x,y by intention, so i can test if top-down is working.
			uint32_t *p = software.context.pixels;
			for (uint32_t y = 0; y < software.context.height; ++y) {
				uint32_t color = 0xFF000000;
				for (uint32_t x = 0; x < software.context.width; ++x) {
					*p++ = color;
				}
			}

			// Create DIB section from bitmapinfo
			software.bitmapInfo = {};
			software.bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			software.bitmapInfo.bmiHeader.biWidth = (LONG)software.context.width;
			software.bitmapInfo.bmiHeader.biHeight = (LONG)software.context.height;
			software.bitmapInfo.bmiHeader.biBitCount = 32;
			software.bitmapInfo.bmiHeader.biCompression = BI_RGB;
			software.bitmapInfo.bmiHeader.biPlanes = 1;
			software.bitmapInfo.bmiHeader.biSizeImage = (DWORD)size;

			software.deviceBitmap = FPL_WIN32_CREATE_DIBSECTION(win32State.window.deviceContext, &software.bitmapInfo, DIB_RGB_COLORS, (void **)software.context.pixels, nullptr, 0);

			return true;
		}
	#	endif // defined(FPL_ENABLE_VIDEO_SOFTWARE)

		fpl_api VideoBackBuffer *GetVideoBackBuffer() {
			VideoBackBuffer *result = nullptr;
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (globalWin32State_Internal->videoDriverType == VideoDriverType::Software) {
			#if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				Win32VideoSoftwareState_Internal software = globalWin32State_Internal->video.software;
				result = &software.context;
			#endif
			}
			return(result);
		}

		fpl_api bool ResizeVideoBackBuffer(const uint32_t width, const uint32_t height) {
			bool result = false;
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (globalWin32State_Internal->videoDriverType == VideoDriverType::Software) {
			#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				Win32ReleaseVideoSoftwareContext_Internal(*globalWin32State_Internal);
				result = Win32CreateVideoSoftware_Internal(*globalWin32State_Internal, width, height);
			#	endif
			}
			return (result);
		}
	}
#	endif

#	if defined(FPL_ENABLE_WINDOW)
	//
	// Win32 Public Window
	//
	namespace window {
		fpl_api void WindowFlip() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (globalWin32State_Internal->videoDriverType == VideoDriverType::OpenGL) {
				FPL_WIN32_SWAP_BUFFERS(globalWin32State_Internal->window.deviceContext);
			} else if (globalWin32State_Internal->videoDriverType == VideoDriverType::Software) {
				FPL_WIN32_POST_MESSAGE(globalWin32State_Internal->window.windowHandle, WM_PAINT, 0, 0);
			}
		}

		constexpr uint32_t MAX_EVENT_COUNT_INTERNAL = 32768;
		struct EventQueue_Internal {
			Event events[MAX_EVENT_COUNT_INTERNAL];
			volatile uint32_t pollIndex;
			volatile uint32_t pushCount;
		};
		fpl_globalvar EventQueue_Internal *globalEventQueue_Internal = nullptr;

		fpl_internal void PushEvent_Internal(const Event &event) {
			EventQueue_Internal *eventQueue = globalEventQueue_Internal;
			FPL_ASSERT(eventQueue != nullptr);
			if (eventQueue->pushCount < MAX_EVENT_COUNT_INTERNAL) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pushCount, 1);
				FPL_ASSERT(eventIndex < MAX_EVENT_COUNT_INTERNAL);
				eventQueue->events[eventIndex] = event;
			}
		}

		struct Win32WindowStyle_Internal {
			DWORD style;
			DWORD exStyle;
		};

		constexpr DWORD Win32ResizeableWindowStyle = WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
		constexpr DWORD Win32ResizeableWindowExtendedStyle = WS_EX_LEFT;

		constexpr DWORD Win32NonResizableWindowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
		constexpr DWORD Win32NonResizableWindowExtendedStyle = WS_EX_LEFT;

		constexpr DWORD Win32FullscreenWindowStyle = WS_POPUP | WS_VISIBLE;
		constexpr DWORD Win32FullscreenWindowExtendedStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

		fpl_internal bool Win32LeaveFullscreen_Internal() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			WindowSettings &settings = globalWin32State_Internal->currentSettings.window;
			FPL_ASSERT(settings.isFullscreen);
			HWND handle = globalWin32State_Internal->window.windowHandle;

			if (settings.isResizable) {
				FPL_WIN32_SET_WINDOW_LONG_PTR(handle, GWL_STYLE, Win32ResizeableWindowStyle);
				FPL_WIN32_SET_WINDOW_LONG_PTR(handle, GWL_EXSTYLE, Win32ResizeableWindowExtendedStyle);
			} else {
				FPL_WIN32_SET_WINDOW_LONG_PTR(handle, GWL_STYLE, Win32NonResizableWindowStyle);
				FPL_WIN32_SET_WINDOW_LONG_PTR(handle, GWL_EXSTYLE, Win32NonResizableWindowExtendedStyle);
			}

			FPL_WIN32_SET_WINDOW_POS(handle, HWND_NOTOPMOST, 0, 0, globalWin32State_Internal->window.lastWindowWidth, globalWin32State_Internal->window.lastWindowHeight, SWP_SHOWWINDOW | SWP_NOMOVE);
			FPL_WIN32_SET_WINDOW_PLACEMENT(handle, &globalWin32State_Internal->window.lastWindowPlacement);
			bool result = (FPL_WIN32_CHANGE_DISPLAY_SETTINGS_A(nullptr, CDS_RESET) == DISP_CHANGE_SUCCESSFUL);
			FPL_WIN32_SHOW_WINDOW(handle, SW_RESTORE);

			settings.isFullscreen = false;
			globalWin32State_Internal->window.lastWindowPlacement = {};
			globalWin32State_Internal->window.lastWindowWidth = globalWin32State_Internal->window.lastWindowHeight = 0;

			return(result);
		}

		fpl_internal bool Win32EnterFullscreen_Internal(const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate, const uint32_t colorBits) {
			// @TODO(final): Support for multi-monitor!
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			WindowSettings &settings = globalWin32State_Internal->currentSettings.window;
			FPL_ASSERT(!settings.isFullscreen);

			HWND windowHandle = globalWin32State_Internal->window.windowHandle;
			HDC deviceContext = globalWin32State_Internal->window.deviceContext;

			DWORD useRefreshRate = refreshRate;
			if (!useRefreshRate) {
				useRefreshRate = FPL_WIN32_GET_DEVICE_CAPS(deviceContext, VREFRESH);
			}

			DWORD useColourBits = colorBits;
			if (!useColourBits) {
				useColourBits = FPL_WIN32_GET_DEVICE_CAPS(deviceContext, BITSPIXEL);
			}

			DWORD useFullscreenWidth = fullscreenWidth;
			DWORD useFullscreenHeight = fullscreenHeight;
			if (!useFullscreenWidth || !useFullscreenHeight) {
				useFullscreenWidth = FPL_WIN32_GET_DEVICE_CAPS(deviceContext, HORZRES);
				useFullscreenHeight = FPL_WIN32_GET_DEVICE_CAPS(deviceContext, VERTRES);
			}

			FPL_WIN32_SET_WINDOW_LONG(windowHandle, GWL_STYLE, Win32FullscreenWindowStyle);
			FPL_WIN32_SET_WINDOW_LONG(windowHandle, GWL_EXSTYLE, Win32FullscreenWindowExtendedStyle);

			FPL_WIN32_SET_WINDOW_POS(windowHandle, HWND_TOPMOST, 0, 0, useFullscreenWidth, useFullscreenHeight, SWP_SHOWWINDOW);

			DEVMODEA fullscreenSettings = {};
			FPL_WIN32_ENUM_DISPLAY_SETTINGS_A(nullptr, 0, &fullscreenSettings);
			fullscreenSettings.dmPelsWidth = useFullscreenWidth;
			fullscreenSettings.dmPelsHeight = useFullscreenHeight;
			fullscreenSettings.dmBitsPerPel = useColourBits;
			fullscreenSettings.dmDisplayFrequency = useRefreshRate;
			fullscreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
			bool result = (FPL_WIN32_CHANGE_DISPLAY_SETTINGS_A(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);

			FPL_WIN32_SHOW_WINDOW(windowHandle, SW_MAXIMIZE);

			settings.isFullscreen = result;

			return(result);
		}

		fpl_internal float Win32XInputProcessStickValue(SHORT value, SHORT deadZoneThreshold) {
			float result = 0;
			if (value < -deadZoneThreshold) {
				result = (float)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
			} else if (value > deadZoneThreshold) {
				result = (float)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
			}
			return(result);
		}

		fpl_internal void Win32PollControllers_Internal(Win32State_Internal *win32State) {
			FPL_ASSERT(globalWin32Functions.xinputGetState != nullptr);
			constexpr DWORD MaxControllerCount = XUSER_MAX_COUNT;
			for (DWORD controllerIndex = 0; controllerIndex < MaxControllerCount; ++controllerIndex) {
				XINPUT_STATE controllerState = {};
				if (globalWin32Functions.xinputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
					if (!win32State->xinput.isConnected[controllerIndex]) {
						// Connected
						win32State->xinput.isConnected[controllerIndex] = true;

						Event ev = {};
						ev.type = EventType::Gamepad;
						ev.gamepad.type = GamepadEventType::Connected;
						ev.gamepad.deviceIndex = controllerIndex;
						PushEvent_Internal(ev);
					} else {
						// State changed
						Event ev = {};
						ev.type = EventType::Gamepad;
						ev.gamepad.type = GamepadEventType::StateChanged;
						ev.gamepad.deviceIndex = controllerIndex;

						XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

						// Analog sticks
						ev.gamepad.state.leftStickX = Win32XInputProcessStickValue(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
						ev.gamepad.state.leftStickY = Win32XInputProcessStickValue(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
						ev.gamepad.state.rightStickX = Win32XInputProcessStickValue(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
						ev.gamepad.state.rightStickY = Win32XInputProcessStickValue(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

						// Triggers
						ev.gamepad.state.leftTrigger = (float)pad->bLeftTrigger / 255.0f;
						ev.gamepad.state.rightTrigger = (float)pad->bRightTrigger / 255.0f;

						// Digital pad buttons
						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
							ev.gamepad.state.dpadUp = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
							ev.gamepad.state.dpadDown = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
							ev.gamepad.state.dpadLeft = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
							ev.gamepad.state.dpadRight = { true };

						// Action buttons
						if (pad->wButtons & XINPUT_GAMEPAD_A)
							ev.gamepad.state.actionA = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_B)
							ev.gamepad.state.actionB = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_X)
							ev.gamepad.state.actionX = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_Y)
							ev.gamepad.state.actionY = { true };

						// Center buttons
						if (pad->wButtons & XINPUT_GAMEPAD_START)
							ev.gamepad.state.start = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_BACK)
							ev.gamepad.state.back = { true };

						// Shoulder buttons
						if (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
							ev.gamepad.state.leftShoulder = { true };
						if (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
							ev.gamepad.state.rightShoulder = { true };

						PushEvent_Internal(ev);
					}
				} else {
					if (win32State->xinput.isConnected[controllerIndex]) {
						// Disonnected
						win32State->xinput.isConnected[controllerIndex] = false;

						Event ev = {};
						ev.type = EventType::Gamepad;
						ev.gamepad.type = GamepadEventType::Disconnected;
						ev.gamepad.deviceIndex = controllerIndex;
						PushEvent_Internal(ev);
					}
				}
			}
		}

		fpl_internal void Win32PushMouseEvent_Internal(const MouseEventType &mouseEventType, const MouseButtonType mouseButton, const LPARAM lParam, const WPARAM wParam) {
			Event newEvent = {};
			newEvent.type = EventType::Mouse;
			newEvent.mouse.type = mouseEventType;
			newEvent.mouse.mouseX = GET_X_LPARAM(lParam);
			newEvent.mouse.mouseY = GET_Y_LPARAM(lParam);
			newEvent.mouse.mouseButton = mouseButton;
			if (mouseEventType == MouseEventType::Wheel) {
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				newEvent.mouse.wheelDelta = (zDelta / (float)WHEEL_DELTA);
			}
			PushEvent_Internal(newEvent);
		}

		fpl_internal Key Win32MapVirtualKey_Internal(const uint64_t keyCode) {
			switch (keyCode) {
				case VK_BACK:
					return Key::Key_Backspace;
				case VK_TAB:
					return Key::Key_Tab;

				case VK_CLEAR:
					return Key::Key_Clear;
				case VK_RETURN:
					return Key::Key_Enter;

				case VK_SHIFT:
					return Key::Key_Shift;
				case VK_CONTROL:
					return Key::Key_Control;
				case VK_MENU:
					return Key::Key_Alt;
				case VK_PAUSE:
					return Key::Key_Pause;
				case VK_CAPITAL:
					return Key::Key_CapsLock;

				case VK_ESCAPE:
					return Key::Key_Escape;
				case VK_SPACE:
					return Key::Key_Space;
				case VK_PRIOR:
					return Key::Key_PageUp;
				case VK_NEXT:
					return Key::Key_PageDown;
				case VK_END:
					return Key::Key_End;
				case VK_HOME:
					return Key::Key_Home;
				case VK_LEFT:
					return Key::Key_Left;
				case VK_UP:
					return Key::Key_Up;
				case VK_RIGHT:
					return Key::Key_Right;
				case VK_DOWN:
					return Key::Key_Down;
				case VK_SELECT:
					return Key::Key_Select;
				case VK_PRINT:
					return Key::Key_Print;
				case VK_EXECUTE:
					return Key::Key_Execute;
				case VK_SNAPSHOT:
					return Key::Key_Snapshot;
				case VK_INSERT:
					return Key::Key_Insert;
				case VK_DELETE:
					return Key::Key_Delete;
				case VK_HELP:
					return Key::Key_Help;

				case 0x30:
					return Key::Key_0;
				case 0x31:
					return Key::Key_1;
				case 0x32:
					return Key::Key_2;
				case 0x33:
					return Key::Key_3;
				case 0x34:
					return Key::Key_4;
				case 0x35:
					return Key::Key_5;
				case 0x36:
					return Key::Key_6;
				case 0x37:
					return Key::Key_7;
				case 0x38:
					return Key::Key_8;
				case 0x39:
					return Key::Key_9;

				case 0x41:
					return Key::Key_A;
				case 0x42:
					return Key::Key_B;
				case 0x43:
					return Key::Key_C;
				case 0x44:
					return Key::Key_D;
				case 0x45:
					return Key::Key_E;
				case 0x46:
					return Key::Key_F;
				case 0x47:
					return Key::Key_G;
				case 0x48:
					return Key::Key_H;
				case 0x49:
					return Key::Key_I;
				case 0x4A:
					return Key::Key_J;
				case 0x4B:
					return Key::Key_K;
				case 0x4C:
					return Key::Key_L;
				case 0x4D:
					return Key::Key_M;
				case 0x4E:
					return Key::Key_N;
				case 0x4F:
					return Key::Key_O;
				case 0x50:
					return Key::Key_P;
				case 0x51:
					return Key::Key_Q;
				case 0x52:
					return Key::Key_R;
				case 0x53:
					return Key::Key_S;
				case 0x54:
					return Key::Key_T;
				case 0x55:
					return Key::Key_U;
				case 0x56:
					return Key::Key_V;
				case 0x57:
					return Key::Key_W;
				case 0x58:
					return Key::Key_X;
				case 0x59:
					return Key::Key_Y;
				case 0x5A:
					return Key::Key_Z;

				case VK_LWIN:
					return Key::Key_LeftWin;
				case VK_RWIN:
					return Key::Key_RightWin;
				case VK_APPS:
					return Key::Key_Apps;

				case VK_SLEEP:
					return Key::Key_Sleep;
				case VK_NUMPAD0:
					return Key::Key_NumPad0;
				case VK_NUMPAD1:
					return Key::Key_NumPad1;
				case VK_NUMPAD2:
					return Key::Key_NumPad2;
				case VK_NUMPAD3:
					return Key::Key_NumPad3;
				case VK_NUMPAD4:
					return Key::Key_NumPad4;
				case VK_NUMPAD5:
					return Key::Key_NumPad5;
				case VK_NUMPAD6:
					return Key::Key_NumPad6;
				case VK_NUMPAD7:
					return Key::Key_NumPad7;
				case VK_NUMPAD8:
					return Key::Key_NumPad8;
				case VK_NUMPAD9:
					return Key::Key_NumPad9;
				case VK_MULTIPLY:
					return Key::Key_Multiply;
				case VK_ADD:
					return Key::Key_Add;
				case VK_SEPARATOR:
					return Key::Key_Separator;
				case VK_SUBTRACT:
					return Key::Key_Substract;
				case VK_DECIMAL:
					return Key::Key_Decimal;
				case VK_DIVIDE:
					return Key::Key_Divide;
				case VK_F1:
					return Key::Key_F1;
				case VK_F2:
					return Key::Key_F2;
				case VK_F3:
					return Key::Key_F3;
				case VK_F4:
					return Key::Key_F4;
				case VK_F5:
					return Key::Key_F5;
				case VK_F6:
					return Key::Key_F6;
				case VK_F7:
					return Key::Key_F7;
				case VK_F8:
					return Key::Key_F8;
				case VK_F9:
					return Key::Key_F9;
				case VK_F10:
					return Key::Key_F10;
				case VK_F11:
					return Key::Key_F11;
				case VK_F12:
					return Key::Key_F12;
				case VK_F13:
					return Key::Key_F13;
				case VK_F14:
					return Key::Key_F14;
				case VK_F15:
					return Key::Key_F15;
				case VK_F16:
					return Key::Key_F16;
				case VK_F17:
					return Key::Key_F17;
				case VK_F18:
					return Key::Key_F18;
				case VK_F19:
					return Key::Key_F19;
				case VK_F20:
					return Key::Key_F20;
				case VK_F21:
					return Key::Key_F21;
				case VK_F22:
					return Key::Key_F22;
				case VK_F23:
					return Key::Key_F23;
				case VK_F24:
					return Key::Key_F24;

				case VK_LSHIFT:
					return Key::Key_LeftShift;
				case VK_RSHIFT:
					return Key::Key_RightShift;
				case VK_LCONTROL:
					return Key::Key_LeftControl;
				case VK_RCONTROL:
					return Key::Key_RightControl;
				case VK_LMENU:
					return Key::Key_LeftAlt;
				case VK_RMENU:
					return Key::Key_RightAlt;

				default:
					return Key::Key_None;
			}
		}

		fpl_internal void Win32PushKeyboardEvent_Internal(const KeyboardEventType keyboardEventType, const uint64_t keyCode, const KeyboardModifierFlags modifiers, const bool isDown) {
			Event newEvent = {};
			newEvent.type = EventType::Keyboard;
			newEvent.keyboard.keyCode = keyCode;
			newEvent.keyboard.mappedKey = Win32MapVirtualKey_Internal(keyCode);
			newEvent.keyboard.type = keyboardEventType;
			newEvent.keyboard.modifiers = modifiers;
			PushEvent_Internal(newEvent);
		}

		fpl_internal bool Win32IsKeyDown_Internal(const uint64_t keyCode) {
			bool result = (FPL_WIN32_GET_ASYNC_KEY_STATE((int)keyCode) & 0x8000) > 0;
			return(result);
		}

		fpl_api WindowSize GetWindowArea() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			WindowSize result = {};
			RECT windowRect;
			if (FPL_WIN32_GET_CLIENT_RECT(globalWin32State_Internal->window.windowHandle, &windowRect)) {
				result.width = windowRect.right - windowRect.left;
				result.height = windowRect.bottom - windowRect.top;
			}
			return(result);
		}

		fpl_api void SetWindowArea(const uint32_t width, const uint32_t height) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			RECT clientRect, windowRect;
			if (FPL_WIN32_GET_CLIENT_RECT(globalWin32State_Internal->window.windowHandle, &clientRect) && FPL_WIN32_GET_WINDOW_RECT(globalWin32State_Internal->window.windowHandle, &windowRect)) {
				int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
				int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
				int newWidth = width + borderWidth;
				int newHeight = height + borderHeight;
				FPL_WIN32_SET_WINDOW_POS(globalWin32State_Internal->window.windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}
		}

		fpl_api bool IsWindowResizable() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			DWORD style = FPL_WIN32_GET_WINDOW_LONG(globalWin32State_Internal->window.windowHandle, GWL_STYLE);
			bool result = (style & WS_THICKFRAME) > 0;
			return(result);
		}

		fpl_api void SetWindowResizeable(const bool value) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (!globalWin32State_Internal->currentSettings.window.isFullscreen) {
				DWORD style;
				DWORD exStyle;
				if (value) {
					style = Win32ResizeableWindowStyle;
					exStyle = Win32ResizeableWindowExtendedStyle;
				} else {
					style = Win32NonResizableWindowStyle;
					exStyle = Win32NonResizableWindowExtendedStyle;
				}
				FPL_WIN32_SET_WINDOW_LONG(globalWin32State_Internal->window.windowHandle, GWL_STYLE, style);
				FPL_WIN32_SET_WINDOW_LONG(globalWin32State_Internal->window.windowHandle, GWL_EXSTYLE, exStyle);
				globalWin32State_Internal->currentSettings.window.isResizable = value;
			}
		}

		fpl_api bool IsWindowFullscreen() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			HWND windowHandle = globalWin32State_Internal->window.windowHandle;
			DWORD style = FPL_WIN32_GET_WINDOW_LONG(windowHandle, GWL_STYLE);
			bool result = (style & Win32FullscreenWindowStyle) > 0;
			return(result);
		}

		fpl_api void SetWindowFullscreen(const bool value, const uint32_t fullscreenWidth, const uint32_t fullscreenHeight, const uint32_t refreshRate) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (value) {
				WindowSettings& windowSettings = globalWin32State_Internal->currentSettings.window;

				// Save window placement and size
				HWND windowHandle = globalWin32State_Internal->window.windowHandle;
				globalWin32State_Internal->window.lastWindowPlacement = {};
				FPL_WIN32_GET_WINDOW_PLACEMENT(windowHandle, &globalWin32State_Internal->window.lastWindowPlacement);

				RECT windowRect = {};
				FPL_WIN32_GET_WINDOW_RECT(windowHandle, &windowRect);
				globalWin32State_Internal->window.lastWindowWidth = windowRect.right - windowRect.left;
				globalWin32State_Internal->window.lastWindowHeight = windowRect.bottom - windowRect.top;

				// Enter fullscreen mode or fallback to window mode
				windowSettings.isFullscreen = Win32EnterFullscreen_Internal(fullscreenWidth, fullscreenHeight, refreshRate, 0);
				if (!windowSettings.isFullscreen) {
					Win32LeaveFullscreen_Internal();
				}
			} else {
				Win32LeaveFullscreen_Internal();
			}
		}

		fpl_api WindowPosition GetWindowPosition() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			WindowPosition result = {};
			WINDOWPLACEMENT placement = {};
			placement.length = sizeof(WINDOWPLACEMENT);
			if (FPL_WIN32_GET_WINDOW_PLACEMENT(globalWin32State_Internal->window.windowHandle, &placement) == TRUE) {
				switch (placement.showCmd) {
					case SW_MAXIMIZE:
					{
						result.left = placement.ptMaxPosition.x;
						result.top = placement.ptMaxPosition.y;
					} break;
					case SW_MINIMIZE:
					{
						result.left = placement.ptMinPosition.x;
						result.top = placement.ptMinPosition.y;
					} break;
					default:
					{
						result.left = placement.rcNormalPosition.left;
						result.top = placement.rcNormalPosition.top;
					} break;
				}
			}
			return(result);
		}

		fpl_api void SetWindowTitle(const char *title) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			HWND handle = globalWin32State_Internal->window.windowHandle;
			// @FIXME(final): This is not correct to set a ansi text, in unicode we have to use the Wide version!
			FPL_WIN32_SET_WINDOW_TEXT_A(handle, title);
		}

		fpl_api void SetWindowPosition(const int32_t left, const int32_t top) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			WINDOWPLACEMENT placement = {};
			placement.length = sizeof(WINDOWPLACEMENT);
			RECT windowRect;
			if (FPL_WIN32_GET_WINDOW_PLACEMENT(globalWin32State_Internal->window.windowHandle, &placement) && FPL_WIN32_GET_WINDOW_RECT(globalWin32State_Internal->window.windowHandle, &windowRect)) {
				switch (placement.showCmd) {
					case SW_NORMAL:
					case SW_SHOW:
					{
						placement.rcNormalPosition.left = left;
						placement.rcNormalPosition.top = top;
						placement.rcNormalPosition.right = placement.rcNormalPosition.left + (windowRect.right - windowRect.left);
						placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + (windowRect.bottom - windowRect.top);
						FPL_WIN32_SET_WINDOW_PLACEMENT(globalWin32State_Internal->window.windowHandle, &placement);
					} break;
				}
			}
		}

		fpl_api void SetWindowCursorEnabled(const bool value) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			globalWin32State_Internal->window.isCursorActive = value;
		}

		fpl_api bool WindowUpdate() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			bool result = false;

			// Poll gamepad controller states
			Win32PollControllers_Internal(globalWin32State_Internal);

			// Poll window events
			if (globalWin32State_Internal->window.windowHandle != 0) {
				MSG msg;
				while (FPL_WIN32_PEEK_MESSAGE(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
					FPL_WIN32_TRANSLATE_MESSAGE(&msg);
					FPL_WIN32_DISPATCH_MESSAGE(&msg);
				}
				result = globalWin32State_Internal->window.isRunning;
			}

			return(result);
		}

		fpl_api bool IsWindowRunning() {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			bool result = globalWin32State_Internal->window.isRunning;
			return(result);
		}

		fpl_api bool PollWindowEvent(Event &ev) {
			EventQueue_Internal *eventQueue = globalEventQueue_Internal;
			FPL_ASSERT(eventQueue != nullptr);
			bool result = false;
			if (eventQueue->pushCount > 0 && (eventQueue->pollIndex < eventQueue->pushCount)) {
				uint32_t eventIndex = atomics::AtomicAddU32(&eventQueue->pollIndex, 1);
				ev = eventQueue->events[eventIndex];
				result = true;
			} else if (globalEventQueue_Internal->pushCount > 0) {
				atomics::AtomicExchangeU32(&eventQueue->pollIndex, 0);
				atomics::AtomicExchangeU32(&eventQueue->pushCount, 0);
			}
			return result;
		}

		fpl_api char *GetClipboardAnsiText(char *dest, const uint32_t maxDestLen) {
			char *result = nullptr;
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (OpenClipboard(globalWin32State_Internal->window.windowHandle)) {
				if (IsClipboardFormatAvailable(CF_TEXT)) {
					HGLOBAL dataHandle = GetClipboardData(CF_TEXT);
					if (dataHandle != nullptr) {
						const char *stringValue = (const char *)GlobalLock(dataHandle);
						result = strings::CopyAnsiString(stringValue, dest, maxDestLen);
						GlobalUnlock(dataHandle);
					}
				}
				CloseClipboard();
			}
			return(result);
		}

		fpl_api wchar_t *GetClipboardWideText(wchar_t *dest, const uint32_t maxDestLen) {
			wchar_t *result = nullptr;
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (OpenClipboard(globalWin32State_Internal->window.windowHandle)) {
				if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
					HGLOBAL dataHandle = GetClipboardData(CF_UNICODETEXT);
					if (dataHandle != nullptr) {
						const wchar_t *stringValue = (const wchar_t *)GlobalLock(dataHandle);
						result = strings::CopyWideString(stringValue, dest, maxDestLen);
						GlobalUnlock(dataHandle);
					}
				}
				CloseClipboard();
			}
			return(result);
		}

		fpl_api bool SetClipboardText(const char *ansiSource) {
			bool result = false;
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (OpenClipboard(globalWin32State_Internal->window.windowHandle)) {
				const uint32_t ansiLen = strings::GetAnsiStringLength(ansiSource);
				const uint32_t ansiBufferLen = ansiLen + 1;
				HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)ansiBufferLen * sizeof(char));
				if (handle != nullptr) {
					char *target = (char*)GlobalLock(handle);
					strings::CopyAnsiString(ansiSource, ansiLen, target, ansiBufferLen);
					GlobalUnlock(handle);
					EmptyClipboard();
					SetClipboardData(CF_TEXT, handle);
					result = true;
				}
				CloseClipboard();
			}
			return(result);
		}

		fpl_api bool SetClipboardText(const wchar_t *wideSource) {
			bool result = false;
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			if (OpenClipboard(globalWin32State_Internal->window.windowHandle)) {
				const uint32_t wideLen = strings::GetWideStringLength(wideSource);
				const uint32_t wideBufferLen = wideLen + 1;
				HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wideBufferLen * sizeof(wchar_t));
				if (handle != nullptr) {
					wchar_t *wideTarget = (wchar_t*)GlobalLock(handle);
					strings::CopyWideString(wideSource, wideLen, wideTarget, wideBufferLen);
					GlobalUnlock(handle);
					EmptyClipboard();
					SetClipboardData(CF_UNICODETEXT, handle);
					result = true;
				}
				CloseClipboard();
			}
			return(result);
		}

		LRESULT CALLBACK Win32MessageProc_Internal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			FPL_ASSERT(globalWin32State_Internal != nullptr);
			Win32State_Internal &win32State = *globalWin32State_Internal;
			if (!win32State.window.windowHandle) {
				return FPL_WIN32_DEF_WINDOW_PROC(hwnd, msg, wParam, lParam);
			}

			LRESULT result = 0;
			switch (msg) {
				case WM_DESTROY:
				case WM_CLOSE:
				{
					win32State.window.isRunning = false;
				} break;

				case WM_SIZE:
				{
					if (win32State.videoDriverType == VideoDriverType::Software) {
					#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
						if (win32State.initSettings.video.isAutoSize) {
							uint32_t w = LOWORD(lParam);
							uint32_t h = HIWORD(lParam);
							if ((w != win32State.video.software.context.width) ||
								(h != win32State.video.software.context.height)) {
								video::Win32ReleaseVideoSoftwareContext_Internal(win32State);
								video::Win32CreateVideoSoftware_Internal(win32State, w, h);
							}
						}
					#	endif
					}

					Event newEvent = {};
					newEvent.type = EventType::Window;
					newEvent.window.type = WindowEventType::Resized;
					newEvent.window.width = LOWORD(lParam);
					newEvent.window.height = HIWORD(lParam);
					PushEvent_Internal(newEvent);
				} break;

				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				case WM_KEYDOWN:
				case WM_KEYUP:
				{
					uint64_t keyCode = wParam;
					bool wasDown = ((lParam & (1 << 30)) != 0);
					bool isDown = ((lParam & (1 << 31)) == 0);

					bool altKeyWasDown = Win32IsKeyDown_Internal(VK_MENU);
					bool shiftKeyWasDown = Win32IsKeyDown_Internal(VK_LSHIFT);
					bool ctrlKeyWasDown = Win32IsKeyDown_Internal(VK_LCONTROL);
					bool superKeyWasDown = Win32IsKeyDown_Internal(VK_LMENU);

					KeyboardEventType keyEventType = isDown ? KeyboardEventType::KeyDown : KeyboardEventType::KeyUp;
					KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
					if (altKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Alt;
					}
					if (shiftKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Shift;
					}
					if (ctrlKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Ctrl;
					}
					if (superKeyWasDown) {
						modifiers |= KeyboardModifierFlags::Super;
					}
					Win32PushKeyboardEvent_Internal(keyEventType, keyCode, modifiers, isDown);

					if (wasDown != isDown) {
						if (isDown) {
							if (keyCode == VK_F4 && altKeyWasDown) {
								win32State.window.isRunning = 0;
							}
						}
					}
				} break;

				case WM_CHAR:
				{
					uint64_t keyCode = wParam;
					KeyboardModifierFlags modifiers = KeyboardModifierFlags::None;
					Win32PushKeyboardEvent_Internal(KeyboardEventType::Char, keyCode, modifiers, 0);
				} break;

				case WM_ACTIVATE:
				{
					Event newEvent = {};
					newEvent.type = EventType::Window;
					if (wParam == WA_INACTIVE) {
						newEvent.window.type = WindowEventType::LostFocus;
					} else {
						newEvent.window.type = WindowEventType::GotFocus;
					}
					PushEvent_Internal(newEvent);
				} break;

				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				{
					MouseEventType mouseEventType;
					if (msg == WM_LBUTTONDOWN) {
						mouseEventType = MouseEventType::ButtonDown;
					} else {
						mouseEventType = MouseEventType::ButtonUp;
					}
					Win32PushMouseEvent_Internal(mouseEventType, MouseButtonType::Left, lParam, wParam);
				} break;
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
				{
					MouseEventType mouseEventType;
					if (msg == WM_RBUTTONDOWN) {
						mouseEventType = MouseEventType::ButtonDown;
					} else {
						mouseEventType = MouseEventType::ButtonUp;
					}
					Win32PushMouseEvent_Internal(mouseEventType, MouseButtonType::Right, lParam, wParam);
				} break;
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
				{
					MouseEventType mouseEventType;
					if (msg == WM_MBUTTONDOWN) {
						mouseEventType = MouseEventType::ButtonDown;
					} else {
						mouseEventType = MouseEventType::ButtonUp;
					}
					Win32PushMouseEvent_Internal(mouseEventType, MouseButtonType::Middle, lParam, wParam);
				} break;
				case WM_MOUSEMOVE:
				{
					Win32PushMouseEvent_Internal(MouseEventType::Move, MouseButtonType::None, lParam, wParam);
				} break;
				case WM_MOUSEWHEEL:
				{
					Win32PushMouseEvent_Internal(MouseEventType::Wheel, MouseButtonType::None, lParam, wParam);
				} break;

				case WM_SETCURSOR:
				{
					// @TODO(final): This is not right to assume default cursor always, because the size cursor does not work this way!
					if (win32State.window.isCursorActive) {
						HCURSOR cursor = GetCursor();
						FPL_WIN32_SET_CURSOR(cursor);
					} else {
						FPL_WIN32_SET_CURSOR(nullptr);
						return 1;
					}
				} break;

				case WM_ERASEBKGND:
				{
					// @TODO(final): No erase the background when opengl is active!
				} break;

				case WM_PAINT:
				{
					// @TODO(final): Full ignore WM_PAINT when opengl is active
					if (win32State.videoDriverType == VideoDriverType::Software) {
						Win32VideoSoftwareState_Internal &software = win32State.video.software;
						PAINTSTRUCT ps = {};
						HDC dc = FPL_WIN32_BEGIN_PAINT(hwnd, &ps);
						uint32_t targetWidth = ps.rcPaint.right - ps.rcPaint.left;
						uint32_t targetHeight = ps.rcPaint.bottom - ps.rcPaint.top;
						uint32_t sourceWidth = software.context.width;
						uint32_t sourceHeight = software.context.height;
						FPL_WIN32_STRETCH_DIBITS(dc, 0, 0, targetWidth, targetHeight, 0, 0, sourceWidth, sourceHeight, software.context.pixels, &software.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
						FPL_WIN32_END_PAINT(hwnd, &ps);
						return 1;
					}
				} break;

				default:
					break;
			}
			result = FPL_WIN32_DEF_WINDOW_PROC(hwnd, msg, wParam, lParam);
			return (result);
		}
	}
#	endif // defined(FPL_ENABLE_WINDOW)

#	if defined(FPL_ENABLE_WINDOW)
	namespace window {
		fpl_internal bool Win32InitWindow_Internal(Win32State_Internal &win32State, const Settings &initSettings) {
			// Register window class
			FPL_WIN32_WNDCLASSEX windowClass = {};
			windowClass.cbSize = sizeof(FPL_WIN32_WNDCLASSEX);
			windowClass.hInstance = win32State.appInstance;
			windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			windowClass.cbSize = sizeof(windowClass);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
			windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			windowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
			windowClass.lpszClassName = FPL_WIN32_CLASSNAME;
			windowClass.lpfnWndProc = Win32MessageProc_Internal;
			if (initSettings.video.driverType == VideoDriverType::OpenGL) {
				windowClass.style |= CS_OWNDC;
			}
			FPL_WIN32_STRINGCOPY(windowClass.lpszClassName, FPL_WIN32_GETSTRINGLENGTH(windowClass.lpszClassName), win32State.window.windowClass, FPL_ARRAYCOUNT(win32State.window.windowClass));
			if (FPL_WIN32_REGISTER_CLASS_EX(&windowClass) == 0) {
				PushError_Internal("[Win32] Failed Registering Window Class '%s'", win32State.window.windowClass);
				return false;
			}

			// Allocate event queue
			size_t eventQueueMemorySize = sizeof(EventQueue_Internal);
			void *eventQueueMemory = memory::MemoryAlignedAllocate(eventQueueMemorySize, 16);
			if (eventQueueMemory == nullptr) {
				PushError_Internal("[Win32] Failed Allocating Event Queue Memory with size '%llu'!", eventQueueMemorySize);
				return false;
			}
			globalEventQueue_Internal = (EventQueue_Internal *)eventQueueMemory;

			// @TODO(final): Additional memory block for temporary buffers, so its not needed to pass in a destBuffer

			// Create window
			win32_char_internal windowTitleBuffer[1024];
			win32_char_internal *windowTitle = FPL_WIN32_UNNAMED_WINDOW;
			WindowSettings &currentSettings = win32State.currentSettings.window;
			currentSettings.isFullscreen = false;
			currentSettings.windowWidth = initSettings.window.windowWidth;
			currentSettings.windowHeight = initSettings.window.windowHeight;
			if (strings::GetAnsiStringLength(initSettings.window.windowTitle) > 0) {
				FPL_WIN32_ANSI_TO_STRING(initSettings.window.windowTitle, strings::GetAnsiStringLength(initSettings.window.windowTitle), windowTitleBuffer, FPL_ARRAYCOUNT(windowTitleBuffer));
				windowTitle = windowTitleBuffer;
				strings::CopyAnsiString(initSettings.window.windowTitle, strings::GetAnsiStringLength(initSettings.window.windowTitle), currentSettings.windowTitle, FPL_ARRAYCOUNT(currentSettings.windowTitle));
			}

			DWORD style;
			DWORD exStyle;
			if (initSettings.window.isResizable) {
				style = Win32ResizeableWindowStyle;
				exStyle = Win32ResizeableWindowExtendedStyle;
				currentSettings.isResizable = true;
			} else {
				style = Win32NonResizableWindowStyle;
				exStyle = Win32NonResizableWindowExtendedStyle;
				currentSettings.isResizable = false;
			}

			RECT windowRect;
			windowRect.left = 0;
			windowRect.top = 0;
			windowRect.right = initSettings.window.windowWidth;
			windowRect.bottom = initSettings.window.windowHeight;
			FPL_WIN32_ADJUST_WINDOW_RECT(&windowRect, style, false);

			int windowX = CW_USEDEFAULT;
			int windowY = CW_USEDEFAULT;
			int windowWidth = windowRect.right - windowRect.left;
			int windowHeight = windowRect.bottom - windowRect.top;

			win32State.window.windowHandle = FPL_WIN32_CREATE_WINDOW_EX(exStyle, windowClass.lpszClassName, windowTitle, style, windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, windowClass.hInstance, nullptr);
			if (win32State.window.windowHandle == nullptr) {
				PushError_Internal("[Win32] Failed creating window for class '%s' and position (%d x %d) with size (%d x %d)", win32State.window.windowClass, windowWidth, windowHeight, windowWidth, windowHeight);
				return false;
			}

			// Get device context so we can swap the back and front buffer
			win32State.window.deviceContext = FPL_WIN32_GET_DC(win32State.window.windowHandle);
			if (win32State.window.deviceContext == nullptr) {
				PushError_Internal("[Win32] Failed aquiring device context from window '%d'", win32State.window.windowHandle);
				return false;
			}

			// Enter fullscreen if needed
			if (initSettings.window.isFullscreen) {
				SetWindowFullscreen(true, initSettings.window.fullscreenWidth, initSettings.window.fullscreenHeight);
			}

			// Create opengl rendering context if required
			win32State.videoDriverType = VideoDriverType::None;
			switch (initSettings.video.driverType) {
				case VideoDriverType::OpenGL:
				{
				#	if defined(FPL_ENABLE_VIDEO_OPENGL)
					bool openglResult = video::Win32CreateVideoOpenGL_Internal(win32State, initSettings.video);
					if (!openglResult) {
						PushError_Internal("[Win32] Failed initializing OpenGL for window '%d'/'%s'", win32State.window.windowHandle, win32State.window.windowClass);
						return false;
					}
					win32State.videoDriverType = VideoDriverType::OpenGL;
				#	endif // defined(FPL_ENABLE_VIDEO_OPENGL)
				} break;

				case VideoDriverType::Software:
				{
				#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
					bool softwareResult = video::Win32CreateVideoSoftware_Internal(win32State, windowWidth, windowHeight);
					if (!softwareResult) {
						PushError_Internal("[Win32] Failed creating software rendering buffer for window '%d'/'%s'", win32State.window.windowHandle, win32State.window.windowClass);
						return false;
					}
					win32State.videoDriverType = VideoDriverType::Software;
				#	endif // defined(FPL_ENABLE_VIDEO_SOFTWARE)
				} break;
			}

			// Show window
			FPL_WIN32_SHOW_WINDOW(win32State.window.windowHandle, SW_SHOW);
			FPL_WIN32_UPDATE_WINDOW(win32State.window.windowHandle);

			// Cursor is visible at start
			win32State.window.defaultCursor = windowClass.hCursor;
			win32State.window.isCursorActive = true;
			win32State.window.isRunning = true;

			return true;
		}

		fpl_internal void Win32ReleaseWindow_Internal(Win32State_Internal &win32State) {
			if (win32State.window.deviceContext != nullptr) {
				FPL_WIN32_RELEASE_DC(win32State.window.windowHandle, win32State.window.deviceContext);
				win32State.window.deviceContext = nullptr;
			}

			if (win32State.window.windowHandle != nullptr) {
				FPL_WIN32_DESTROY_WINDOW(win32State.window.windowHandle);
				win32State.window.windowHandle = nullptr;
				FPL_WIN32_UNREGISTER_CLASS(win32State.window.windowClass, win32State.appInstance);
			}

			memory::MemoryAlignedFree(globalEventQueue_Internal);
			globalEventQueue_Internal = nullptr;
		}

		fpl_internal HMODULE Win32LoadXInput_Internal() {
			// Windows 8
			HMODULE result = FPL_WIN32_LOAD_LIBRARY("xinput1_4.dll");
			if (!result) {
				// Windows 7
				result = FPL_WIN32_LOAD_LIBRARY("xinput1_3.dll");
			}
			if (!result) {
				// Windows Generic
				result = FPL_WIN32_LOAD_LIBRARY("xinput9_1_0.dll");
			}
			if (result) {
				globalWin32Functions.xinputGetState = (xinput_get_state *)FPL_WIN32_GET_PROC_ADDRESS(result, "XInputGetState");
			}
			if (!globalWin32Functions.xinputGetState) {
				globalWin32Functions.xinputGetState = Win32XInputGetStateStub;
			}
			return(result);
		}

		fpl_internal void Win32UnloadXInput_Internal(HMODULE &module) {
			if (module) {
				FPL_WIN32_FREE_LIBRARY(module);
				module = nullptr;
			}
		}
	}
#endif

	fpl_api bool InitPlatform(const InitFlags initFlags, const Settings &initSettings) {
		if (fpl::globalWin32State_Internal != nullptr) {
			PushError_Internal("[Win32] Platform is already initialized!");
			return false;
		}

		// Allocate win32 state
		void *win32StateMemory = fpl::memory::MemoryAlignedAllocate(sizeof(fpl::Win32State_Internal), 16);
		FPL_ASSERT(win32StateMemory != nullptr);
		fpl::globalWin32State_Internal = (fpl::Win32State_Internal *)win32StateMemory;
		new(fpl::globalWin32State_Internal) fpl::Win32State_Internal();
		Win32State_Internal &win32State = *globalWin32State_Internal;
		win32State.appInstance = globalWin32AppInstance_Internal;
		win32State.initSettings = initSettings;
		win32State.initFlags = initFlags;
		win32State.currentSettings = initSettings;
		win32State.videoDriverType = VideoDriverType::None;

		// Allocate last error state
		void *lastErrorStateMemory = memory::MemoryAlignedAllocate(sizeof(ErrorState_Internal), 16);
		FPL_ASSERT(lastErrorStateMemory != nullptr);
		globalLastError_Internal = (ErrorState_Internal *)lastErrorStateMemory;

		// Expect it to be not initialized!
		FPL_ASSERT(!win32State.isInitialized);

		// Timing
		FPL_WIN32_QUERY_PERFORMANCE_FREQUENCY(&win32State.performanceFrequency);

		// Get main thread infos
		HANDLE mainThreadHandle = GetCurrentThread();
		DWORD mainThreadHandleId = GetCurrentThreadId();
		threading::ThreadContext *context = &threading::globalThreadState_Internal.mainThread;
		*context = {};
		context->id = mainThreadHandleId;
		context->internalHandle = (void *)mainThreadHandle;
		context->state = threading::ThreadState::Running;

	#if defined(FPL_ENABLE_WINDOW)
		// XInput
		win32State.xinput.xinputLibrary = window::Win32LoadXInput_Internal();

		// Window is required for video always
		if (win32State.initFlags & InitFlags::Video) {
			win32State.initFlags |= InitFlags::Window;
		}

		if (win32State.initFlags & InitFlags::Window) {
			if (!window::Win32InitWindow_Internal(win32State, initSettings)) {
				PushError_Internal("[Win32] Failed creating a window with flags '%d' and settings (Width=%d, Height=%d, Videoprofile=%d)", win32State.initFlags, initSettings.window.windowWidth, initSettings.window.windowHeight, initSettings.video.profile);
				return false;
			}
		}
	#endif // defined(FPL_ENABLE_WINDOW)

		win32State.isInitialized = true;

		return (true);
	}

	fpl_api void ReleasePlatform() {
		FPL_ASSERT(globalWin32State_Internal != nullptr);
		Win32State_Internal &win32State = *globalWin32State_Internal;

		FPL_ASSERT(win32State.isInitialized);

	#if defined(FPL_ENABLE_WINDOW)
		if (win32State.currentSettings.window.isFullscreen) {
			window::Win32LeaveFullscreen_Internal();
		}
		window::Win32UnloadXInput_Internal(win32State.xinput.xinputLibrary);

		switch (win32State.videoDriverType) {
			case VideoDriverType::OpenGL:
			{
			#	if defined(FPL_ENABLE_VIDEO_OPENGL)
				video::Win32ReleaseVideoOpenGLContext_Internal(win32State);
			#	endif
			} break;
			case VideoDriverType::Software:
			{
			#	if defined(FPL_ENABLE_VIDEO_SOFTWARE)
				video::Win32ReleaseVideoSoftwareContext_Internal(win32State);
			#	endif
			} break;
		}

		window::Win32ReleaseWindow_Internal(win32State);
	#endif // defined(FPL_ENABLE_WINDOW)

		memory::MemoryAlignedFree(globalLastError_Internal);
		globalLastError_Internal = nullptr;

		win32State.isInitialized = false;

		fpl::memory::MemoryAlignedFree(fpl::globalWin32State_Internal);
		fpl::globalWin32State_Internal = nullptr;
	}

	fpl_api const char *GetPlatformLastError(const size_t index) {
		const char *result = nullptr;
		if (globalLastError_Internal != nullptr) {
		#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
			if (index > -1 && index < (int32_t)globalLastError_Internal->count) {
				result = globalLastError_Internal->errors[index];
			} else {
				result = globalLastError_Internal->errors[globalLastError_Internal->count - 1];
			}
		#else
			result = globalLastError_Internal->error;
		#endif // defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		}
		return (result);
		}

	fpl_api const char *GetPlatformLastError() {
		const char *result = nullptr;
		if (globalLastError_Internal != nullptr) {
		#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
			if (globalLastError_Internal->count > 0) {
				size_t index = globalLastError_Internal->count - 1;
				result = GetPlatformLastError(index);
			}
		#else
			result = globalLastError_Internal->error;
		#endif // defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
		}
		return (result);
		}

	fpl_api size_t GetPlatformLastErrorCount() {
		size_t result = 0;
		if (globalLastError_Internal != nullptr) {
		#if defined(FPL_ENABLE_MULTIPLE_ERRORSTATES)
			result = globalLastError_Internal->count;
		#else
			result = strings::GetAnsiStringLength(globalLastError_Internal->error) > 0 ? 1 : 0;
		#endif
		}
		return (result);
		}

	fpl_api const Settings &GetCurrentSettings() {
		FPL_ASSERT(globalWin32State_Internal != nullptr);
		const Win32State_Internal &win32State = *globalWin32State_Internal;
		return (win32State.currentSettings);
	}
	}

//
// Win32 Entry-Point
//
#	if defined(FPL_ENABLE_WINDOW)

#		if defined(UNICODE)
int WINAPI wWinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	fpl::globalWin32AppInstance_Internal = appInstance;
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#		else
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
	fpl::globalWin32AppInstance_Internal = appInstance;
	// @TODO(final): Parse command line parameters
	int result = main(0, 0);
	return(result);
}
#		endif // defined(UNICODE)
#	endif // defined(FPL_ENABLE_WINDOW)

#endif // defined(FPL_PLATFORM_WIN32)

#endif // defined(FPL_IMPLEMENTATION) && !defined(FPL_IMPLEMENTED)