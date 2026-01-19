// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef STRING_DEF_HPP_INCLUDED
#define STRING_DEF_HPP_INCLUDED

namespace pensar_digital
{
	namespace cpplib
	{
        //#define WIDE_CHAR   ///< Define WIDE_CHAR to use wide char strings. Comment this line to use char strings.
        #ifdef WIDE_CHAR 
            using C = wchar_t;
            // define macro W(x) to be used with string literals it just adds L before x.
            #define W(x) L ## x    
        #else
            using C = char;
            // define macro W(x) to be used with string literals it just repeats x.
            #define W(x) x
        #endif // WIDE_CHAR 

            inline static const C* EMPTY = W("");
            inline static const C  SPACE = W(' ');
            inline static const C* CURRENT_DIR = W(".");
            inline static const C* DEFAULT_TXT_FILE_EXTENSION = W(".txt");
            inline static const C* DEFAULT_BIN_FILE_EXTENSION = W(".bin");
            inline static const C* DOUBLE_QUOTES = W("\"");

            inline static const C NULL_CHAR = W('\0');

            using S = std::basic_string<C>;
            using SView = std::basic_string_view<C>;
            using SIter = std::basic_string<C>::iterator;
            using SConstIter = std::basic_string<C>::const_iterator;
            using InStream = std::basic_istream<C>;
            using OutStream = std::basic_ostream<C>;
            using SStream = std::basic_stringstream<C>;
            using InStreamBuf = std::basic_streambuf<C>;
            using OutStreamBuf = std::basic_streambuf<C>;
            using InStreamBufIter = std::istreambuf_iterator<C>;
            using OutStreamBufIter = std::ostreambuf_iterator<C>;
            using InStringStream = std::basic_istringstream<C>;
            using OutStringStream = std::basic_ostringstream<C>;
            using InFStream = std::basic_ifstream<C>;
            using OutFStream = std::basic_ofstream<C>;
            using FStream = std::basic_fstream<C>;
            using InFileBuf = std::basic_filebuf<C>;
            using InFileBufIter = std::istreambuf_iterator<C>;
            using OutFileBuf = std::basic_filebuf<C>;
            using OutFileBufIter = std::ostreambuf_iterator<C>;

    }
}

#endif // STRING_DEF_HPP_INCLUDED
