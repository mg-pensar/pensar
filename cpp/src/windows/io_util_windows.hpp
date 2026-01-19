// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef WINDOWS_IO_UTIL_HPP
#define WINDOWS_IO_UTIL_HPP
#include "../io_util.hpp"
#include "../code_util.hpp"
#include "../s.hpp"

#include <string>
#include <io.h>
#include <windows.h>
#include <filesystem>

namespace pensar_digital
{
    namespace cpplib
    {
        namespace fs = std::filesystem;
        using LINE_HANDLER = void(*)(const int64_t line_count, const S& line);

		// Get the full path of the executable.
		inline Result<S> get_exe_full_path()
		{
			// Get the full path of the executable.
			C buffer[MAX_PATH];
			if (GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0)
			{
				return Result<S>(W("Error getting executable path"));
			}
			// Convert to wide string.  
			S exe_path(buffer);
            Result<S> r = exe_path;
			return r;
		}


        inline S& read_file (const S& filename, S* s) 
        {
            #ifdef WIDE_CHAR
                std::string fname = to_string(filename);
                HANDLE file = CreateFileA(fname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            #else
                // Convert to filename to LPCSTR.
            std::wstring fname = to_wstring(filename);
                HANDLE file = CreateFileW(fname.c_str (), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            #endif
            if (file == INVALID_HANDLE_VALUE) {
                // handle error
            }
            DWORD file_size = GetFileSize(file, NULL);
            #pragma warning(push)
            #pragma warning(disable: 6387)
                HANDLE file_mapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
                if (file_mapping == NULL)
                {
                    log_and_throw("windows_read_file: Error mapping file: " + filename);
                }
            #pragma warning(pop)
            else
            {
                char* addr = static_cast<char*>(MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, file_size));
                if (addr == NULL) 
                {
                    log_and_throw("windows_read_file: Error mapping view of file: " + filename);
                }
                else
                {
                    #ifdef WIDE_CHAR
                        std::string* s2 = new std::string(addr, file_size);
                        *s = to_wstring(*s2);
                    #else
                        s = new S(addr, file_size);
                    #endif      

                    UnmapViewOfFile(addr);
                    if (file_mapping != 0) CloseHandle(file_mapping);
                    CloseHandle(file);
                    return *s;
                }
            }
        }

        template <typename T>
        void binary_write(std::ostream& os, const T& t, const size_t& size, const std::endian& byte_order = std::endian::native)
        {
            os.write((char*)&size, sizeof(size));
            os.write((char*)&t, size);
            
            /*
            if (byte_order == LITTLE_ENDIAN) {
				for (size_t i = 0; i < size; ++i) {
					os.put(static_cast<char>(t >> (i * 8)));
				}
			}
			else {
				for (size_t i = 0; i < size; ++i) {
					os.put(static_cast<char>(t >> ((size - i - 1) * 8)));
				}
			}
            */
		}
                      
        /*
        template <typename T>
        void binary_write(std::ostream& os, const T& t, const size_t& size)
        {
            os.write(reinterpret_cast<const char*>(&t), size);
        }
        */

        template <Sizeofable T>
        void binary_write (std::ostream& os, const T& t, const std::endian& byte_order = std::endian::native)
        {
            os.write ((char*)&t, sizeof(t));
		}

        // binary_write for std::basic_string.
        template <typename CharType>
		void binary_write (std::ostream& os, const std::basic_string<CharType>& s, const std::endian& byte_order = std::endian::native)
		{
			binary_write<size_t> (os, s.size(), byte_order);
			for (auto&& c : s) 
            {
				binary_write<CharType> (os, c, byte_order);
			}
		}  

        // binary_write for S.
        inline void binary_write (std::ostream& os, const S& s, const std::endian& byte_order = std::endian::native)
        {
            binary_write<S::value_type> (os, s, byte_order);
		}

        template <typename T>
        void binary_read (std::istream& is, T& t, const size_t& size, const std::endian& byte_order = std::endian::native)
		{
            is.read ((char*)(&size), sizeof(size));
            //str.resize(size);
            is.read ((char*)(&t), size);
            /*
            t = 0;
			if (bye_order == LITTLE_ENDIAN) 
            {
                for (size_t i = 0; i < size; ++i) 
                {
                    t |= static_cast<T>(static_cast<uint8_t>(is.get())) << (i * 8);
                }
            }
            else 
            {
                for (size_t i = 0; i < size; ++i) 
                {
                    t |= static_cast<T>(static_cast<uint8_t>(is.get())) << ((size - i - 1) * 8);
                }
            }
            */
        }

        template <Sizeofable T>
        void binary_read (std::istream& is, T& t, const std::endian& byte_order = std::endian::native)
        {
            is.read ((char*)(&t), sizeof(t));
        }

        inline void binary_read (std::istream& is, S& s, const std::endian& byte_order = std::endian::native)
		{
            size_t size;
            binary_read<size_t>(is, size, byte_order);
            s.clear();
            s.reserve(size);
            is.read((char*)(&s), size);
        }

        template <typename DataType = uint8_t, typename CharType = S::value_type>
        std::basic_string<CharType>& binary_to_string (const std::vector<DataType>& data, std::basic_string<CharType>& out)
        {
            out.clear();
            out.reserve(data.size() * sizeof(DataType) / sizeof(CharType));
            for (auto&& byte : data) {
                for (size_t i = 0; i < sizeof(DataType); ++i) {
                    out.push_back(static_cast<CharType>(byte >> (i * 8)));
                }
            }
            return out;
        }

        template <typename DataType = uint8_t, typename CharType = char>
        void string_to_binary (const std::basic_string<CharType>&in, std::vector<DataType>&out)
        {
			out.clear();
			out.reserve(in.size() * sizeof(CharType) / sizeof(DataType));
            for (size_t i = 0; i < in.size(); i += sizeof(DataType)) {
				DataType byte = 0;
                for (size_t j = 0; j < sizeof(DataType); ++j) {
					byte |= static_cast<DataType>(static_cast<uint8_t>(in[i + j])) << (j * 8);
				}
				out.push_back(byte);
			}
		} 

// ******************************
//         
        // Create an empty file.
        inline void create_empty_file(const C* file_full_path)
        {
            InFStream fs(file_full_path, std::ios::out);
            if (!fs.is_open())
            {
                S serror = W("create_file: It was not possible to create file.");
#ifdef WIDE_CHAR    
                std::string serr = to_string(serror);
                S path = file_full_path;
                std::string full_path = to_string(path);
                throw std::runtime_error(serr + full_path);
#else
                throw std::runtime_error(serror + file_full_path);
#endif
                fs.close();
            }
        }

        inline void handle_error(const char* msg)
        {
            perror(msg);
            exit(255);
        }

        inline uintmax_t read_file(const S& fname, LINE_HANDLER f)
        {
            InFStream fs(fname);
            S line;
            int line_count = 0;
            while (std::getline(fs, line))
            {
                f(line_count++, line);
            }
            return line_count;
        }

        inline bool file_exists(const std::string& filename)
        {
            struct stat file_info;
            int intStat;

            // Attempt to get the file attributes
            intStat = stat(filename.c_str(), &file_info);
            if (intStat == 0)
            {
                // We were able to get the file attributes
                // so the file obviously exists.
                return true;
            }
            else
            {
                // We were not able to get the file attributes.
                // This may mean that we don't have permission to
                // access the folder which contains this file. If you
                // need to do that level of checking, lookup the
                // return values of stat which will give you
                // more details on why stat failed.
                return false;
            }
        }

        // Constantes usadas em is_same ().
        inline const int SAME_NAME = 0x01;
        inline const int SAME_SIZE = 0x02;
        inline const int SAME_TIME = 0x04;
        inline const int SAME_ALL = SAME_NAME | SAME_SIZE | SAME_TIME;

#ifdef CODE_GEAR
#pragma package(smart_init)
        inline bool operator == (const ftime& left, const ftime& right)
        {
            return left.ft_year == right.ft_year
                && left.ft_month == right.ft_month
                && left.ft_day == right.ft_day
                && left.ft_hour == right.ft_hour
                && left.ft_min == right.ft_min
                && left.ft_tsec == right.ft_tsec;
        }

        inline bool operator != (const ftime& left, const ftime& right)
        {
            return !(left == right);
        }

        inline bool operator < (const ftime& left, const ftime& right)
        {
            bool less = left.ft_year < right.ft_year;
            if (!less)
            {
                less = left.ft_month < right.ft_month;
                if (!less)
                {
                    less = left.ft_day < right.ft_day;
                    if (!less)
                    {
                        less = left.ft_hour < right.ft_hour;
                        if (!less)
                        {
                            less = left.ft_min < right.ft_min;
                            if (!less)
                            {
                                less = left.ft_tsec < right.ft_tsec;
                            }
                        }
                    }
                }
            }
            return less;
        }

        inline ftime fileTimestamp(const char* gridFilePath)
        {
            FILE* fp;
            ftime ft;
            if ((fp = fopen(gridFilePath, "r")) != NULL)
            {
                getftime(fileno(fp), &ft);
                fclose(fp);
            }
            return ft;
        }

        inline std::string fileTimestampString(const char* gridFilePath)
        {
            FILE* fp;
            stringstream ss;
            if ((fp = fopen(gridFilePath, "r")) != NULL)
            {
                ftime ft;
                getftime(fileno(fp), &ft);
                ss << (ft.ft_year + 1980) << "-" << ft.ft_month << "-" << ft.ft_day << " " << ft.ft_hour << "h" << ft.ft_min;
                fclose(fp);
            }
            return ss.str();
        }
#endif


#ifdef WINDOWS
        inline __int64 get_file_size(const S& file_name)
        {
            HANDLE hFile;
#ifdef WIDE_CHAR
            hFile = CreateFile(file_name.c_str(),    // file to open
                GENERIC_READ,          // open for reading
                FILE_SHARE_READ,       // share for reading
                NULL,                  // default security
                OPEN_EXISTING,         // existing file only
                FILE_ATTRIBUTE_NORMAL, // normal file
                NULL);                 // no attr. template
#else
            hFile = CreateFile(to_wstring(file_name).c_str(),    // file to open
                GENERIC_READ,          // open for reading
                FILE_SHARE_READ,       // share for reading
                NULL,                  // default security
                OPEN_EXISTING,         // existing file only
                FILE_ATTRIBUTE_NORMAL, // normal file
                NULL);                 // no attr. template
#endif

            if (hFile == INVALID_HANDLE_VALUE)
            {
                return -1;
            }

            LARGE_INTEGER file_size;
            BOOL ok = GetFileSizeEx(hFile, &file_size);

            CloseHandle(hFile);
            return ok ? file_size.QuadPart : -1;
        }
#endif

// ******************************
        //extern bool file_exists (std::string filename);
    }   // namespace cpplib
}       // namespace pensar_digital
#endif // WINDOWS_IO_UTIL_HPP