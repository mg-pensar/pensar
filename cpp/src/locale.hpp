// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef LOCALE_HPP
#define LOCALE_HPP

#include "object.hpp"
#include "s.hpp"
#include "country.hpp"
#include "constant.hpp"
#include "language.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        class Locale : public virtual Object
        {
            public:
                 /** Default constructor */
                Locale(const Id aid = NULL_ID) {}

                /** Default destructor */
                virtual ~Locale() {}
                /** Copy constructor
                 *  \param other Object to copy from
                 */
                Locale(const Locale& other) {}
                /** Assignment operator
                 *  \param other Object to assign from
                 *  \return A reference to this
                 */
                Locale& operator=(const Locale& other) {}
                /** Access name
                 * \return The current value of name
                 */
                S get_name() { return W(""); }
                /** Access language
                 * \return The current value of language
                 */
                Language* get_language() { return language; }

                /** Access country
                 * \return The current value of country
                 */
                Country* get_country() { return country; }

            private:
                Language* language; //!< Member variable "language"
                Country* country; //!< Member variable "country"
        };
    } // namespace cpplib
} // namespace pensar_digital
#endif // LOCALE_HPP
