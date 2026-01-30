// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef FILE_HPP
#define FILE_HPP

#ifdef CODE_GEAR
#pragma hdrstop
#endif

#include <fstream>
#include <string>
#include <filesystem>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <functional>

#include "s.hpp"
#include "object.hpp"
#include "path.hpp"  
#include "generator.hpp"
#include "random_util.hpp"
#include "log.hpp"
#include "error.hpp"
#include "icu_util.hpp"
#include "constant.hpp"
#include "encoding.hpp"

#ifdef _WIN32
    #define STRERROR(buffer, buffer_size, error_code) strerror_s((buffer), (buffer_size), (error_code))
#else
    #define STRERROR(buffer, buffer_size, error_code) \
        do { \
            const char* _msg = strerror((error_code)); \
            std::strncpy((buffer), _msg ? _msg : "", (buffer_size)); \
            if ((buffer_size) > 0) (buffer)[(buffer_size) - 1] = 0; \
        } while (0)
#endif


namespace pensar_digital
{
    namespace cpplib
    {
        namespace pd = pensar_digital::cpplib;
        namespace fs = std::filesystem;

        class TextMode;
        class BinMode;
        class OpenMode
        {
            public:
            using Mode = std::ios::openmode;
            
            private:
                Mode _mode;
            public:
                OpenMode(const std::ios::openmode mode) : _mode(mode) {}

                inline bool is_bin_mode  (const Mode mode) const noexcept { return mode & std::ios::binary; }
                inline bool is_bin_mode  (                   ) const noexcept { return is_bin_mode (_mode)          ; }
                inline bool is_text_mode (const Mode mode) const noexcept { return ! is_bin_mode (mode)         ; }
                inline bool is_text_mode (                   ) const noexcept { return !is_bin_mode ()              ; }

                inline const Mode mode() const noexcept { return _mode; }

                inline S to_string () const noexcept 
                { 
                    S s;
					if (_mode & std::ios::in      ) s += W("in");
                    if (_mode & std::ios::out     ) s += (s.empty() ? W("out"   ) : W(" | out"   ));
					if (_mode & std::ios::binary  ) s += (s.empty() ? W("binary") : W(" | binary"));
					if (_mode & std::ios::app     ) s += (s.empty() ? W("app"   ) : W(" | app"   ));
					if (_mode & std::ios::ate     ) s += (s.empty() ? W("ate"   ) : W(" | ate"   ));
					if (_mode & std::ios::trunc   ) s += (s.empty() ? W("trunc" ) : W(" | trunc" ));
                    return s;
                }

                inline const C* c_str() const noexcept 
                {

					// Allocates a buffer with the size of to_string () + 1.
					C* buffer = new C[to_string().size() + 1];
                    return buffer; 
                }

                inline bool operator == (const OpenMode& mode) const noexcept { return _mode == mode._mode; }
                inline bool operator != (const OpenMode& mode) const noexcept { return _mode != mode._mode; }

                inline bool operator == (const Mode mode) const noexcept { return _mode == mode; }
                inline bool operator != (const Mode mode) const noexcept { return _mode != mode; }


                // implicit conversion to Mode
				inline operator Mode() const noexcept { return _mode; }

				// implicit conversion to string
				inline operator S() const noexcept { return to_string(); }

				// implicit conversion to C*
		        inline operator const C* () const noexcept { return c_str(); }
                
                // Open mode constants.
                static const BinMode BIN_READ;
                static const BinMode BIN_WRITE;
                static const BinMode BIN_APPEND;
                static const BinMode BIN_ATE;
                static const BinMode BIN_RW;
                static const BinMode BIN_RW_ATE;

                static const TextMode TEXT_READ;
                static const TextMode TEXT_WRITE;
                static const TextMode TEXT_APPEND;
                static const TextMode TEXT_ATE;
                static const TextMode TEXT_RW;
                static const TextMode TEXT_RWA; // It will create the file if it does not exist.
                static const TextMode TEXT_RW_ATE;

