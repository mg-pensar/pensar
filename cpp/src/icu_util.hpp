// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef ICU_UTIL_HPP
#define ICU_UTIL_HPP 

// ICU headers.
// Works with vcpkg, system installations, and package managers (Conan, Homebrew, etc.)
#include <unicode/ucsdet.h>
#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/normalizer2.h>
#include <unicode/uchar.h>
#include <unicode/utf16.h>


// std c++ headers.
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include <exception>

#include <stdio.h>
#include <string.h>

// cpplib headers.
#include "string_def.hpp"
#include "log.hpp"
#include "encoding.hpp"


namespace pensar_digital
{
    namespace cpplib
    {
        namespace icu_util
        {
            inline static const size_t BUFFER_SIZE = 1024;
            
            //Gets the max value of int32_t.
            #undef max // This is necessary because of a conflict with the max macro in Windows.h
            static inline constexpr size_t max_int32_t = std::numeric_limits<int32_t>::max();
            
            // Function to convert a UTF-8 string to a UChar string using ICU
            inline void convert_utf8_to_uchar(const char* src, UChar* dest, int32_t destCapacity) {
                UErrorCode status = U_ZERO_ERROR;
                // Calculate the required buffer size
                int32_t destLen = ucnv_toUChars(NULL, NULL, 0, src, -1, &status);
                status = U_ZERO_ERROR; // Reset status for actual conversion

                if (destLen <= destCapacity) {
                    // Perform the conversion
                    ucnv_toUChars(NULL, dest, destLen, src, -1, &status);

                    if (U_FAILURE(status)) 
                    {
                        std::basic_string<C> msg = W("convert_utf8_to_uchar: Failed to convert error message");
                        LOG(msg);
                        throw msg;
                    }
                }
                else 
                {
                    std::stringstream msg;
                    msg << "convert_utf8_to_uchar: destination buffer is too small. Required size = " << destLen << " but it was " << destCapacity;
					std::basic_string<C> error = W(msg.str());
                    LOG(error);
                    throw error;
                }
            }


            inline void char_ptr_to_uchar_ptr(const char* msg, UChar* out)
            {
                // Gets msg length and throws an error if it is too big.
                if (strlen(msg) > BUFFER_SIZE)
                {
                    std::stringstream msg;
                    msg << "char_ptr_to_uchar_ptr: Message is too big to be converted to UChar string. Max. size = " << BUFFER_SIZE;
                    std::basic_string<C> error = W(msg.str());
                    LOG(error);
                    throw error;
                }
                convert_utf8_to_uchar (msg, out, BUFFER_SIZE); // Convert to UChar string
            }

            inline void utf8_char_ptr_to_wchar_t_ptr (const char* msg, wchar_t* out)
            {
                UChar uMsg[BUFFER_SIZE];
                convert_utf8_to_uchar (msg, uMsg, BUFFER_SIZE); // Convert to UChar string

                UErrorCode status = U_ZERO_ERROR;

                wchar_t wcharMsg[BUFFER_SIZE];
                u_strToWCS(wcharMsg, BUFFER_SIZE, NULL, uMsg, -1, &status);
                if ( !U_SUCCESS(status)) 
                {
                    std::basic_string<C> error = W("char_ptr_to_wchar_ptr: Error converting UChar to wchar_t");
                    LOG(error);
                    throw error;
                }
            }

            inline std::wstring to_wstring(const std::string& s)
            {
                // Convert a UTF-8 string to a UTF-16 string using ICU library.
                UErrorCode status = U_ZERO_ERROR;
                UConverter* conv = ucnv_open("utf-8", &status);
                if (U_FAILURE(status)) {
					wprintf(L"Failed to open the converter\n");
				}
                // Calculate the required buffer size
				int32_t destLen = ucnv_toUChars(conv, NULL, 0, s.c_str(), -1, &status);
				status = U_ZERO_ERROR; // Reset status for actual conversion

				UChar* dest = new UChar[destLen];
				ucnv_toUChars(conv, dest, destLen, s.c_str(), -1, &status);

                if (U_FAILURE(status)) {
					wprintf(L"Failed to convert error message\n");
				}

				// Convert the UChar string to a wchar_t string
				wchar_t wcharMsg[BUFFER_SIZE];
				u_strToWCS(wcharMsg, BUFFER_SIZE, NULL, dest, -1, &status);
                if (!U_SUCCESS(status))
                {
 				}

				return std::wstring(wcharMsg);

            }

