#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "object.hpp"
#include "s.hpp"
#include "generator.hpp"
#include "constant.hpp"

namespace pensar_digital
{
    namespace cpplib
    {
        /// <summary>
        /// Language contains ISO-638-3 information (2 and 3 letter codes, name, etc).
        /// See https://iso639-3.sil.org/code_tables/download_tables for more information and 
        /// download files.
        /// </summary>
        class Language : public virtual Object
        {
            public:
                /// \brief Default constructor.
                Language(Id aid = NULL_ID) : Object(aid == NULL_ID ? generator.get_id () : aid){}
                /** Default destructor */
                virtual ~Language() {};
                /** Copy constructor
                 *  \param other Object to copy from
                 */
                Language(const Language& other) = default;
                /** Assignment operator
                 *  \param other Object to assign from
                 *  \return A reference to this
                 */
                Language& operator=(const Language& other)
                {
                    if (this != &other)
                    {
                        Object::assign(other);
                    }
                    return *this;
                }
            private:
                static Generator<Language> generator;
        };
    }
}

#endif // LANGUAGE_HPP
