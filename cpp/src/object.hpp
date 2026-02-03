// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef OBJECT_HPP
#define OBJECT_HPP


#include <sstream>
#include <iostream>
#include <memory> // for std::shared_ptr
#include <concepts> // std::convertible_to
#include <string>
#include <typeinfo> // for typeid
#include <vector>
#include <span>
#include <cstddef>
#include <bit>
#include <cstring>
#include <bit> // for std::byteswap

#include "data.hpp"
#include "constant.hpp"
#include "s.hpp"
#include "clone_util.hpp"
#include "factory.hpp"
#include "log.hpp"
#include "string_def.hpp"
#include "equal.hpp"
#include "concept.hpp"
#include "class_info.hpp"
#include "binary_buffer.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        namespace pd = pensar_digital::cpplib;
 
        template <class T>
        requires StdLayoutTriviallyCopyableNoPadding<typename T::DataType> && requires(T t)
        {
            { t.data() } -> std::convertible_to<typename T::DataType*>;
        }
        T& assigns (T& l, const T& r) noexcept 
        { 
            std::memcpy  (l.data (), ((T&)r).data (), sizeof(typename T::DataType)); 
            return l; 
        }

        template <class T>
        requires StdLayoutTriviallyCopyableNoPadding<typename T::DataType> && requires(T t)
        {
            { t.data() } -> std::convertible_to<typename T::DataType*>;
        }
        T& moves (T& l, const T& r) noexcept
        { 
            std::memmove (l.data (), ((T&)r).data (), sizeof(typename T::DataType));
            return l; 
        }
    

        /// \remark To create a child class of Object:
        /// 1) Define a nested `Data` with standard layout, trivially copyable, and no padding.
        /// 2) Add `using DataType = Data;` and a `data()` accessor returning `Data*`.
        /// 3) Add `using Ptr = std::shared_ptr<Derived>;` for consistent smart pointers.
        /// 4) Call `initialize()` (or `assign()`) from constructors to set the data.
        /// 5) Override `info_ptr()` and keep `INFO` in sync for serialization.
        /// 6) If you need factory construction, add a `Factory` typedef and a static
        ///    factory instance like the base class does (or reuse `Object::FactoryType`).
        /// 7) If you customize binary serialization, override `binary_read()` and
        ///    `binary_write()` but keep the class/version checks consistent with `INFO`.
        ///
        /// Minimal example:
        /// \code
        /// class Person : public pd::Object
        /// {
        /// public:
        ///     using Ptr = std::shared_ptr<Person>;
        ///     struct Data : public pd::Object::DataType
        ///     {
        ///         pd::Id age;
        ///         Data(const pd::Id& id = pd::NULL_ID, const pd::Id& a = 0) noexcept
        ///             : pd::Object::DataType(id), age(a) {}
        ///     };
        ///     static_assert(pd::TriviallyCopyable<Data>, "Data must be trivially copyable");
        ///     static_assert(pd::StandardLayout<Data>, "Data must be standard layout");
        ///     static_assert(pd::NoPadding<Data>, "Data must have no padding");
        ///     using DataType = Data;
        ///     inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Person"), 1, 1, 1 };
        ///     const ClassInfo* info_ptr() const noexcept override { return &INFO; }
        ///     using Factory = pd::Factory<Person, DataType>;
        ///     inline static Factory mfactory = { 3, 10, DataType{} };
        ///
        /// private:
        ///     Data mdata;
        ///
        /// public:
        ///     const pd::Data* data() const noexcept override { return &mdata; }
        ///     Person(const Data& d = Data{}) noexcept { initialize(d); }
        ///
        /// };
        /// \endcode
        class Object
        {
		    public:
            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Object"), 1, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
            
            private:
            /// \brief Class Object::Data is compliant with the TriviallyCopyable concept. 
            /// \see https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable  
            struct Data : public pd::Data
            {                
                Id mid;         //!< Unique id (among objects of the same class).
                Data(const Id& id = NULL_ID) noexcept : mid(id) {}               
            };
            static_assert(TriviallyCopyable<Data>, "Data must be a trivially copyable type");
			static_assert(StandardLayout<Data>, "Data must be a standard layout type");
            static_assert(NoPadding<Data>, "Data must have unique object representations (no padding)");

            Data mdata; //!< Member variable mdata contains the object data.
        public:
			using Ptr = std::shared_ptr<Object>;
            inline const static Data NULL_DATA = { NULL_ID };
            using DataType = Data;
        private:
            using Factory = pd::Factory<Object, typename Object::DataType>;
		public:
            inline virtual const pd::Data* get_null_data() const noexcept { return (pd::Data*)(&NULL_DATA); }

            using FactoryType = Factory;
 
            inline static constexpr size_t DATA_SIZE = sizeof(mdata);
            inline static constexpr size_t      SIZE = DATA_SIZE + sizeof(ClassInfo);

            virtual const pd::Data* data() const noexcept { return &this->mdata; }
            virtual size_t data_size() const noexcept { return DATA_SIZE; }
            virtual size_t size     () const noexcept { return SIZE     ; }
        protected:
            /// Set id
            /// \param val New value to set
            void set_id(const Id& value) { mdata.mid = value; }
            Object& assign (const Object&  o) noexcept { std::memcpy ((void*)data(), o.data(), data_size()); return *this; }
            Object& assign (      Object&& o) noexcept { std::memmove((void*)data(), o.data(), data_size()); return *this; }
              
        private:
            inline static Factory mfactory = { 3, 10, NULL_DATA }; //!< Member variable "factory"

            // Set Factory as friend to allow access to private members.
            friend Factory;
        public:

            /// Default constructor.
            Object(const Data& data = NULL_DATA) noexcept
            {
                initialize(data);
            }

            /// Copy constructor
            /// \param other Object to copy from
            Object(const Object& o) { assign(o); }

            /// Move constructor
            Object(Object&& o) noexcept { assign(o); }

            /** Default destructor */
            virtual ~Object() = default;

            inline ConstByteSpan data_bytes () const noexcept 
            {
                return std::as_bytes(std::span{(BytePtr) data(), data_size() });
            }
            
            /// \brief Uses std::as_writable_bytes to get a span of writable bytes from the object.
            inline ByteSpan data_wbytes() noexcept
            {
                static_assert (sizeof(char) == sizeof(std::byte));

                auto byte_span = ByteSpan((std::byte*)(data()), data_size());
                return std::as_writable_bytes(byte_span);
            }

            inline virtual BinaryBuffer& write (BinaryBuffer& bb) const noexcept
            {
                // Add INFO.bytes and data_bytes together
                bb.write (INFO.bytes () );
                bb.write (std::span<const std::byte>((const std::byte*)&mdata, DATA_SIZE));
                return bb;                  
            }

            inline virtual BinaryBuffer& read (BinaryBuffer& bb) noexcept
            {
                // Read ClassInfo first and verify
                ClassInfo info;
                bb.read(std::span<std::byte>((std::byte*)&info, sizeof(ClassInfo)));
                if (info != INFO)
                {
                    LOG(W("ClassInfo mismatch in Object::read"));
                    return bb;
                }
                // Read data bytes
                auto byte_span = ByteSpan((std::byte*)(&mdata), DATA_SIZE);
               return bb.read(byte_span);
            }
                       
            inline virtual std::string sclass_name() const
            {
                std::string s = typeid(*this).name();
                if (s.starts_with ("class "))
                    s.erase(0, sizeof("class ") - 1);
                return s;
            }

            inline virtual S class_name() const
            {
                std::string s = sclass_name ();
#ifdef WIDE_CHAR
                return pd::to_wstring(s);
#else
                return s;
#endif  
            }

            // Clone method. 
            inline Object::Ptr clone() const noexcept { return pd::clone<Object>(*this, mdata.mid); }

            inline virtual bool equals(const Object& o) const noexcept
            {
                return equal<Object>(*this, o);
            }

            /// Access object id
            /// \return The current value of id
            ///
            inline virtual const Id id() const noexcept { return mdata.mid; };

            /// \brief Access hash
            ///
            /// \return  The current value of hash
            inline virtual const Hash hash() const noexcept { return this->id(); };

            // Implements initialize method from Initializable concept.
            inline virtual bool initialize(const Data& data) noexcept
            {
                mdata = data;

                return true;
            }

            /// Conversion to string.
            /// \return A string with the object id.
            inline S to_string() const noexcept { return pd::to_string(mdata.mid); }

            /// Implicit conversion to string.
            /// \return A string with the object id.
            inline operator S () const noexcept { return to_string(); }

            /// Debug string.
            /// \return A string with the object id.
            inline virtual S debug_string() const noexcept
            {
                SStream ss;
                ss << W("id = ") << Object::to_string();
                return ss.str();

            }

            /// Assignment operator
            /// \param o Object to assign from
            /// \return A reference to this
            inline Object& operator=(const Object& o) noexcept { return assign(o); }

            /// Move assignment operator
            inline Object& operator=(Object&& o) noexcept { return assign(o); }

            static inline Factory::P  get(const Data& data = NULL_DATA)
            {
                return mfactory.get(data);
            };

            static inline Factory::P  get(const Id& id)
            {
                return mfactory.get(Data(id));
            };

            inline virtual InStream& read(InStream& is) { return is >> mdata.mid; }
            inline virtual OutStream& write(OutStream& os) const { return os << id(); }

            inline InStream& operator >> (InStream& is)
            {
                return read(is);
            }

            inline OutStream& operator << (OutStream& os)
            {
                return write(os);
            }

            friend inline bool operator==(const Object& a, const Object& b) noexcept;
            friend inline bool operator!=(const Object& a, const Object& b) noexcept;
        };  //  class Object.

        inline InStream& operator >> (InStream& is, Object& o)
        {
            return o.read(is);
        }

        inline OutStream& operator << (OutStream& os, const Object& o)
        {
            return o.write(os);
        }

        bool operator==(const Object& a, const Object& b) noexcept
        {
            return a.equals(b);
        }
        
        bool operator!=(const Object& a, const Object& b) noexcept
        {
            return !a.equals(b);
        }


