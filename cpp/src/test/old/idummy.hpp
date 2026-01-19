// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef IDummy_HPP
#define IDummy_HPP

#include "constant.hpp"
#include "string_def.hpp"
#include "header_lib/json.hpp"
#include "version.hpp"
#include "iobject.hpp"

#include <iostream>
#include <memory> // for std::shared_ptr

namespace pensar_digital
{
    namespace cpplib
    {
        class IDummy;
        class IDummyRO;
        typedef std::shared_ptr<IDummyRO> IDummy_RO_Ptr;
        typedef std::shared_ptr<IDummy> IDummyPtr;

        // / \brief Base class for all Dummys. Read only interface of Dummy.
        class IDummyRO: public virtual IObjectRO
        {
        public:
            inline static const IVersionPtr VERSION = pd::versionf.get (1, 1, 1);

            virtual ~IDummyRO() noexcept = default;

            virtual String get_name() const noexcept = 0;
        };

        class IDummy : public virtual IDummyRO, public virtual IObject
        {
        public:
            inline static const IVersionPtr VERSION = pd::versionf.get (1, 1, 1);

            virtual ~IDummy() noexcept = default;
            
            virtual std::istream& read (std::istream& os, const IO_Mode& amode = TEXT, const ByteOrder& abyte_order = LITTLE_ENDIAN) = 0;
			virtual void set_name(const String& name) noexcept = 0;

            // Convertion from xml string.
            virtual void from_xml(const String& sxml) = 0;
        };
        //extern Factory<IDummy, Id, String> Dummy_factory;

    }  // namespace cpplib
}      // namespace pensar_digital
#endif // Dummy_HPP
