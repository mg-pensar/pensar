// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef MACROS_HPP_INCLUDED
#define MACROS_HPP_INCLUDED

	#define WIDE2(x) L##x
	#define WIDE1(x) WIDE2(x)
	#define __WFILE__ WIDE1(__FILE__)

	#ifdef WIDE_CHAR
		#define FILE_LINE  __WFILE__ << L"\t linha " << __LINE__ << L"\t"
	#else
		#define FILE_LINE  __FILE__ << "\t linha " << __LINE__ << "\t"
	#endif

#endif // MACROS_HPP_INCLUDED
