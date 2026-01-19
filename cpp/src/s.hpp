// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef S_HPP
#define S_HPP

#include "constant.hpp"
#include "string_def.hpp"

#include "concept.hpp"

#include "icu_util.hpp"


#include <array>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>  // setprecision
#include <cctype>   // tolower, toupper
#include <cwctype>  // towlower, towupper

//#include <unicode/unistr.h> // ICU library

namespace pensar_digital
{
    namespace cpplib
    {
        inline void runtime_error(const S& message)
        {
            #ifdef WIDE_CHAR
                throw std::runtime_error(to_string(message));
            #else
                throw std::runtime_error(message);
            #endif
        }

        // Based on #ifdef WIDE_CHAR defines a constexpr function returning __FILE__ as a wide string or a narrow string.
        inline static constexpr S sfile () 
        {
            #ifdef WIDE_CHAR
                        return __FILEW__;
            #else
                        return __FILE__;
            #endif
        }

        // Reads all chars from a istream into a string.
        // \param is the input stream.
        // \param s the string to hold the result.
        inline S& read_all(InStream& is, S& s)
        {
            C c;
            while (is.get(c))
                s += c;
            return s;
        }

        static inline S& rtrim(S& s)
        {
            #ifdef WIDE_CHAR    
                s.erase(std::find_if_not(s.rbegin(), s.rend(), [](wchar_t c) {return iswspace(c); }).base(), s.end());
            #else
                s.erase(std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) {return iswspace(c); }).base(), s.end());
            #endif
            return s;
         }

        static inline S& ltrim(S& s)
		{
			#ifdef WIDE_CHAR    
				s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](wchar_t c) {return iswspace(c); }));
			#else
				s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](unsigned char c) {return iswspace(c); }));
			#endif
			return s;
		}

        static inline void trim(S& s)
        {
            ltrim(rtrim(s));
        }

        static inline S trim(const S& s)
        {
            S t = s;
            return ltrim(rtrim(t));
        }

        /// An auto_ptr to a vector of strings.
        typedef std::unique_ptr<std::vector<S>> SVectorPtr;

        /// Breaks the string s using the character c as a separator and puts the results in a vector.
        /// \param s the string to be splitted.
        /// \param c the separator char.
        /// \param it the iterator of the container to hold the results.
        /// \param trim_elements if true, split will trim the elements before adding them to vector v.
        template<typename Container = std::vector<S>>
        inline void split(const S& s, C ch,
            Container& c, bool trim_elements = true, bool include_empty_fields = false)//, const std::locale& loc=std::locale())
        {
            size_t i = 0;
            size_t j = s.find(ch);

            if (j == S::npos)
                c.insert(c.end(), s);
            else
                while (j != S::npos)
                {
                    S aux = s.substr(i, j - i);
                    if (trim_elements)
                        trim(aux);
                    if (include_empty_fields || aux.length() > 0)
                        c.insert(c.end(), aux);
                    i = ++j;
                    j = s.find(ch, j);

                    if (j == S::npos)
                    {
                        aux = s.substr(i, s.length());
                        if (trim_elements)
                            trim(aux);
                        if (include_empty_fields || aux.length() > 0)
                            c.insert(c.end(), aux);
                    }
                }
        }

        /// Breaks the string s using the character c as a separator and puts the results in a vector.
        /// Examples: \ref string_util_example.cpp
        /// \param s the string to be splitted.
        /// \param c the separator char.
        /// \param v the vector of strings holding the results.
        /// \param trim_elements if true, split will trim the elements before adding them to vector v.
        /*void split(const S& s, C c, std::vector<S>& v,
            bool trim_elements = true, bool include_empty_fields = false)//, const std::locale& loc=std::locale())
        {
            size_t i = 0;
            size_t j = s.find(c);

            if (j == S::npos)
                v.push_back(s);
            else
                while (j != S::npos)
                {
                    S aux = s.substr(i, j - i);
                    if (trim_elements)
                        trim(aux);
                    if (include_empty_fields || aux.length() > 0)
                        v.push_back(aux);
                    i = ++j;
                    j = s.find(c, j);

                    if (j == S::npos)
                    {
                        aux = s.substr(i, s.length());
                        if (trim_elements)
                            trim(aux);
                        if (include_empty_fields || aux.length() > 0)
                            v.push_back(aux);
                    }
                }
        }*/

        /// Breaks the string s using the character c as a separator and puts the results in a vector.
        /// Examples: \ref string_util_example.cpp
        /// \param s the string to be splitted.
        /// \param c the separator char.
        /// \param trim_elements if true, split will trim the elements before adding them to vector v.
        /// \result An auto_ptr to a vector of strings holding the results.
        /*
        template<typename CharType>
        std::auto_ptr<std::vector<std::basic_string<CharType> > > split (const std::basic_string<CharType>& s, CharType c, bool trim_elements = true)
        {
            std::vector<std::basic_string<CharType> > *vp = new std::vector<std::basic_string<CharType> > ();
            split<CharType> (s, c, *vp, trim_elements);
            return std::auto_ptr<std::vector<std::basic_string<CharType> > > (vp);
        }
        */

        /// \copydoc template<typename CharType> void split(const std::basic_string<CharType>&, CharType, std::vector<std::basic_string<CharType> >*, bool)
        /*inline void split(S& s, C c, std::vector<S>& v, bool trim_elements = true)
        {
            split(s, c, v, trim_elements);
        }*/

        /*
        /// \copydoc template<typename CharType> std::auto_ptr<std::vector<std::basic_string<CharType> > > split (const std::basic_string<CharType>&, CharType, bool)
        inline std::auto_ptr<std::vector<std::string> > split (const std::string& s, char c, bool trim_elements = true)
        {
            return split<char> (s, c, trim_elements);
        }
        */


