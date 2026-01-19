// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef ERROR_HPP_INCLUDED
#define ERROR_HPP_INCLUDED

#include "string_def.hpp"
#include "object.hpp"
#include "log.hpp"

#include <stdexcept>
#include <sstream>

namespace pensar_digital
{
    namespace cpplib
    {
        class Exception : public std::exception
		{
			public:
				Exception (const S& message): message (message) {}
				const S what_error () const noexcept 
                { 
                    #ifdef WIDE_CHAR
                    std::string s = what();
                    return to_wstring (s);
                    #else
                        return what ();
                    #endif  
                }
			private:
				S message;
		};

        #define INVALID_ARGUMENT(condition,message) \
        if (condition)\
        {\
            SStream ss;\
            ss << __FILE__ << '\t' << __LINE__ << '\t' << message << std::endl;\
            std::cerr << ss.str ();\
            std::cerr.flush ();\
            throw std::invalid_argument (ss.str ());\
        };
            
        #define WINVALID_ARGUMENT(condition,message) \
        if (condition)\
        {\
            std::wstringstream ss;\
            ss << WFILE << L'\t' << WLINE << L'\t' << message << std::endl;\
            std::wcerr << ss.str ();\
            std::wcerr.flush ();\
            throw std::invalid_argument (ss.str ());\
        };

        class Error : public Object
        {
            public:
                
                Error (const S& error_msg, const Id aid = 0): Object (aid), error_message (error_msg) {}

            const S& get_error_message () const { return error_message; } ;

            void set_error_message    (const S& error_msg) { error_message = error_msg; };
            void append_error_message (const S& error_msg) { error_message = error_message + error_msg; };

            virtual InStream& read (InStream& is)
            {
                Id id;
                is >> id >> error_message;
                set_id (id);
                return is;
            };

            virtual OutStream& write (OutStream& os) const
            {
                os << error_message << id ();
                return os;
            };

            // + operator overloading to append error messages.
            Error& operator+ (const S& msg)
            {
                append_error_message (msg);
                return *this;
            };

            private:
                S error_message;
        };

        inline OutStream& operator<< (OutStream& os, const Error& e) { return e.write (os); }
        inline InStream&  operator>> (InStream&  is,       Error& e) { return e.read  (is); }

        inline static void log_and_throw(const S& error_msg = W("")) 
        {
            // Log error.
            LOG(error_msg);
            throw Error(error_msg);
        }

    }
}

#endif // ERROR_HPP_INCLUDED;
