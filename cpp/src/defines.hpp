#ifndef DEFINES_HPP				// This is a header guard. Used to prevent the header from being included more than once.
	#define DEFINES_HPP
	
	#define WINDOWS				// Comment this out if you are not using Windows.
	#ifdef WINDOWS
		#define VISUAL_STUDIO	// This is a define that is used to include the precompiled header in Visual Studio.
	#endif

namespace pensar_digital
{
	namespace cpplib
	{
	}
}

#endif	// DEFINES_HPP


