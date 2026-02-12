// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef PERSON_HPP
#define PERSON_HPP

#include <string>
#include <utility>
#include <type_traits>
#include <array>
#include <cstring> // std::memcpy
#include <cstddef>

#include "../cpp/src/constant.hpp"
#include "../cpp/src/concept.hpp"
#include "../cpp/src/date.hpp"
#include "../cpp/src/object.hpp"
#include "../cpp/src/s.hpp"
#include "../cpp/src/io_util.hpp"
#include "../cpp/src/generator.hpp"
#include "../cpp/src/binary_buffer.hpp"
#include "contact.hpp"   // Contact, ContactLocationType, ContactQualifier.
#include "phone.hpp"	 // PhoneNumber
#include "e-mail.hpp"	 // Email

namespace pensar_digital
{
    namespace cpplib
    {
        namespace contact
        {
            static const size_t MAX_FIRST_NAME = 20;
            static const size_t MAX_MIDDLE_NAME = 20;
            static const size_t MAX_LAST_NAME = 20;
            static const size_t MAX_NAME = MAX_FIRST_NAME + MAX_MIDDLE_NAME + MAX_LAST_NAME + 2;


            class PersonName
            {
                public:
                    using First  = CS<0, MAX_FIRST_NAME >;
                    using Middle = CS<0, MAX_MIDDLE_NAME>;
                    using Last   = CS<0, MAX_LAST_NAME  >;

                    First  mfirst;
                    Middle mmiddle;
                    Last   mlast;

                    constexpr PersonName(const First& f = EMPTY, const Middle& m = EMPTY, const Last& l = EMPTY)
                        : mfirst(f), mmiddle(m), mlast(l) {}

                    const CS<0, MAX_NAME> name() const
                    {
                        S s = mfirst.to_string() + SPACE;
                        s += mmiddle.empty() ? mlast : mmiddle + mlast;
                        return s;
                    }

                    bool operator==(const PersonName& other) const
                    {
                        return mfirst == other.mfirst && mmiddle == other.mmiddle && mlast == other.mlast;
                    }

					bool operator== (const S& other) const
					{
						return name() == other;
					}

                    OutStream& write (OutStream& os) const
					{
						os << mfirst << SPACE;
						if (!mmiddle.empty())
							os << mmiddle << SPACE;
						return os << mlast;
					}
                    
					InStream& read (InStream& is)
                        {
                        is >> mfirst;
							if (is.peek() == SPACE)
								is.ignore();
							if (is.peek() != SPACE)
								is >> mmiddle;
							if (is.peek() == SPACE)
								is.ignore();
							return is >> mlast;
						}

            };

            inline static const PersonName null_person_name()
            {
                return { W(""), W(""), W("") };
            }

            // Make PersonName OutputStreamable.
            inline OutStream& operator<<(OutStream& os, const PersonName& name)
            {
                return name.write (os);
            }

            inline InStream& operator>>(InStream& is, PersonName& name)
			{
				return name.read (is);
			}

            // Person class. Inherits from pd::Object.
            // Contains fullname, date of birth, phone numbers and e-mails.
            // Uses BinaryBuffer for serialization (ClassInfo-based, no Version).
            class Person : public pd::Object
            {
                public:
                    static const size_t MAX_PHONE_NUMBERS = 5;
                    static const size_t MAX_EMAILS = 5;
                private:
                    /// \brief Person::Data — StdLayoutTriviallyCopyableNoPadding.
                    /// Fields ordered by sizeof descending for optimal packing.
                    struct Data : public pd::Data
                    {
                        Email memail1;              // 320 bytes
                        Email memail2;              // 320 bytes
                        Email memail3;              // 320 bytes
                        Email memail4;              // 320 bytes
                        Email memail5;              // 320 bytes
                        PersonName mname;           //  60 bytes
                        pd::Date mdate_of_birth;    //   4 bytes  (align 2)
                        PhoneNumber mphone1;        //  21 bytes
                        PhoneNumber mphone2;        //  21 bytes
                        PhoneNumber mphone3;        //  21 bytes
                        PhoneNumber mphone4;        //  21 bytes
                        PhoneNumber mphone5;        //  21 bytes
                        uint8_t _reserved = 0;      //   1 byte   (fills tail padding)

                        Data(const PersonName& name = null_person_name(), pd::Date dob = pd::NULL_DATE)
                            : mname(name), mdate_of_birth(dob) {}
                    };
                    static_assert(pd::TriviallyCopyable<Data>, "Person::Data must be trivially copyable");
                    static_assert(pd::StandardLayout<Data>,    "Person::Data must be standard layout");
                    static_assert(pd::NoPadding<Data>,         "Person::Data must have no padding");