            // Converts from std::wstring to std::string.
            inline std::string utf16_to_utf8 (const std::wstring& s)
            {
                // Convert a UTF-16 string to a UTF-8 string using ICU library.
                UErrorCode status = U_ZERO_ERROR;
                UConverter* conv = ucnv_open("utf-8", &status);
                if (U_FAILURE(status))
                {
                    std::basic_string<C> error = W("pd::icu_util::utf16_to_utf8: Error getting converter.");
                    LOG(error);
                    throw error;
                }

                // Convert std::wstring to UChar*
                const UChar* source = reinterpret_cast<const UChar*>(s.c_str());
                
                size_t slen = s.length();
                if (slen > max_int32_t)
                {
                    std::basic_string<C> error = W("pd::icu_util::utf16_to_utf8: Error converting string. Buffer size is too big.");
                    LOG(error);
                    throw error;
                }
                uint32_t len = static_cast<uint32_t>(slen); 
                // Calculate the required buffer size for the UTF-8 string
                int32_t destLen = ucnv_fromUChars(conv, NULL, 0, source, len, &status);
                status = U_ZERO_ERROR; // Reset status for actual conversion

                // Allocate buffer for the UTF-8 string
                char* dest = new char[destLen + 1]; // +1 for null terminator

                // Perform the conversion
                ucnv_fromUChars(conv, dest, destLen + 1, source, len, &status);

                if (U_FAILURE(status))
                {
                    S error = W("pd::icu_util::utf16_to_utf8: Error converting string.");
                    LOG(error);
                    throw error;
                }

                std::string result(dest, destLen); // Construct the result string

                delete[] dest; // Clean up the allocated buffer
                ucnv_close(conv); // Close the converter

                return result;
            }

            #ifdef Windows
                // Converts from std::wstring to std::string.
                std::string to_string(const std::wstring& s)
                {
                    return utf16_to_utf8(s);
                }
            #endif

            inline std::string utf32_to_utf8(const std::wstring& s) 
            {
                UErrorCode status = U_ZERO_ERROR;

                // Open a converter for UTF-32 to UTF-8 conversion
                UConverter* convToUTF8 = ucnv_open("UTF-8", &status);
                if (U_FAILURE(status)) 
                {
                    S error = W("1. pd::icu_util::utf32_to_utf8: Failed to open UTF-8 converter.");
                    LOG(error);
                    throw error;
                }

                UConverter* convFromUTF32 = ucnv_open("UTF-32", &status);
                if (U_FAILURE(status)) 
                {
                    ucnv_close(convToUTF8); // Ensure the first converter is closed before throwing
                    S error = W("2. pd::icu_util::utf32_to_utf8: Failed to open UTF-32 converter.");
                    LOG(error);
                    throw error;
                }
                
                size_t slen = s.length();
                if (slen > max_int32_t)
                {
                    S error = W("pd::icu_util::utf32_to_utf8: Error converting string. Buffer size is too big.");
                    LOG(error);
                    throw error;
                }
                int32_t srcLength = static_cast<int32_t>(slen);

                // Convert std::wstring (UTF-32) to UChar*
                const UChar* source = reinterpret_cast<const UChar*>(s.c_str());

                // Calculate the required buffer size for the UTF-8 string
                int32_t utf8Length = ucnv_fromUChars(convToUTF8, NULL, 0, source, srcLength, &status);
                if (U_FAILURE(status)) 
                {
                    ucnv_close(convToUTF8);
                    ucnv_close(convFromUTF32);
                    S error = W("pd::icu_util::utf32_to_utf8: Failed to calculate buffer size for UTF-8 string.");
                    LOG(error);
                    throw error;
                }

                // Allocate buffer for the UTF-8 string
                char* utf8String = new char[utf8Length + 1]; // +1 for null terminator

                // Reset status for actual conversion
                status = U_ZERO_ERROR;

                // Perform the conversion
                ucnv_fromUChars(convToUTF8, utf8String, utf8Length + 1, source, srcLength, &status);
                if (U_FAILURE(status)) {
                    delete[] utf8String;
                    ucnv_close(convToUTF8);
                    ucnv_close(convFromUTF32);
                    S error = W("pd::icu_util::to_string: Failed to convert string from UTF-32 to UTF-8.");
                    LOG(error);
                    throw error;
                }

                std::string result(utf8String, utf8Length); // Construct the result string

                delete[] utf8String; // Clean up the allocated buffer
                ucnv_close(convToUTF8);
                ucnv_close(convFromUTF32);

                return result;
			}   // utf32_to_utf8

            inline std::string to_utf8(const S& s)
            {
#ifdef WIDE_CHAR
    #ifdef WINDOWS
                return utf16_to_utf8(s);
    #else
                return utf32_to_utf8(s);
    #endif
#else
                return std::string(s);
#endif
            }

