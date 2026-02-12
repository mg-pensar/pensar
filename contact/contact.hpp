// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_HPP
#define CONTACT_HPP

#include <cstdint>

namespace pensar_digital
{
    namespace cpplib
    {
        namespace contact
        {
            enum class ContactLocationType : uint8_t { Home, Work, Other };
            enum class ContactQualifier  : uint8_t { Business, Personal, Other };

            struct Contact
            {
            };
        }   // namespace contact
    }       // namespace cpplib
}           // namespace pensar_digital

#endif // CONTACT_HPP