                static const BinMode  DEFAULT_BIN_MODE;
                static const TextMode DEFAULT_TEXT_MODE;
                static const OpenMode DEFAULT_OPEN_MODE;
        };


            class BinMode : public OpenMode
            {
                public:
                    BinMode (const OpenMode::Mode mode) : OpenMode (mode)
                    {
                        if (!is_bin_mode (mode))
                          throw Error(W("BinMode::BinMode (): Error: Invalid binary mode: ") + OpenMode(mode).to_string ());
                    }
            };

            class TextMode : public OpenMode
            {
                public:
                    TextMode (const Mode mode) : OpenMode(mode)
                    {
                        if (!is_text_mode(mode))
                        throw Error(W("TextMode::TextMode(): Error: Invalid text mode: ") + OpenMode(mode).to_string ());
                }
            };

            inline const BinMode OpenMode::BIN_READ = BinMode(std::ios::binary | std::ios::in);
            inline const BinMode OpenMode::BIN_WRITE = BinMode(std::ios::binary | std::ios::out);
            inline const BinMode OpenMode::BIN_APPEND = BinMode(std::ios::binary | std::ios::app);
            inline const BinMode OpenMode::BIN_ATE = BinMode(std::ios::binary | std::ios::ate);
            inline const BinMode OpenMode::BIN_RW = BinMode(std::ios::binary | std::ios::in | std::ios::out);
            inline const BinMode OpenMode::BIN_RW_ATE = BinMode(std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);

            inline const TextMode OpenMode::TEXT_READ = TextMode(std::ios::in);
            inline const TextMode OpenMode::TEXT_WRITE = TextMode(std::ios::out);
            inline const TextMode OpenMode::TEXT_APPEND = TextMode(std::ios::app);
            inline const TextMode OpenMode::TEXT_ATE = TextMode(std::ios::ate);
            inline const TextMode OpenMode::TEXT_RW = TextMode(std::ios::in | std::ios::out);
            inline const TextMode OpenMode::TEXT_RWA = TextMode(std::ios::in | std::ios::out | std::ios::app); 
            inline const TextMode OpenMode::TEXT_RW_ATE = TextMode(std::ios::in | std::ios::out | std::ios::ate);

            inline const BinMode  OpenMode::DEFAULT_BIN_MODE = BIN_RW;
            inline const TextMode OpenMode::DEFAULT_TEXT_MODE = TEXT_RWA;
            inline const OpenMode OpenMode::DEFAULT_OPEN_MODE = DEFAULT_BIN_MODE;

        /// \brief File class.
        ///
        class File : public Object
        {
            protected:
                OpenMode      _mode;     // Last mode used to open the file.
                Path          _fullpath; // Full path of the file. 
                std::fstream* _file;
            public:
                inline static const size_t MAX_BUFFER_SIZE = 1024 ^ 3; // 1 GB
                enum Seek { SET = SEEK_SET, CUR = SEEK_CUR, END = SEEK_END };

                inline S s_io_error_msg (const errno_t error) const
                {
                    C msg[icu_util::BUFFER_SIZE];
                    static_assert(icu_util::BUFFER_SIZE > 0, "BUFFER_SIZE must be greater than zero.");
                    STRERROR(msg, icu_util::BUFFER_SIZE, error);

                    #ifdef WIDE_CHAR
                        icu_util::utf8_char_ptr_to_wchar_t_ptr(msg, sys_err_msg);
                    #endif  
                    // null terminates the string.
                    msg[icu_util::BUFFER_SIZE - 1] = 0;
					return S(msg);
                }

                decltype(File::_file)& file_open (const Path& file_name, OpenMode mode)
                {
                    
                    #ifdef WIDE_CHAR
                        err = _wfopen_s(&_file, file_name.c_str(), mode.c_str());
                    #else
					    _file->open (_fullpath.str (), mode);
                    #endif

                    if (_file->fail ())
                        log_and_throw(S(W("File::file_open(): Error: Could not open file ")));
                    // Sets UTF-8.
                    //_file->imbue(std::locale("en_US.UTF-8"));
                    return _file;
                }

