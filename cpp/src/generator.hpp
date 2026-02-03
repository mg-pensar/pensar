// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef GENERATOR_HPP_INCLUDED
#define GENERATOR_HPP_INCLUDED

#include "object.hpp"
#include "constant.hpp"
#include "string_def.hpp"
#include "s.hpp"
#include "factory.hpp"
#include "error.hpp"

#include <memory>   // shared_ptr
#include <iosfwd>   // istream, ostream
#include <bit>      // endian
#include <concepts> // std::convertible_to

namespace pensar_digital
{
    namespace cpplib
    {
      /// Generator is meant to be used as unique identifier generator for other classes.
      ///
      /// Usually this is used as a static member for an entity class:
      /// \code {.cpp}
      ///  template <class String = std::wstring, class T = int> class Person : public Object
      ///  {
      ///    public:
      ///      Person(const String& aname = W("", const T aid = -1): Object((aid == NULL_ID) ? generator.get_id () : aid){};
      ///      virtual ~Person(){};
      ///
      ///    private:
      ///       static Generator<T> generator;
      ///       String name;
      ///
      ///  };
      /// \endcode
      template <typename Type = Id, typename T = Id>
      class Generator : public Object     
      {
        public:
            using UsedByType = Type;
            using IdType = T;

            using G = Generator<Type, T>; //!< Generator alias.
            using GeneratorPtr = std::shared_ptr<G>;
            using Factory = pd::Factory<G, T, T, T>;
            inline static Factory mfactory = { 3, 10, null_value<T>(), 0, 1}; //!< Member variable "factory"
            
       private:
          struct Data : public pd::Data
          {
              T minitial_value; //!< Generator initial_value.
              T mvalue; //!< Generator current value.
              T mstep; //!< Step to increment value.
              Data(T initial_value = 0, T step = 1) : minitial_value(initial_value), mvalue(initial_value), mstep(step) {}
          }; // struct Data
          Data mdata;
          static_assert(WireSafe<Data>, "Data must be a WireSafe type");
          public:

            inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Generator"), 2, 1, 1 };
            inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
            using DataType = Data;
            inline static constexpr size_t DATA_SIZE = sizeof(mdata);
            inline static constexpr size_t      SIZE = DATA_SIZE + sizeof(INFO) + Object::SIZE;

          inline const   pd::Data* generator_data     () const noexcept { return &mdata   ; }
          virtual const pd::Data* data() const noexcept { return &mdata; }
		  virtual size_t data_size() const noexcept { return DATA_SIZE; }
          
		  inline const BytePtr generator_data_bytes() const noexcept { return (BytePtr)&mdata; }
		  inline static const Data NULL_DATA = { null_value<T>(), null_value<T>() }; //!< Null data.
          inline virtual pd::Data* get_null_data() const noexcept { return (pd::Data*)(&NULL_DATA); }
            /// \brief Constructs a Generator.
            /// \param [in] initial_value Initial value for the generator, defaults to 0.
            /// \param [in] astep Step to be used when incrementing the generator, defaults to 1.
            Generator (T aid = null_value<T>(), T initial_value = 0, T step = 1) noexcept : Object(aid == null_value<T>() ? 0 : aid), mdata(initial_value, step) {};

            virtual ~Generator () = default;

            /// \brief Increments value and return the new value.
            /// \return The new value.
            inline virtual T get_id () { mdata.mvalue += mdata.mstep; return mdata.mvalue; }

            /// \brief Gets the next value without incrementing the current one.
            /// \return The next value.
            inline virtual const T next() { return (mdata.mvalue + mdata.mstep); }

            /// \brief Gets the current value.
            /// \return The current value.
            inline virtual const T current () const { return mdata.mvalue; }
            
            /// \brief Initialize a Generator.
            /// \param [in] initial_value Initial value for the generator, defaults to 0.
            /// \param [in] astep Step to be used when incrementing the generator, defaults to 1.
            virtual bool initialize (T aid = null_value<T>(), T initial_value = 0, T step = 1) noexcept
            {
                bool ok = Object::initialize(aid == null_value<T>() ? 0 : aid);
                mdata.minitial_value = initial_value;
                mdata.mvalue = initial_value;
                mdata.mstep = step;
                return ok;
            }
            
           inline virtual BinaryBuffer&write (BinaryBuffer& bb) const noexcept
            {
                // Add Object part
                Object::write(bb);
                // Add INFO.bytes and data_bytes together
                bb.write(INFO.bytes());
                bb.write(data_bytes());
                return bb;                  
            }

            inline virtual BinaryBuffer& read (BinaryBuffer& bb) noexcept
            {
                // Read Object part
                Object::read(bb);

                // Read ClassInfo first and verify
                ClassInfo info;
                bb.read(std::span<std::byte>((std::byte*)&info, sizeof(ClassInfo)));
                if (info != INFO)
                {
                    LOG(W("ClassInfo mismatch in Generator::read"));
                    return bb;
                }
                // Read data bytes
                return bb.read(data_wbytes());
            }
           
            /// \brief Create a new Generator using the factory method.

            /// \brief Set value. Next call to get will get value + 1.
            /// \param val New value to set
            inline virtual void set_value(T val) { mdata.mvalue = val; }

             void set_id (const T& aid) { Object::set_id (aid); }

           
            static inline Factory::P  get (T aid = null_value<T>(), T initial_value = 0, T step = 1) noexcept
            {
                return mfactory.get (aid, initial_value, step);
            };

            Factory::P clone()
            {
                return get (get_id (), mdata.minitial_value, mdata.mstep);
            };

            inline virtual InStream& read(InStream& is) 
            {
                Object::read(is);   
                is >> mdata.minitial_value; 
                is >> mdata.mstep;
                is >> mdata.mvalue;
                return is;
            }

            inline virtual OutStream& write (OutStream& os) const { Object::write(os); return os << mdata.minitial_value
                                                                                                 << mdata.mstep << W(" ")
                                                                                                 << mdata.mvalue; }
       }; // class Generator

      /// Makes Generator Streamable.
      template <class Type, typename T> OutStream& operator << (OutStream& os, const Generator<Type, T>& g) { return g.write (os); }
      template <class Type, typename T> InStream&  operator >> (InStream&  is,       Generator<Type, T>& g) { return g.read  (is); }

     } // namespace cpplib;

} // namespace pensar_digital
/// \example GeneratorTest.cpp

#endif // GENERATOR_HPP_INCLUDED