            inline icu::UnicodeString to_unicode_string(const S& s)
            {
                std::string utf8 = to_utf8(s);
                return icu::UnicodeString::fromUTF8(utf8);
            }

            inline std::vector<UChar32> to_codepoints(const S& s)
            {
                icu::UnicodeString ustr = to_unicode_string(s);
                std::vector<UChar32> codepoints;
                codepoints.reserve(static_cast<size_t>(ustr.countChar32()));

                for (int32_t i = 0; i < ustr.length(); )
                {
                    UChar32 c = ustr.char32At(i);
                    codepoints.push_back(c);
                    i += U16_LENGTH(c);
                }

                return codepoints;
            }

			inline bool check_bom (const std::vector<unsigned char>& buffer, Encoding& encoding)
			{
                bool detected = true;
                if (buffer.size () >= 4)
                {
                    if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xFE && buffer[3] == 0xFF)
                    {
                        encoding = UTF_32_BE_BOM;
                    }
                    else if (buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0x00 && buffer[3] == 0x00)
                    {
                        encoding = UTF_32_LE_BOM;
                    }
                }
                else
                    if (buffer.size() >= 3)
                    {
                        if (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF)
                        {
                            encoding = UTF_8_BOM;
                        }
                    }
                    else
                        if (buffer.size() >= 2)
                        {
                            if (buffer[0] == 0xFE && buffer[1] == 0xFF)
                            {
                                encoding = UTF_16_BE_BOM;
                            }
                            else if (buffer[0] == 0xFF && buffer[1] == 0xFE)
                            {
                                encoding = UTF_16_LE_BOM;
                            }
                        }
                        else
                            detected = false;
                return detected;
			}

            inline Encoding s_encoding_to_encoding(const std::string& s_encoding)
            {
				if (s_encoding == "ASCII")
				{
					return ASCII;
				}
				else if (s_encoding == "UTF-8")
				{
					return UTF_8;
				}
				else if (s_encoding == "UTF-8-BOM")
				{
					return UTF_8_BOM;
				}
				else if (s_encoding == "UTF-16-BE")
				{
					return UTF_16_BE;
				}
				else if (s_encoding == "UTF-16-BE-BOM")
				{
					return UTF_16_BE_BOM;
				}
				else if (s_encoding == "UTF-16-LE")
				{
					return UTF_16_LE;
				}
				else if (s_encoding == "UTF-16-LE-BOM")
				{
					return UTF_16_LE_BOM;
				}
				else if (s_encoding == "UTF-32-BE")
				{
					return UTF_32_BE;
				}
				else if (s_encoding == "UTF-32-BE-BOM")
				{
					return UTF_32_BE_BOM;
				}
				else if (s_encoding == "UTF-32-LE")
				{
					return UTF_32_LE;
				}
				else if (s_encoding == "UTF-32-LE-BOM")
				{
					return UTF_32_LE_BOM;
				}
				else
				{
					throw std::runtime_error("Error: Unknown encoding " + s_encoding);
				}
			}

            /// Remove accents from UTF-8 string by decomposing characters and removing combining marks.
            inline std::string remove_accents(const std::string& s)
            {
                UErrorCode status = U_ZERO_ERROR;
                
                // Get NFD normalizer instance (decomposes accented chars into base + combining diacritic)
                const icu::Normalizer2* nfd = icu::Normalizer2::getNFDInstance(status);
                if (U_FAILURE(status))
                {
                    throw std::runtime_error("remove_accents: Failed to get NFD normalizer instance");
                }
                
                // Convert UTF-8 string to UChar buffer
                int32_t srcLen = static_cast<int32_t>(s.length());
                if (s.length() > max_int32_t)
                {
                    throw std::runtime_error("remove_accents: String is too long");
                }
                
                // Calculate buffer size needed for UChars
                UConverter* conv = ucnv_open("UTF-8", &status);
                if (U_FAILURE(status))
                {
                    throw std::runtime_error("remove_accents: Failed to open UTF-8 converter");
                }
                
                int32_t ucharLen = ucnv_toUChars(conv, NULL, 0, s.c_str(), srcLen, &status);
                status = U_ZERO_ERROR;
                
                UChar* uchars = new UChar[ucharLen + 1];
                ucnv_toUChars(conv, uchars, ucharLen + 1, s.c_str(), srcLen, &status);
                ucnv_close(conv);
                
                if (U_FAILURE(status))
                {
                    delete[] uchars;
                    throw std::runtime_error("remove_accents: Failed to convert to UChars");
                }
                
                // Normalize using NFD
                icu::UnicodeString ustr(uchars, ucharLen);
                delete[] uchars;
                
                icu::UnicodeString normalized = nfd->normalize(ustr, status);
                if (U_FAILURE(status))
                {
                    throw std::runtime_error("remove_accents: Failed to normalize string");
                }
                
                // Remove combining diacritical marks (category Mn - Non-spacing Mark)
                icu::UnicodeString result;
                for (int32_t i = 0; i < normalized.length(); ++i)
                {
                    UChar32 c = normalized.char32At(i);
                    if (u_charType(c) != U_NON_SPACING_MARK)
                    {
                        result.append(c);
                    }
                }
                
                // Convert back to UTF-8 std::string
                std::string output;
                result.toUTF8String(output);
                return output;
            }