#ifdef CODE_GEAR
       // extern void        __fastcall split(const AnsiString& as, char c, std::vector<AnsiString>* v, bool trim_elements = true);
        //extern AnsiString  __fastcall only_digits(const AnsiString& s);
        //extern AnsiString  __fastcall remove_acentos(const AnsiString& s);
        //extern AnsiString format_cpf(AnsiString cpf);
#endif

        /// Pads a string to the <b>right</b> with the given character till it reaches the given size.<p>
        /// Important: negative values are not allowed for parameter n. An assertion will end the program if
        /// that happens.
        /// If length is less than s.length () it does nothing, returns the same string.<br>
        /// \param s the string to be padded.
        /// \param c the character to be used to pad the string.
        /// \param n the number of characters the string needs to be padded.
        inline void pad(S& s, const C c, const typename S::size_type n, bool where = PAD_RIGHT)
        {
            if (n == 0)
                return;
            // This is a protection against negative numbers being passed and automatically converted to huge unsigned numbers.
            // Unfortunately most compilers do not warn about it.
            assert(static_cast<int>(n) > 0);

            const typename S::size_type length = s.length();
            if (where == PAD_RIGHT)
            {
                if (n > length)
                    s.append(n - length, c);
            }
            else
            {
                if (n > length)
                    s.insert(0, n - length, c);
            }
        }

        /// \copydoc pad_copy(S&, const T, const typename S::size_type)
        /// \return padded string.
        inline S pad_copy (const S& s, const C c, const typename S::size_type n, bool where = PAD_RIGHT)
        {
            S aux(s);
            pad(aux, c, n, where);
            return aux;
        }

        /// \copydoc pad_copy(S&, const T, const typename S::size_type)
        /// \return Padded string.
        inline S pad_copy(const C* s, const C c, const unsigned n, bool where = PAD_RIGHT)
        {
            assert(s != 0);

            S aux = s;
            pad(aux, c, n, where);
            return aux;
        }

        /// Removes all characters that are not digits from string.
        inline S only_digits(const S& s)
        {
            S out;
            for (const auto& ch : s)
            {
                if (isdigit(ch))
                    out += ch;
            }

            return out;
        }

        //template<typename T = int>
        //struct IsAlphaNumeric : std::unary_function<T,bool>
        //{
        //    bool operator() (const T& ch) const {return isalnum (ch);}
        //};


        /// Removes all characters that are not alpha-numeric from string.
        //template<typename T>
       // void only_alpha_numeric_inplace (S& s)
       // {
        //    if (! s.size ()) return;
        //    std::remove_if (s.begin (), s.end (), std::not1(IsAlphaNumeric<T> ()));
       // }

        /// Removes all characters that are not alpha-numeric from string.
        inline S only_alpha_numeric(const S& s)
        {
            S out;
            for (const C& ch : s)
            {
                if (isalnum(ch))
                    out += ch;
            }

            //std::string out = s;
            //only_alpha_numeric_inplace (out);
            ////std::remove_copy_if (s.begin (), s.end (), out.begin (), std::not1(IsAlphaNumeric<T> ()));
            return out;
        }

        /// Remove accents from string s (replacing accented characters with their base form).
        inline void remove_accents(S& s)
        {
            s = pensar_digital::cpplib::icu_util::remove_accents(s);
        }

        inline S no_accents(const S& s)
        {
            S r = s;
            remove_accents(r);
            return r;
        }

        template <class CharT = char>
        inline bool is_space(CharT c)//, const std::locale& loc = std::locale())
        {
            return std::isspace<CharT>(c, std::locale());//, loc);
        }

        /// Remove blanks from string.
        /// ' '	(0x20)	space (SPC)
        /// '\t'	(0x09)	horizontal tab (TAB)
        /// '\n'	(0x0a)	newline (LF)
        /// '\v'	(0x0b)	vertical tab (VT)
        /// '\f'	(0x0c)	feed (FF)
        /// '\r'	(0x0d)	carriage return (CR)
        template<typename T = char>
        inline void remove_blanks(S& s)
        {
            s.erase(std::remove_if(s.begin(), s.end(), is_space<T>), s.end());
        }

        /// Remove accents from string s (replacing for example � for a).
        template<typename T = char>
        inline S copy_remove_accents(S s)
        {
            S s2 = s;
            remove_accents(s2);
            return s2;
        }

        /// Remove all occurrences of string s from target.
        template<typename T = char>
        inline void remove(const S& s, S& target)
        {
            typename S::size_type pos = target.find(s);
            while (pos != S::npos)
            {
                target.erase(pos, s.length());
                pos = target.find(s);
            }
        }

        /// Remove delimiter char from string.
        template<typename T = char>
        inline void remove_delimiters(T delimiter, S& s)
        {
            if (s[0] == delimiter && s[s.length() - 1] == delimiter)
            {
                s.erase(0, sizeof(T));
                s.erase(s.length() - 1, sizeof(T));
            }
        }

        /// Remove all occurrences of string s from target.
        template<typename T = char>
        inline S copy_remove(const S& s, S target)
        {
            remove(s, target);
            return target;
        }

  
        /// Removes all instances of substring p from s.
        template<typename C = char>
        inline void remove_substr(S& s, const S& p)
        {
            std::string::size_type n = p.length();
            for (std::string::size_type i = s.find(p); i != std::string::npos; i = s.find(p))
                s.erase(i, n);
        }

        /// Removes all instances of substring p from s.
        // o: original substring
        // r: replacement
        inline bool replace_substr(S& s, const S& o, const S& r)
        {
            if (s.size() == 0)
                return false;
            bool replaced = false;
            S::size_type n = o.length();
            for (S::size_type i = s.find(o); i != S::npos; i = s.find(o))
            {
                s.replace(i, n, r);
                replaced = true;
            }
            return replaced;
        }

        inline bool replace_substr (S& s, const C* o, const C* r)
        {
            return replace_substr (s, S(o), S(r));
        }

        inline S insert_grouping_char (const S& s, typename S::value_type grouping_char = W(','))
        {
            S f; //formatted string.
            unsigned primeiro = s.length() % 3;
            if (primeiro == 0)
                primeiro = 3;
            --primeiro;
            unsigned i = 0;
            unsigned count = 0;
            for (typename S::const_iterator it = s.begin(); it != s.end(); ++it)
            {
                f += *it;
                bool last = (++it == s.end()) ? true : false;
                --it;
                if ((i++ == primeiro || ++count == 3) && !last)
                {
                    f += grouping_char;
                    count = 0;
                }
            }
            return f;
        }

        inline std::wstring to_wstring(const std::string& s)
        {
            return pensar_digital::cpplib::icu_util::to_wstring(s);
        }

        template<typename IntType = int, bool use_grouping_char = false>
        S to_string(IntType number, C grouping_char = W(','))
        {
            OutStringStream ss;
            ss << number;
            S s = ss.str();
            //if (number < 0) inserts a minus sign at the beginning of the string.
            if (number < 0)
                s.insert(s.begin(), W('-'));

            return use_grouping_char ? insert_grouping_char(s, grouping_char) : s;
        }

        template<bool use_grouping_char = false>
        S to_string(size_t number, C grouping_char = W(','))
        {
            return to_string <size_t, use_grouping_char>(number, grouping_char);
        }

        template<bool use_grouping_char>
        S to_string(int number, C grouping_char)
        {
            return to_string <int, use_grouping_char>(number, grouping_char);
        }

        template<bool use_grouping_char>
        S to_string(long number, C grouping_char)
        {
            return to_string <long, use_grouping_char>(number, grouping_char);
        }

        template<bool use_grouping_char>
        S to_string(long long int number, C grouping_char)
        {
            return to_string <long long int, use_grouping_char>(number, grouping_char);
        }

        template<bool use_grouping_char>
        S to_string(unsigned int number, C grouping_char)
        {
            return to_string <unsigned int, use_grouping_char, C>(number, grouping_char);
        }

        //template<bool use_grouping_char>
        //S to_string(unsigned long int number, C grouping_char)
        //{
        //    return to_string <unsigned long int, use_grouping_char>(number, grouping_char);
        //}

        //template<bool use_grouping_char = false>
        //S to_string(unsigned long long int number, typename String::value_type grouping_char = W(','))
        //{
        //    return to_string <unsigned long long int, use_grouping_char>(number, grouping_char);
       // }

        // todo: use use_grouping.
        inline S to_string(double number, unsigned num_decimals /*= 2*/, bool use_grouping/* = true*/, C grouping_char = W(','), C decimal_separator = W(','))
        {
            long long int integer_part = (long long)trunc(number);
            S s = to_string(integer_part);
            SStream ss;
            ss << std::fixed << std::setprecision(num_decimals) << number;
            S s1 = ss.str();
            typename S::size_type pos = s1.find(decimal_separator);
            S decimal_part = W(".00");
            if (pos != S::npos)
            {
                size_t n = (num_decimals + pos) < s1.length() ? num_decimals : s1.length();
                decimal_part = s1.substr(pos, n + 1);
            }
            pad(decimal_part, W('0'), num_decimals);
            return s + decimal_part;
        }
        // Converts from std::wstring to std::string.
        inline std::string  to_string(const std::wstring& s)
        {
#ifdef WINDOWS
            return icu_util::utf16_to_utf8(s);
#else
            return icu_util::utf32_to_utf8(s);
#endif
        }

        inline S pad_left0(long long int number, const unsigned n = 4)
        {
            return pad_copy(pd::to_string<decltype(number), false>(number).c_str(), W('0'), n, PAD_LEFT);
        }

        /// Remove extension from file name.
        inline void remove_ext(S& fname)
        {
            typename S::size_type pos = fname.find_last_of(W('.'));
            if (pos != S::npos)
            {
                fname.erase(pos, fname.length());
            }
        }

        /// Remove todos espa�os duplos.
        inline S& remove_double_spaces(S& s)
        {
            if (s.size() == 0)
                return s;
            while (replace_substr (s, W("  "), W(" ")));
            return s;
        }

        //---------------------------------------------------------
        //----------------------------------------------------------------------
 
 

        inline S  reverse(const S& s)
        {
            S out;
            for (typename S::const_reverse_iterator i = s.crbegin(); i != s.crend(); ++i)
            {
                out += *i;
            }
            return out;
        }

        inline void to_upper(S& s)
        {
            for (S::iterator p = s.begin(); p != s.end(); ++p)
            {
#ifdef WIDE_CHAR
                *p = std::towupper(*p);
#else
                *p = std::toupper(static_cast<unsigned char>(*p));
#endif
            }
        }

        inline void to_lower(S& s)
        {
            for (S::iterator p = s.begin(); p != s.end(); ++p)
            {
#ifdef WIDE_CHAR
                *p = std::towlower(*p);
#else
                *p = std::tolower(static_cast<unsigned char>(*p));
#endif
            }
        }

        inline S upper(const S& s)
        {
            S r = s;
            to_upper(r);
            return r;
        }

        inline S lower(const S& s)
        {
            S r = s;
            to_lower(r);
            return r;
        }

        /*void troca_char(wchar_t* c, const std::locale& loc)
        {
            //T ch = std::use_facet<std::ctype<T> >(loc).tolower (*c);
            switch (*c)
            {
                // According to codepage ISO/IEC 8859-1
            case L'\u00C0'): // A com crase.
            case L'\u00C1'): // A com agudo.
            case L'\u00C2'): // A com circunflexo.
            case L'\u00C3'): // A com til.
            case L'\u00C4'): // A com trema.
            case L'\u00C5'): // A com bolinha de angstron.
                *c = L'A');
                break;
            case L'\u00C8'):  // E com crase.
            case L'\u00C9'):  // E com agudo.
            case L'\u00CA'):  // E com circunflexo.
            case L'\u00CB'):  // E com trema.
                *c = L'E');
                break;
            case L'\u00CC'):  // I com crase.
            case L'\u00CD'):  // I com agudo.
            case L'\u00CE'):  // I com circunflexo.
            case L'\u00CF'):  // I com trema.
                *c = L'I');
                break;
            case L'\u00D2'): // O com crase.
            case L'\u00D3'): // O com agudo.
            case L'\u00D4'): // O com circunflexo.
            case L'\u00D5'): // O com til.
            case L'\u00D6'): // O com trema.
                *c = L'O');
                break;
            case L'\u00D9'): // U com crase.
            case L'\u00DA'): // U com agudo.
            case L'\u00DB'): // U com circunflexo.
            case L'\u00DC'): // U com trema.
                *c = L'U');
                break;
            case L'\u00C7'): // C cidilha.
                *c = L'C');
                break;
            case L'\u00D1'): // N com til
                *c = L'N');
                break;
            case L'\u00E0'): // a com crase.
            case L'\u00E1'): // a com agudo.
            case L'\u00E2'): // a com circunflexo.
            case L'\u00E3'): // a com til.
            case L'\u00E4'): // a com trema.
                *c = L'a');
                break;
            case L'\u00E8'):  // e com crase.
            case L'\u00E9'):  // e com agudo.
            case L'\u00EA'):  // e com circunflexo.
            case L'\u00EB'):  // e com trema.
                *c = L'e');
                break;
            case L'\u00EC'):  // i com crase.
            case L'\u00ED'):  // i com agudo.
            case L'\u00EE'):  // i com circunflexo.
            case L'\u00EF'):  // i com trema.
                *c = L'i');
                break;
            case L'\u00F2'): // o com crase.
            case L'\u00F3'): // o com agudo.
            case L'\u00F4'): // o com circunflexo.
            case L'\u00F5'): // o com til.
            case L'\u00F6'): // o com trema.
                *c = L'o');
                break;
            case L'\u00E7'): // c cidilha.
                *c = L'c');
                break;
            case L'\u00F1'): // n com til
                *c = L'n');
                break;
            case L'\u00F9'): // u com crase.
            case L'\u00FA'): // u com agudo.
            case L'\u00FB'): // u com circunflexo.
            case L'\u00FC'): // u com trema.
                *c = L'u');
                break;
            default:
                break;
            }
        }
        */
