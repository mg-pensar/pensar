#ifndef COUNTRY_HPP
#define COUNTRY_HPP

#include "object.hpp"
#include "s.hpp"
#include "generator.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        // alias for pensar_digital::cpplib namespace.
        namespace pd = pensar_digital::cpplib;
        /// <summary>
        /// Country contains ISO-638-3 information (2 and 3 letter codes, name, etc).
        /// See https://iso639-3.sil.org/code_tables/download_tables for more information and 
        /// download files.
        /// </summary>
        class Country : public Object
        {
        public:
            inline static const VersionPtr VERSION = pd::Version::get (1, 1, 1);
            /// \brief Default constructor.
            Country(Id aid = NULL_ID) : Object(aid == NULL_ID ? generator.get_id () : aid) {}
            /** Default destructor */
            virtual ~Country() {};
            /** Copy constructor
             *  \param other Object to copy from
             */
            Country(const Country& other) : Object (other) {}

            /** Assignment operator
             *  \param other Object to assign from
             *  \return A reference to this
             */
            Country& operator=(const Country& other) { Object::set_id(other.id()); }
        protected:
        private:
            static Generator<Country> generator;
        };
    }
}

#endif // COUNTRY_HPP