        public:
            
            inline File(const Path& full_path = W("."), const Id id = NULL_ID, const OpenMode mode = OpenMode::DEFAULT_BIN_MODE) :
                _fullpath(full_path), Object(id), _mode(mode)
            {
                _fullpath.create_dir (); // Create the directory if it does not exist.
				_file = new std::fstream ();
                file_open (_fullpath, mode);
            }

            inline virtual ~File() noexcept
            { 
                close();
                free(_file);
            }

            inline void close() noexcept 
            { 
                if (is_open())
                {
                    _file->close ();
                    if (_file->fail ())
                    {
                        log_and_throw (S("File::~File(): Error: Could not close file ") + _fullpath.str());
                    }
                }
            }

            inline bool exists() const { return _fullpath.exists(); }

            inline const Path& fullpath() const noexcept { return _fullpath; }

            inline bool is_open() const noexcept { return _file->is_open (); }
            
            inline bool is_bin_read  () const noexcept { return _mode == OpenMode::BIN_READ  ; }
            inline bool is_bin_write () const noexcept { return _mode == OpenMode::BIN_WRITE ; }
            inline bool is_bin_rw    () const noexcept { return _mode == OpenMode::BIN_RW_ATE; }

            inline bool is_text_read  () const noexcept { return _mode == OpenMode::TEXT_READ  ; }
            inline bool is_text_write () const noexcept { return _mode == OpenMode::TEXT_WRITE ; }
            inline bool is_text_rw    () const noexcept { return _mode == OpenMode::TEXT_RW_ATE; }

            inline bool eof  () const noexcept  { return _file->eof  () != 0; }
            inline bool fail () const noexcept  { return _file->fail () != 0; }

            // Returns the file path as a C-style string.
            // The caller is responsible for deleting the allocated buffer.
            inline const C* c_str() const noexcept 
            { 
                S s = _fullpath.str();

                // Allocates a buffer with the size of to_string () + 1.
                C* buffer = new C[s.size() + 1];
                std::strcpy(buffer, s.c_str());
                return buffer;
            }

            // Delete the file.
            inline bool remove() 
            { 
                if (is_open()) 
                    close();
               
                return fs::remove(_fullpath);
            }

                

            // Implements initialize method from Initializable concept.
            inline virtual bool initialize(const Path& afull_path = CURRENT_DIR, const Id id = NULL_ID, const OpenMode mode = OpenMode::DEFAULT_OPEN_MODE) noexcept
            {
                _fullpath = afull_path;
                _mode = mode;
                Object::set_id(id);
                return true;
            }

            //FilePtr clone() const  noexcept { return pd::clone<File>(*this, _fullpath, _mode, id()); }

            inline virtual OpenMode get_mode() const noexcept
            {
                return _mode;
            }

            inline virtual S debug_string() const noexcept
            {
                return Object::debug_string() + W(" path = ") + _fullpath.str();
            }

            inline void log_if_error (const errno_t error, const S& error_msg = W("")) const noexcept
			{
                if (error != 0)
                { 
                    // Log error.
                    S errmsg = error_msg + W(" in file ") + _fullpath.to_string() + W (" ") + s_io_error_msg (error);
                    LOG(errmsg);
                }
            }

            inline void log_and_throw_if_error(const errno_t error, const S& error_msg = W("")) const
            {
                if (error != 0)
                {
                    // Log error.
                    S errmsg = error_msg + W(" in file ") + _fullpath.to_string() + W(" ") + s_io_error_msg(error);
                    LOG(errmsg);
                    throw Error (errmsg);
                }
            }

            inline void move_cursor (FILE* f, const long pos, const Seek seek = Seek::SET) const
			{
				errno_t err = fseek (f, pos, seek);
                if (err != 0)
				{
					S error_msg = W("File::move_cursor(): Error: Could not move cursor to position.");
                    log_and_throw_if_error(err, error_msg);
				}
			}

