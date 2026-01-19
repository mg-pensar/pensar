// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "defines.hpp"

#ifdef CODE_GEAR
#pragma hdrstop
#endif

#include "string_def.hpp"
#include "s.hpp"
#include "io_util.hpp"

#include <iosfwd>
#include <stdexcept>
#include <stdlib.h>
#include <winnt.h>
#include <handleapi.h>
#include <cstdint>
#include <fileapi.h>
#include <minwindef.h>

//#include <ctwime>
#include <sys/stat.h>

//#include <fcntl.h>  // O_RDONLY

#ifndef _MSC_BUILD  
#include <unistd.h> // read
#endif

#include <string>


namespace pensar_digital
{
    namespace cpplib
    {

        // Create an empty file.
        void create_empty_file (const C* file_full_path)
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

        void handle_error(const char* msg)
        {
            perror(msg);
            exit(255);
        }

        uintmax_t read_file(const S& fname, LINE_HANDLER f)
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

        bool file_exists (const std::string& filename)
        {
            struct stat file_info;
            int intStat;

            // Attempt to get the file attributes
            intStat = stat (filename.c_str(), &file_info);
            if(intStat == 0)
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
        const int SAME_NAME = 0x01;
        const int SAME_SIZE = 0x02;
        const int SAME_TIME = 0x04;
        const int SAME_ALL  = SAME_NAME | SAME_SIZE | SAME_TIME;

        #ifdef CODE_GEAR
        #pragma package(smart_init)
        bool operator == (const ftime& left, const ftime& right)
        {
            return left.ft_year  == right.ft_year
                && left.ft_month == right.ft_month
                && left.ft_day   == right.ft_day
                && left.ft_hour  == right.ft_hour
                && left.ft_min   == right.ft_min
                && left.ft_tsec  == right.ft_tsec;
        }

        bool operator != (const ftime& left, const ftime& right)
        {
            return ! (left == right);
        }

        bool operator < (const ftime& left, const ftime& right)
        {
            bool less = left.ft_year  < right.ft_year;
            if (! less)
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

        ftime fileTimestamp (const char* gridFilePath)
        {
            FILE *fp;
            ftime ft;
            if ((fp = fopen(gridFilePath, "r")) != NULL)
            {
                getftime(fileno(fp), &ft);
                fclose (fp);
            }
            return ft;
        }

        string fileTimestampString (const char* gridFilePath)
        {
            FILE *fp;
            stringstream ss;
            if ((fp = fopen(gridFilePath, "r")) != NULL)
            {
                ftime ft;
                getftime(fileno(fp), &ft);
                ss << (ft.ft_year + 1980) << "-" << ft.ft_month << "-" << ft.ft_day << " " << ft.ft_hour << "h" << ft.ft_min;
                fclose (fp);
            }
            return ss.str();
        }
        #endif


        #ifdef WINDOWS
        __int64 get_file_size (const S& file_name)
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
            hFile = CreateFile(to_wstring(file_name).c_str (),    // file to open
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

            CloseHandle (hFile);
            return ok ? file_size.QuadPart : -1;
        }
        #endif

    }   // namespace cpplib
}       // namespace pensar_digital
