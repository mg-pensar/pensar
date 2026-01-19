// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef OBJ_MEMORY_BUFFER_HPP
#define OBJ_MEMORY_BUFFER_HPP

#include "constant.hpp"
#include "s.hpp"
#include "bytes_util.hpp"
#include "concept.hpp"
#include "memory_buffer.hpp"    

#include <memory> // for std::shared_ptr
#include <concepts>
#include <exception>
#include <string>
#include <typeinfo>      // for typeid
#include <string.h>
#include <utility>       // for std::move
#include <unordered_map> // for std::unordered_map
#include <functional>    // for std::hash
#include <typeindex>
#include <span>

namespace pd = pensar_digital::cpplib;

namespace pensar_digital
{
    namespace cpplib
    {
        /// <summary>
    /// Memory buffer to deal with binary data for BinaryIO types.
    /// </summary>
        template <BinaryIO T>
        class ObjMemoryBuffer : public MemoryBuffer
        {
        public:

            /// \brief Adds data to the buffer. 
            void add (const T& obj) noexcept
            {

				MemoryBuffer::Ptr mb = obj.bytes ();  

                // Check if the id already exists.
                auto offset_it = this->mindex.find(obj.id());

                // If obj is in the buffer.
                if (offset_it != this->mindex.end())
                {
                    this->mwrite_offset = offset_it->second;
                }
                //else
                //{
                //    auto id = obj.id();
                //    MemoryBuffer::write ((BytePtr) &id, sizeof(id));
               // }
				// Appends mb to the buffer.
				this->append (*mb);
                
            }

            template <typename... Args> requires FactoryConstructible<T, Args...>
            T::Factory::P CreateAndAddObj (Args&&... args) noexcept
            {
                typename T::Factory::P p = T::get(std::forward<Args>(args)...);
                add (*p);

                return p;
            }

            T::Factory::P CreateAndAddObj() noexcept
            {
                typename T::Factory::P p = T::get();
                T obj = *p;
                add(obj);

                return p;
            }

            void read_obj (typename T::Factory::P* p)
            {
                if (*p != nullptr)
                {
                    Id id = (*p)->id();
                    auto offset_it = this->mindex.find(id);
                    if (offset_it != this->mindex.end())
                    {
                        // Copy the data from the buffer.
                        MemoryBuffer::read((BytePtr)((*p)->data()), offset_it->second + sizeof(Id), (*p)->data_size()); //todo get correct size here
                        return;
                    }
                }
                else
                {
                    (*p) = T::get();
                }
                // Reads next data from the buffer.
                Id id = NULL_ID;
                MemoryBuffer::read((BytePtr)(&id), this->mread_offset, sizeof(Id));
                (*p)->set_id(id);
                MemoryBuffer::read((BytePtr)((*p)->data()), this->mread_offset, sizeof(T)); //todo use correct size.
                return;
            }


            /// \brief Gets data from the buffer into a factory constructed object. 
            template <typename... Args> requires FactoryConstructible<T, Args...>
            typename T::Factory::P read_obj (Args... args)
            {
                typename T::Factory::P p = T::get(args ...); // Create an object using its factory.

                read_obj (&p);

                return p;
            }

            /// \brief Gets data from the buffer into a factory constructed object. 
            typename T::Factory::P read_obj ()
            {
                typename T::Factory::P p = T::get(); // Create an object using its factory.

                read_obj (&p);

                return p;
            }
        }; // ObjMemoryBuffer
	} // cpplib
} // pensar_digital

#endif // OBJ_MEMORY_BUFFER_HPP