            inline void move_to_start (FILE* f) const
            {
                    
                move_cursor (f, 0, Seek::SET);
            }

            inline void move_to_end (FILE* f) const
			{
				move_cursor (f, 0, Seek::END); 
            }  

            long position (FILE* f) const
			{
				long pos = ftell (f);
				if (pos == -1) 
				{
					S error_msg = W("File::position(): Error: Could not get file position ");
					log_and_throw_if_error(errno, error_msg);
				}
				return pos;
			}
            
            inline void set_binary_mode (FILE* f) noexcept
            {
                long pos = 0;
                if (is_open())
				{
                    // Gets the current position in the file.
                    long pos = position (f);
					close();
				}

                _mode = OpenMode::BIN_RW;
                open();
                if (pos > 0)
					move_cursor (f, pos);
            }

            inline void set_text_mode (FILE* f)
            {
                long pos = 0;
				if (is_open())
                {
					// Gets the current position in the file.
					long pos = position (f);
					close();
				}

                _mode = OpenMode::TEXT_RW;
                open();
                if (pos > 0)
                    move_cursor (f, pos);
            }

            // Opens the file if necessary and returns the file stream.
            inline decltype(_file)& open (const OpenMode& mode = OpenMode::DEFAULT_OPEN_MODE)
            {
                _mode = mode;
                if (!is_open())
                {
                    _file->open (_fullpath, _mode);
                }
                return _file;
            }
            
            inline size_t size() const noexcept
            {
				return fs::file_size(_fullpath);
            }

            inline size_t size (FILE* f) const noexcept
            {
                move_to_end (f);

                long size = ftell(f);
                if (size == -1)
                {
                    S error_msg = W("TextFile::read(): Error: Could not get file size ");
                    //log_and_throw_if_error(errno, error_msg);
					LOG(error_msg);
                }
                return size;
            }

            decltype(_file)& get_stream()
            {
                open();
                return _file;
            }

        };  // class File   

        class TextFile : public File
        {
            private:
            public:
                inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("TextFile"), 1, 1, 1 };
                inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
                TextFile(const Path& full_path, const S& content = EMPTY,
                    const TextMode mode = OpenMode::DEFAULT_TEXT_MODE, const Id id = NULL_ID) : File(full_path, id, mode)
                {
                    if (File::exists())
                    {
                        if (content != EMPTY)
                            append(content);
                        else
                        {
                            open (mode);
                            //move_to_start();
                        }
                    }
                    else // file does not exist.
                    {
							open (mode);
							//move_to_start();
                            write(content);
                            File::close();
                    }
                }

                //TextFile(const Path& full_path, const S& content = empty<C> (), const Id aid = NULL_ID) :
                //    TextFile(full_path, File::DEFAULT_MODE, content, id)
                //{
                //}

                inline virtual ~TextFile() = default;
    
                virtual bool get_char(C* c)
                {
					if (!is_open() || !_mode.is_text_mode())
						return false;

					*c = _file->get();
					return *c != EOF;
                }

                inline bool read_line(std::string& line)
                {
                    if (!is_open() || !_mode.is_text_mode ())
                        return false;

                    std::ifstream file_stream(_fullpath.str());
                    if (!file_stream.is_open())
                        return false;

                    if (std::getline(file_stream, line))
                        return true;

                    return false;
                }

                inline TextFile& write (const S& content, bool flushes = true)
                {
					// Writes content.
					if (! File::is_open()) 
						File::open ();
                    
                    // Writes content.
					_file->write(content.c_str(), content.size());
                    if (fail ())
					{
						log_and_throw (S(W("TextFile::write(): Error: Could not write to the file ")));
					}

					if (flushes)
					{
						// Flushes the file.
						_file->flush();
						if (fail())
						{
							log_and_throw(S(W("TextFile::write(): Error: Could not flush the file ")));
						}
					}
					
                    return *this;
                }
                inline TextFile& append (const S& content)
                {
                    // Appends content.
                    if (! File::is_open()) 
                        File::open ();

                    //move_to_end ();
				    write (content);

                    return *this;
                }