#ifdef CODE_GEAR
        /*void __fastcall split(const AnsiString& as, char c, std::vector<AnsiString>* v, bool trim_elements)
        {
            S s = as.c_str();
            S::size_type i = 0;
            S::size_type j = s.find(c);

            if (j == S::npos)
                v->push_back(as);
            else
                while (j != S::npos)
                {
                    S aux = s.substr(i, j - i);
                    if (trim_elements)
                        trim(aux);
                    AnsiString s1 = aux.c_str();
                    v->push_back(s1);
                    i = ++j;
                    j = s.find(c, j);

                    if (j == S::npos)
                    {
                        aux = s.substr(i, s.length());
                        if (trim_elements)
                            trim(aux);
                        s1 = aux.c_str();
                        v->push_back(s1);
                    }
                }
        }

        AnsiString __fastcall only_digits(const AnsiString& s)
        {
            AnsiString d = "";
            for (int i = 1; i <= s.Length(); i++)
            {
                if (isdigit(s[i]))
                    d += s[i];
            }
            return d;
        }

        AnsiString __fastcall remove_acentos(const AnsiString& s)
        {
            AnsiString sem_acentos = s;
            for (int i = 1; i <= s.Length(); ++i)
            {
                troca_char<char>(&sem_acentos[i]);
            }
            return sem_acentos;
        }*/
