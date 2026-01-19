#ifndef MULTIPLATFORM_HPP
#define MULTIPLATFORM_HPP

// Include TargetConditionals.h for Apple platform detection
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace pensar_digital
{
	namespace cpplib
	{
		// Include platform-specific implementation
		// Define the platform-specific folder. Example:
		// #include INCLUDE(io_util) translates to: "windows/io_util_windows.hpp"

#if defined(_WIN32) || defined(_WIN64)
#define _PLATFORM_FOLDER windows
#define _PLATFORM_SUFFIX windows
#define _INCLUDE_PATH(base_name) _STRINGIFY(./windows/base_name##_windows.hpp)
#define INCLUDE(base_name) _INCLUDE_PATH(base_name)
#elif defined(__linux__)
#define _PLATFORM_FOLDER linux
#define _PLATFORM_SUFFIX linux
#define _INCLUDE_PATH(base_name) _STRINGIFY(./linux/base_name##_linux.hpp)
#define INCLUDE(base_name) _INCLUDE_PATH(base_name)
#elif defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
#define _PLATFORM_FOLDER macos
#define _PLATFORM_SUFFIX macos
#define _INCLUDE_PATH(base_name) _STRINGIFY(./macos/base_name##_macos.hpp)
#define INCLUDE(base_name) _INCLUDE_PATH(base_name)
#elif defined(__APPLE__) && TARGET_OS_IPHONE
#define _PLATFORM_FOLDER ios
#define _PLATFORM_SUFFIX ios
#define _INCLUDE_PATH(base_name) _STRINGIFY(./ios/base_name##_ios.hpp)
#define INCLUDE(base_name) _INCLUDE_PATH(base_name)
#elif defined(__ANDROID__)
#define _PLATFORM_FOLDER android
#define _PLATFORM_SUFFIX android
#define _INCLUDE_PATH(base_name) _STRINGIFY(./android/base_name##_android.hpp)
#define INCLUDE(base_name) _INCLUDE_PATH(base_name)
#else
#error "Unsupported platform"
#endif

// Helper macro for stringification
#define _STRINGIFY(x) #x

	}  // namespace cpplib
}  // namespace pensar_digital

#endif // MULTIPLATFORM_HPP