                inline S fullpath_to_string() const noexcept
                {
                    return File::_fullpath.to_string ();
                }

                // Implicit converts _fullpath to string.
                inline operator S() const noexcept
                {
                    return to_string();
                }

                // Reads the file content and returns it as a std::basic_string<C>.
                inline S read() 
                {
					if (! File::is_open()) 
						open (OpenMode::TEXT_READ);

					// Gets the file size in bytes.
                    size_t file_size = size ();

                    size_t count = file_size / sizeof(C);

					// Allocates a buffer to store the file content.
                    C* content = new C[count + 1];
                    
                    // Initializes the buffer with zeros.
                    memset(content, 0, count + 1);

					//move_to_start ();
					_file->seekg(0, std::ios::beg);
                    size_t read_count = _file->read(content, count).gcount();
                    if (read_count != count)
					{
                        // Frees the buffer.    
                        delete[] content;

						S error_msg = W("TextFile::read(): Error: Could not read the file ");
                        log_and_throw (error_msg);
					}

                    // Disables warning C6001: Using uninitialized memory 'content'.
                    #pragma warning(suppress:6001)
                    return S(content);
				}
        };
        
        // Random file file name generator.
        
        class RandomFileNameGenerator
        {
			private:

				inline static const size_t DEFAULT_LENGTH                =     8;
				inline static const S      DEFAULT_TEXT_FILE_EXTENSION   = W("txt");
                inline static const S      DEFAULT_BINARY_FILE_EXTENSION = W("bin");
                
                // Extension type. It can be fixed, random, none, numeric sequence, C char sequence, or a function.  
                enum class ExtensionType { FIXED, RANDOM, NONE, NUMERIC_SEQUENCE, CHAR_SEQUENCE, FUNCTION };
                inline static const ExtensionType DEFAULT_EXTENSION_TYPE = ExtensionType::FIXED;

                // Determines the function signature as a typedef. It is named custom_exension_generator and it should return a string and take no arguments.
                typedef std::function<S()> CUSTOM_EXT_FUNCTION;
                // Defines a NULL_CUSTOM_EXT_FUNCTION.  That should always return an empty string.
                inline static const CUSTOM_EXT_FUNCTION NULL_CUSTOM_EXT_FUNCTION = []() { return EMPTY; };

				// Extension type.
                ExtensionType mextension_type;

                // Extension.
                S mextension;

                inline static const size_t MAX_FULL_NAME_LENGTH = 255;
                

                // Numeric sequence.    
                typedef Generator<RandomFileNameGenerator, size_t> G;
                G generator;

                // Char sequence.
                // todo: CharGenerator<C> char_generator;

                // Custom extension function.
                CUSTOM_EXT_FUNCTION mcustom_ext_function;

