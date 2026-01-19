// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef MACOS_IO_UTIL_HPP
#define MACOS_IO_UTIL_HPP


#include <string>
#include <filesystem>
#include <sys/stat.h>
//#include <mach-o/dyld.h>
#include <unistd.h>
#include <limits.h>
#include <cstring>

// Declare _NSGetExecutablePath without including mach-o/dyld.h
extern "C" int _NSGetExecutablePath(char*, uint32_t*);

#include "../string_def.hpp"
#include "../s.hpp"
#include "../code_util.hpp"
#include "../error.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        namespace fs = std::filesystem;
        using LINE_HANDLER = void(*)(const int64_t line_count, const S& line);

		// Get the full path of the executable.
		inline Result<S> get_exe_full_path()
	    {			
			uint32_t size = 0;
            // Get the full path of the executable on macOS using _NSGetExecutablePath		
            _NSGetExecutablePath(nullptr, &size);
			
			char* buffer = new char[size];
			if (_NSGetExecutablePath(buffer, &size) != 0)
			{
				delete[] buffer;
				return Result<S>(W("Error getting executable path"));
			}
			
			// Convert to canonical path (resolve symlinks)
			char resolved_path[PATH_MAX];
			if (realpath(buffer, resolved_path) == nullptr)
			{
				delete[] buffer;
				return Result<S>(W("Error resolving executable path"));
			}
			
			S exe_path(resolved_path);
			delete[] buffer;
			Result<S> r = exe_path;
			return r;
		}


        inline S& read_file (const S& filename, S* s) 
        {
            // On macOS, use standard file I/O (std::ifstream)
            // Note: This is a simplified version; for memory-mapped files, use mmap
            InFStream fs(filename, std::ios::binary);
            if (!fs.is_open())
            {
                log_and_throw(W("macos_read_file: Error opening file: ") + filename);
            }
            
            // Get file size
            fs.seekg(0, std::ios::end);
            std::streamsize file_size = fs.tellg();
            fs.seekg(0, std::ios::beg);
            
            // Read file content
            *s = S();
            s->resize(static_cast<size_t>(file_size));
            fs.read(reinterpret_cast<char*>(s->data()), file_size);
            
            if (!fs)
            {
                log_and_throw(W("macos_read_file: Error reading file: ") + filename);
            }
            
            fs.close();
            return *s;
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
            OutFStream fs(file_full_path, std::ios::out);
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
            }
            fs.close();
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

        // Get file size on macOS
        inline int64_t get_file_size(const S& file_name)
        {
            struct stat file_info;
            int intStat;

            // Convert S to std::string for stat
            #ifdef WIDE_CHAR
                std::string fname = to_string(file_name);
            #else
                std::string fname = file_name;
            #endif

            intStat = stat(fname.c_str(), &file_info);
            if (intStat == 0)
            {
                return static_cast<int64_t>(file_info.st_size);
            }
            else
            {
                return -1;
            }
        }

// ******************************
        //extern bool file_exists (std::string filename);
    }   // namespace cpplib
}       // namespace pensar_digital
#endif // MACOS_IO_UTIL_HPP
