#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include "macros.hpp"
#include "string_def.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace pensar_digital
{
    namespace cpplib
    {
        static inline bool log_on = true;
        static const std::ios::openmode DEFAULT_LOG_FILE_OPEN_MODE = (std::ios::out | std::ios::app);
        // Comment the line below to exclude logging from the compiled file.
        #define LOG_ON
            #define CODEGEAR_BUG

            // Comment the line below if you do not want the log to show where the message was originated.
            #define SHOW_LOCAL

            inline static const C* default_log_file_name() 
            {
                return W("C:\\out\\log.txt");
            }
            static bool initialized = false;
            static inline FStream log_stream; // The log stream.           
        #ifdef LOG_ON
                // The name was changed from log to cpplog to avoid conflict with math.h log function.
                #ifdef SHOW_LOCAL
                    #define cpplog if (pensar_digital::cpplib::log_on) pensar_digital::cpplib::log_stream << __LINE__
                #else
                    #define cpplog if (pensar_digital::cpplib::log_on) pensar_digital::cpplib::log_stream
                #endif
        
             
                // Opens the file if necessary and returns the file stream.
                static inline FStream& open_log_file (const C* full_path = default_log_file_name (), const std::ios::openmode mode = DEFAULT_LOG_FILE_OPEN_MODE)
                {
					if (!initialized)
					{
						log_stream.open(full_path, mode);
						if (!log_stream.is_open())
						{
							std::cerr << "Error opening log file " << full_path << std::endl;
						}
						initialized = true;
					}
                    else if (!log_stream.is_open())
                    {
                        log_stream.open (full_path, mode);
                        if (!log_stream.is_open())
                        {
                            std::cerr << "Error opening log file " << full_path << std::endl;
                        }
                    }
                    return log_stream;
                }

            
                static inline void enable_log  () {if ((!initialized) || (!log_stream.is_open ())) open_log_file (); log_on = true;}
				static inline void disable_log() { log_stream.flush(); log_stream.close(); log_on = false; initialized = false; }


                #define LOG(msg) cpplog << msg << std::endl; pensar_digital::cpplib::log_stream.flush();
                #define LOG_FLUSH pensar_digital::cpplib::log_stream.flush ();
            #else // LOG_ON
            #ifndef CODEGEAR_BUG
                #define cpplog \/\/
                #define LOG \/\/
                #define LOG_FLUSH \/\/
            #else
                #define cpplog if (0) out ()
                #define LOG if (0)
                #define LOG_FLUSH ;
            #endif
        #endif


            inline static void log_throw(const S& error_msg = W(""))
            {
                // Log error.
                LOG(error_msg);
                throw std::runtime_error(error_msg);
            }

    }   // namespace cpplib
}       // namespace pensar_digital

#endif // LOG_H_INCLUDED

