#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "constant.hpp"
#include <cstring> // memcpy

// Detects and includes platform-specific implementation header.
// Examples:
// #include "windows/io_util_windows.hpp"
// #include "linux/io_util_linux.hpp" 
// #include "android/io_util_android.hpp" or other platforms
//#include INCLUDE(memory)

namespace pensar_digital
{
	namespace cpplib
	{
		extern size_t getPeakRSS();
		extern size_t get_available_memory();


		// Helper to avoid static_casts all over the place.
		static inline BytePtr memory_copy(const BytePtr dest, const BytePtr src, size_t size)
		{
			std::memcpy(static_cast <void*> (dest), static_cast <void*> (src), size);
			return dest;
		}
	}
}
#endif // MEMORY_H_INCLUDED