// Object
        // CloneableConcept any class T with a T::Ptr clone (); method. where T::Ptr is a shared_ptr <T>.
        /*
        template <typename T>
        concept CloneableConcept = requires (T t)
        {
            { t.clone() } -> std::convertible_to<Object*>;
        };
        */


        /*
        template<typename Container>
        std::istream& binary_read(Container& c, std::istream& is, const std::endian& byte_order)
        {
            // Assuming you have a way to determine the number of elements to read.
            // For example, reading the size first:
            size_t size = 0;
            is.read(reinterpret_cast<char*>(&size), sizeof(size));
            if (byte_order != std::endian::native) {
                size = std::bit_cast<size_t>(std::byteswap<uint64_t>(std::bit_cast<uint64_t>(size)));
            }

            // Clear the existing contents of the container
            c.clear();

            // Reserve capacity if the container supports it (like std::vector)
            auto* as_vector = dynamic_cast<std::vector<Object::Ptr>*>(&c);
            if (as_vector) {
                as_vector->reserve(size);
            }

            for (size_t i = 0; i < size; ++i) {
                // Create a new Object::Ptr, read its data, and add it to the container
                Object::Ptr obj = Object::get ();
                obj->binary_read(is, byte_order); // Assuming Object has a binary_read method
                c.insert(c.end(), obj); // Insert the object into the container
            }

            return is;
        }

        template<typename Container>
        std::ostream& binary_write(const Container& c, std::ostream& os, const std::endian& byte_order)
        {
            // Write the size of the container first
            size_t size = c.size();
            if (byte_order != std::endian::native) {
                size = std::bit_cast<size_t>(byteswap(std::bit_cast<uint64_t>(size)));
            }
            os.write(reinterpret_cast<const char*>(&size), sizeof(size));

            // Write each object in the container
            for (const auto& obj : c) {
                obj->binary_write(os, byte_order); // Assuming Object has a binary_write method
            }

            return os;
        }

        template<typename Container>
        std::ostream& binary_write(const Container& c, std::ostream& os, const std::endian& byte_order)
        {
			// Write the size of the container first
			size_t size = c.size();
			if (byte_order != std::endian::native) {
				size = std::bit_cast<size_t>(byteswap(std::bit_cast<uint64_t>(size)));
			}
			os.write(reinterpret_cast<const char*>(&size), sizeof(size));

			// Write each object in the container
			for (const auto& obj : c) {
				obj->binary_write(os, byte_order); // Assuming Object has a binary_write method
			}

			return os;
		}   
        */


        //***
        // Dependency class is a Constrainable class used to define dependencies between objects.
            /*template <Versionable MainClass, Versionable RequiredClass>
            class Dependency
            {
            private:
                Version::Int required_public_interface_version;
                Version::Int required_protected_interface_version;
                Version::Int required_private_interface_version;
            public:
                Dependency(Version v) noexcept
                    : required_public_interface_version(v.get_public ()),
                    required_protected_interface_version(v.get_protected ()),
                    required_private_interface_version(v.get_private ()) {}
                virtual ~Dependency() {}
                virtual bool ok() const noexcept = 0;

                // method to set the class dependency. 
            };*/
         } // namespace cpplib
} // namespace pensar_digital
#endif // OBJECT_HPP