            /// Remove accents from wide string.
            inline std::wstring remove_accents(const std::wstring& s)
            {
#ifdef WINDOWS
                // Windows uses UTF-16
                return to_wstring(remove_accents(utf16_to_utf8(s)));
#else
                // Unix/Linux/macOS use UTF-32
                return to_wstring(remove_accents(utf32_to_utf8(s)));
#endif
            }

            inline Encoding detect_encoding (const std::string& file_name)
            {
                
                std::ifstream file_stream(file_name, std::ios::binary);
                if (!file_stream.is_open())
                {
                    throw std::runtime_error("Error: Could not open file " + file_name);
                }
                
                constexpr size_t MAX_BYTES = 10000; // Sufficient for ucsdet_setText, if changed it cannot exceed INT32_MAX.
                std::vector<unsigned char> buffer(MAX_BYTES); // Allocate fixed size
                file_stream.read(reinterpret_cast<char*>(buffer.data()), MAX_BYTES);
                std::streamsize bytes_read = file_stream.gcount();
                if (file_stream.bad()) {
                    file_stream.close();
                    throw std::runtime_error("Error: Could not read file " + file_name);
                }
                file_stream.close();

                // Check for BOM
                Encoding bom_encoding;
                if (check_bom (buffer, bom_encoding))
                {
                    return bom_encoding;
                }

                // Use ICU to detect the encoding
                UErrorCode error = U_ZERO_ERROR;
                UCharsetDetector* csd = ucsdet_open(&error);
                if (U_FAILURE(error))
                {
                    throw std::runtime_error("Error: Could not open ICU charset detector");
                }
                
                ucsdet_setText(csd, reinterpret_cast<const char*>(buffer.data()), static_cast<int32_t>(buffer.size()), &error);
                
                if (U_FAILURE(error))
                {
                    ucsdet_close(csd);
                    throw std::runtime_error("Error: Could not set text for ICU charset detector");
                }

                const UCharsetMatch* match = ucsdet_detect(csd, &error);
                if (U_FAILURE(error))
                {
                    ucsdet_close(csd);
                    throw std::runtime_error("Error: Could not detect charset");
                }

                const char* detected_encoding = ucsdet_getName(match, &error);
                if (U_FAILURE(error))
                {
                    ucsdet_close(csd);
                    throw std::runtime_error("Error: Could not get detected charset name");
                }

                std::string s_encoding(detected_encoding);
                ucsdet_close(csd);

                return s_encoding_to_encoding (detected_encoding);
            }

            /// Remove accent from a single character using ICU normalization.
            /// Returns the base character without diacritical marks.
            template<typename CharT>
            inline void remove_accent_char(CharT* c) noexcept
            {
                static_assert(sizeof(CharT) == sizeof(char) || sizeof(CharT) == sizeof(wchar_t), 
                              "CharT must be char or wchar_t");
                
                if (*c < 128) return; // ASCII fast path - no accents to remove
                
                UErrorCode status = U_ZERO_ERROR;
                
                // Get NFD normalizer instance (cached by ICU)
                const icu::Normalizer2* nfd = icu::Normalizer2::getNFDInstance(status);
                if (U_FAILURE(status)) return; // Fail silently, keep original character
                
                // Create UnicodeString from single character
                icu::UnicodeString input(static_cast<UChar32>(*c));
                icu::UnicodeString normalized = nfd->normalize(input, status);
                if (U_FAILURE(status)) return;
                
                // Get first non-combining character (the base character)
                for (int32_t i = 0; i < normalized.length(); ++i)
                {
                    UChar32 ch = normalized.char32At(i);
                    if (u_charType(ch) != U_NON_SPACING_MARK)
                    {
                        *c = static_cast<CharT>(ch);
                        return;
                    }
                }
            }
		}   // namespace icu_util
	}   // namespace cpplib
}   // namespace pensar_digital::cpplib


#endif // ICU_UTIL_HPP