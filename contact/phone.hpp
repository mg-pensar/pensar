// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef PHONE_HPP
#define PHONE_HPP

#include "../cpp/src/s.hpp"
#include "contact.hpp"

#include <sstream>

namespace pensar_digital
{
    namespace cpplib
    {
        namespace contact
        {
            namespace pd = pensar_digital::cpplib;

            struct PhoneNumber : public Contact
            {
                typedef CS<0,  4> CountryCode;
                typedef CS<0,  5> AreaCode; // Referred as Identification Code in E.164 standard. (https://en.wikipedia.org/wiki/E.164)
                typedef CS<0, 11> Number; // E.164 standard allows for a maximum of 15 digits. (https://en.wikipedia.org/wiki/E.164)

                CountryCode mcountry_code;
                AreaCode    mareacode;
                Number      mnumber;
                ContactQualifier mqualifier;
                PhoneNumber(const CountryCode& cc = EMPTY, const AreaCode& ac = EMPTY, const Number& n = EMPTY, ContactQualifier cq = ContactQualifier::Business)
                    : mcountry_code(cc), mareacode(ac), mnumber(n), mqualifier(cq) {}

                inline bool equal_country_code(const PhoneNumber& other) const noexcept
                {
                    return mcountry_code == other.mcountry_code;
                }

                inline bool equal_area_code(const PhoneNumber& other) const noexcept
                {
                    return mareacode == other.mareacode;
                }

                inline bool equal_number(const PhoneNumber& other) const noexcept
                {
                    return mnumber == other.mnumber;
                }

                inline bool operator==(const PhoneNumber& other) const noexcept
                {
                    return equal_country_code(other) && equal_area_code(other) && equal_number(other);
                }

                inline bool operator!=(const PhoneNumber& other) const noexcept
                {
                    return !(*this == other);
                }
            };
        }   // namespace contact
    }       // namespace cpplib
}           // namespace pensar_digital

#endif // PHONE_HPP
