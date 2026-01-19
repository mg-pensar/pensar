// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef DUMMY_HPP
#define DUMMY_HPP

#include "../../../unit_test/src/test.hpp"
#include "../s.hpp"
#include "../object.hpp"
#include "../clone_util.hpp"
#include "../factory.hpp"
#include "../string_types.hpp"
#include "../equal.hpp"

#include <memory>

namespace pensar_digital
{
    namespace cpplib
    {
        class Dummy;
#pragma warning( disable : 4250) // Disable warning C4250: inherits via dominance.
        typedef std::shared_ptr<Dummy> DummyPtr;


        /// <summary>
        /// Dummy class is streamable and comparable.
        /// </summary>
        class Dummy : public Object
        {
            private:
                struct Data : public pd::Data
                {
                    CS<0, 20> mname;
                    size_t data_size() const noexcept { return mname.length (); }  
                    Data(const S& name = W("")) noexcept : mname(name) {}

                };
                Data mdata; //!< Member variable mdata contains the object data.
            public:
				using Ptr = std::shared_ptr<Dummy>;
                    typedef pd::Factory<Dummy, Id, S> Factory;
                    inline static Factory factory = {3, 10, NULL_ID, EMPTY};
                    inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Dummy"), 1, 1, 1 };
                    inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

                    virtual const pd::Data* data() const noexcept { return &mdata; }
                    virtual size_t data_size() const noexcept { return mdata.data_size (); }

                    Dummy(const Id& id = NULL_ID, const S& name = EMPTY) : Object(id), mdata(name) {}
                    Dummy(const Dummy& d) : Object(d), mdata (d.mdata.mname) { }
                    Dummy(Dummy&& d) noexcept : Object(d) { assign (d); }
                
                    //Dummy& assign(const Dummy&  o) noexcept { Object::assign(o); std::memcpy(&mdata, &(o.mdata), sizeof(mdata)); }
                    //Dummy& assign(const Dummy&& o) noexcept { Object::assign(o); std::memmove(&mdata, &(o.mdata), sizeof(mdata)); }

                    inline static Factory::P get(const Id& aid = NULL_ID, const S& aname = EMPTY)
                    {
                        return factory.get(aid, aname);
                    }

                    inline Factory::P clone () const noexcept
                    {
                        return get(id  (), get_name ());
                    }

                    inline Factory::P clone(const DummyPtr& ptr) { return ptr->clone (); }
                    Dummy& operator = (const Dummy& d) noexcept { assign(d); return *this; }
                    Dummy& operator = (Dummy&& d) noexcept { assign(d); return *this; }
 
                    virtual ~Dummy() {}
                    //Dummy& assign(const Dummy& d) noexcept { Object::assign(d); mdata = d.mdata; return *this; }

                    virtual S class_name() const noexcept { return Object::class_name (); }

                    virtual void set_id(const Id& aid) noexcept { Object::set_id(aid); }
            
                    // Implements initialize method from Initializable concept.
                    virtual bool initialize(const Id& aid = NULL_ID, const S& name = W("")) noexcept
                    {
                        Object::set_id(aid);
                        mdata.mname = name;
                        return true;
                    }

                    virtual std::istream& binary_read(std::istream& is, const std::endian& byte_order = std::endian::native)
                    {
                        Object::binary_read(is, byte_order);
                        INFO.test_class_name_and_version(is, byte_order);
                        is.read((char*)data(), data_size());
                        return is;
                    }

                    virtual std::ostream& binary_write(std::ostream& os, const std::endian& byte_order = std::endian::native) const
                    {
                        Object::binary_write (os, byte_order);
                        INFO.binary_write(os, byte_order);
                        os.write((const char*)(&mdata), DATA_SIZE);
                        return os;
                    }
                

                virtual S get_name() const noexcept { return mdata.mname; }
                void   set_name(const S& aname) noexcept { mdata.mname = aname; }
            
                virtual S to_string() const noexcept { return Object::to_string () + W(" ") + mdata.mname.to_string (); }

                /// Implicit conversion to string.
                /// \return A string with the object id.
                operator S () const noexcept { return to_string(); }

                /// Debug string.
                /// \return A string with the object id.
                virtual S debug_string() const noexcept
                {
				    return Object::debug_string() + W(" name = ") + mdata.mname;
                }

                virtual bool equals(const Object& o) const noexcept
                {
                    return (equal<const Dummy::Data>(this->mdata, dynamic_cast<const Dummy&>(o).mdata) && Object::equals(o));
                }

                // Hide base overload so it is not viable
                bool operator==(const Object&) const = delete;
                bool operator!=(const Object&) const = delete;

                bool operator==(const Dummy& d) const noexcept
                {
                    return equals(d); // or ::pensar_digital::cpplib::operator==(*this, d)
                }

                bool operator!=(const Dummy& d) const noexcept
                {
                    return !(*this == d);
                }
/*            inline virtual bool equals(const Object& o) const noexcept
            {
                // Asserts the cast of const Object& o to const Dummy& o is valid.
                static_assert (std::is_base_of<Object, Dummy>::value, "Dummy must be derived from Object.");
                
                // Casts o to const Dummy& and compares with this.
                const Dummy& od = dynamic_cast<const Dummy&>(o);
                return equal<const Dummy&>(static_cast<const Dummy&>(*this), od );
            }
*/
        };

        inline InStream&  operator >> (InStream&  is,       Dummy& o) { return o.read  (is); }
        inline OutStream& operator << (OutStream& os, const Dummy& o) { return o.write (os); }

    }  // namespace cpplib
}      // namespace pensar_digital
#endif // DUMMY_HPP