                    Data mdata; //!< Member variable mdata contains the person-specific data.
                    inline static pd::Generator<Person> generator; //!< Unique id generator.

                public:
                    using Ptr = std::shared_ptr<Person>;
                    using DataType = Data;

                    inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Person"), 1, 1, 1 };
                    const ClassInfo* info_ptr() const noexcept override { return &INFO; }

                    inline static constexpr size_t DATA_SIZE = sizeof(Data);
                    inline static constexpr size_t SIZE = Object::SIZE + sizeof(ClassInfo) + DATA_SIZE;
                    inline const static Data NULL_DATA = { null_person_name(), pd::NULL_DATE };

                    Data*              data()       noexcept { return &mdata; }
                    const pd::Data*    data() const noexcept override { return &mdata; }
                    size_t        data_size() const noexcept override { return DATA_SIZE; }
                    size_t             size() const noexcept override { return SIZE; }
                    const pd::Data* get_null_data() const noexcept override { return reinterpret_cast<const pd::Data*>(&NULL_DATA); }

                    // --- Construction ---

                    Person(const Data& d = NULL_DATA, const pd::Id id = pd::NULL_ID)
                        : Object(id == pd::NULL_ID ? generator.get_id() : id), mdata(d) {}

                    // --- BinaryBuffer serialization (matches Generator pattern) ---

                    BinaryBuffer& write(BinaryBuffer& bb) const noexcept override
                    {
                        Object::write(bb);
                        bb.write(INFO.bytes());
                        bb.write(data_bytes());
                        return bb;
                    }

                    BinaryBuffer& read(BinaryBuffer& bb) noexcept override
                    {
                        Object::read(bb);
                        ClassInfo info;
                        bb.read(std::span<std::byte>(reinterpret_cast<std::byte*>(&info), sizeof(ClassInfo)));
                        if (info != INFO)
                        {
                            LOG(W("ClassInfo mismatch in Person::read"));
                            return bb;
                        }
                        return bb.read(data_wbytes());
                    }

                    // --- Accessors ---

                    const PersonName& name() const noexcept { return mdata.mname; }

                    void set_phone1(const PhoneNumber& phone) { mdata.mphone1 = phone; }
                    void set_phone2(const PhoneNumber& phone) { mdata.mphone2 = phone; }
                    void set_phone3(const PhoneNumber& phone) { mdata.mphone3 = phone; }
                    void set_phone4(const PhoneNumber& phone) { mdata.mphone4 = phone; }
                    void set_phone5(const PhoneNumber& phone) { mdata.mphone5 = phone; }

                    void set_email1(const Email& email) { mdata.memail1 = email; }
                    void set_email2(const Email& email) { mdata.memail2 = email; }
                    void set_email3(const Email& email) { mdata.memail3 = email; }
                    void set_email4(const Email& email) { mdata.memail4 = email; }
                    void set_email5(const Email& email) { mdata.memail5 = email; }

                    const PhoneNumber& phone1() const noexcept { return mdata.mphone1; }
                    const PhoneNumber& phone2() const noexcept { return mdata.mphone2; }
                    const PhoneNumber& phone3() const noexcept { return mdata.mphone3; }
                    const PhoneNumber& phone4() const noexcept { return mdata.mphone4; }
                    const PhoneNumber& phone5() const noexcept { return mdata.mphone5; }

                    const Email& email1() const noexcept { return mdata.memail1; }
                    const Email& email2() const noexcept { return mdata.memail2; }
                    const Email& email3() const noexcept { return mdata.memail3; }
                    const Email& email4() const noexcept { return mdata.memail4; }
                    const Email& email5() const noexcept { return mdata.memail5; }

                    // --- Equality ---

                    bool equals(const Object& other) const noexcept override
                    {
                        const Person* pother = dynamic_cast<const Person*>(&other);
                        if (pother == nullptr)
                            return false;
                        return (std::memcmp(&mdata, &pother->mdata, sizeof(mdata)) == 0);
                    }

                    // --- Text streaming ---

                    InStream& read(InStream& is) override { Object::read(is); return is >> mdata.mname; }
                    OutStream& write(OutStream& os) const override { Object::write(os); return os << mdata.mname; }

                    friend InStream&  operator>>(InStream&  is,       Person& p);
                    friend OutStream& operator<<(OutStream& os, const Person& p);
            }; // class Person

            inline InStream& operator>>(InStream& is, Person& p)
            {
                return p.read(is);
            }

            inline OutStream& operator<<(OutStream& os, const Person& p)
            {
                return p.write(os);
            }

        }       // namespace contact
    }           // namespace cpplib
}               // namespace pensar_digital

#endif // PERSON_HPP