                // Generates a random string of length len.
                inline static S random_string(const size_t len)
                {
					static const S CHARS = W("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
                    Random<size_t> r(0, CHARS.size() - 1);

					S s;
                    size_t i = len;
					s.reserve(i);
                    typename S::value_type c = CHARS[r()];
                    
                    size_t count = 1000; // Max count is 1000. If c is still a number after 1000 tries, then throws an exception.
                    // while c is a number, try again.
                    while (c >= '0' && c <= '9') 
                    {
                        c = CHARS[r()];
                        if (--count == 0)
                        {
                            // Logs error and throws an exception.
                            S error_msg = W("RandomFileNameGenerator::random_string(): Error: Could not generate a random string after 1000 tries.");
                            LOG(error_msg);
                            runtime_error(error_msg);
                        }
							
                    }
					s += c;
                    --i;
					while (i--)
						s += CHARS[r ()];

					return s;
				}

                inline S get_extension() const
                {
                    switch (mextension_type)
                    {
						case ExtensionType::FIXED:
							return mextension;
						case ExtensionType::RANDOM:
							return random_string(DEFAULT_LENGTH);
						case ExtensionType::NONE:
							return EMPTY;
						//case ExtensionType::NUMERIC_SEQUENCE:
							//return std::to_string((const G::IdType)generator.get_id());
						case ExtensionType::CHAR_SEQUENCE:
							return EMPTY; // todo: char_generator();
						case ExtensionType::FUNCTION:
							return mcustom_ext_function();
						default:
							return EMPTY;
					}
				}   

                    
            public:
            // Constructor.
                inline RandomFileNameGenerator (const ExtensionType ext_type = DEFAULT_EXTENSION_TYPE, const S& extension = DEFAULT_TEXT_FILE_EXTENSION, const CUSTOM_EXT_FUNCTION& custom_ext_function = NULL_CUSTOM_EXT_FUNCTION) :
                mextension_type(ext_type), mextension(extension), mcustom_ext_function(custom_ext_function)
            {
			}

            // Generates a random text file name. () operator.
            inline Path operator() (const S& name_prefix = W(""), const S& name_suffix = W(""), const S& extension = DEFAULT_TEXT_FILE_EXTENSION, const Path& path = TMP_DIR)
            {
                return path / (name_prefix + random_string(DEFAULT_LENGTH) + name_suffix + W(".") + get_extension());
            }
        };

        // TmpTextFile is a temporary text file.
        class TmpTextFile : public TextFile
        {
			public:
                inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("TmpTextFile"), 1, 1, 1 };
                inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
                inline TmpTextFile (const S& file_name = EMPTY, const S& content = EMPTY, const Id id = null_value<Id>()) :
                    TextFile(TMP_PATH / file_name, content, OpenMode::DEFAULT_TEXT_MODE, id)
                {
				}

                inline TmpTextFile (const C* file_name = EMPTY, const C* content = EMPTY, const Id id = null_value<Id>())
                    : TextFile (S(file_name), S(content), OpenMode::DEFAULT_TEXT_MODE, id) {}

                inline void log_error()
                {
                    LOG(W("Failed to remove temporary file ") + File::_fullpath.s ());
                }

                inline virtual ~TmpTextFile()
                {
                    // Removes the file. If it is open, it is closed first. If operation fails, a log message is generated.
                    if (!File::remove())
                    {
                        log_error();
                    }
				}
		};  // class TmpTextFile

        class BinaryFile : public File
        {
			private:
			public:
                inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("BinaryFile"), 1, 1, 1 };
                inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
        private:
                inline BinaryFile (const Path&                   full_path,
                            const OpenMode mode                     = OpenMode::DEFAULT_BIN_MODE,
                            const BytePtr                 data      = nullptr, 
                            const size_t                  size      = 0,
                            const Id                      id        = null_value<Id>()
                            ) : File (full_path, id, mode)
                {
					append (data, size);
				}

                inline BinaryFile (const Path& full_path, const BytePtr data = nullptr, const size_t size = 0, const Id id = null_value<Id>()) 
                    : BinaryFile(full_path, OpenMode::DEFAULT_BIN_MODE, data, size, id)
                {
				}

                inline virtual ~BinaryFile() { close(); }

                inline BinaryFile& append(const BytePtr data = nullptr, const size_t size = 0)
                {
                    static_assert (sizeof(char) == sizeof(std::byte), W("sizeof(char) != sizeof(Byte)"));
                    if ((data != nullptr) && (size > 0))
					{
						if (! is_open()) 
							open (OpenMode::BIN_RW_ATE);

						//move_to_end ();
						_file->write(reinterpret_cast<const char*>(data), size);
                        if (_file->fail ())
						{
							S error_msg = W("BinaryFile::append(): Error: Could not append data to the file ");
						 log_and_throw_if_error(errno, error_msg);
						}
					}
					return *this;
                }
        };
        // Streaming operators.
        extern InStream&  operator >> (InStream&  is,       File& file);
        extern OutStream& operator << (OutStream& os, const File& file);
    } // namespace cpplib
} // namespace pensar_digital.

#endif // FILE_HPP