#endif
        //----------------------------------------------------------------------


        /*void remove_accent(wchar_t* c) noexcept
        {
            //T ch = std::use_facet<std::ctype<T> >(loc).tolower (*c);
            switch (*c)
            {
                // According to codepage ISO/IEC 8859-1
            case L'\u00C0'): // A com crase.
            case L'\u00C1'): // A com agudo.
            case L'\u00C2'): // A com circunflexo.
            case L'\u00C3'): // A com til.
            case L'\u00C4'): // A com trema.
            case L'\u00C5'): // A com bolinha de angstron.
                *c = L'A');
                break;
            case L'\u00C8'):  // E com crase.
            case L'\u00C9'):  // E com agudo.
            case L'\u00CA'):  // E com circunflexo.
            case L'\u00CB'):  // E com trema.
                *c = L'E');
                break;
            case L'\u00CC'):  // I com crase.
            case L'\u00CD'):  // I com agudo.
            case L'\u00CE'):  // I com circunflexo.
            case L'\u00CF'):  // I com trema.
                *c = L'I');
                break;
            case L'\u00D2'): // O com crase.
            case L'\u00D3'): // O com agudo.
            case L'\u00D4'): // O com circunflexo.
            case L'\u00D5'): // O com til.
            case L'\u00D6'): // O com trema.
                *c = L'O');
                break;
            case L'\u00D9'): // U com crase.
            case L'\u00DA'): // U com agudo.
            case L'\u00DB'): // U com circunflexo.
            case L'\u00DC'): // U com trema.
                *c = L'U');
                break;
            case L'\u00C7'): // C cidilha.
                *c = L'C');
                break;
            case L'\u00D1'): // N com til
                *c = L'N');
                break;
            case L'\u00E0'): // a com crase.
            case L'\u00E1'): // a com agudo.
            case L'\u00E2'): // a com circunflexo.
            case L'\u00E3'): // a com til.
            case L'\u00E4'): // a com trema.
                *c = L'a');
                break;
            case L'\u00E8'):  // e com crase.
            case L'\u00E9'):  // e com agudo.
            case L'\u00EA'):  // e com circunflexo.
            case L'\u00EB'):  // e com trema.
                *c = L'e');
                break;
            case L'\u00EC'):  // i com crase.
            case L'\u00ED'):  // i com agudo.
            case L'\u00EE'):  // i com circunflexo.
            case L'\u00EF'):  // i com trema.
                *c = L'i');
                break;
            case L'\u00F2'): // o com crase.
            case L'\u00F3'): // o com agudo.
            case L'\u00F4'): // o com circunflexo.
            case L'\u00F5'): // o com til.
            case L'\u00F6'): // o com trema.
                *c = L'o');
                break;
            case L'\u00E7'): // c cidilha.
                *c = L'c');
                break;
            case L'\u00F1'): // n com til
                *c = L'n');
                break;
            case L'\u00F9'): // u com crase.
            case L'\u00FA'): // u com agudo.
            case L'\u00FB'): // u com circunflexo.
            case L'\u00FC'): // u com trema.
                *c = L'u');
                break;
            default:
                break;
            }
        }*/

    }   // namespace cpplib
}       // namespace pensar_digital
#endif  // S_HPP

