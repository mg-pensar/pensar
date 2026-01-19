#ifndef CLONE_UTIL_HPP
#define CLONE_UTIL_HPP

#include "factory.hpp"


#include <memory> // for std::shared_ptr

namespace pensar_digital
{
	namespace cpplib
	{
        template<class T, typename... Args> //requires FactoryCloneable<T, Args...>
        static typename NewFactory<T, Args...>::P clone(const T& other, const Args& ... args)
        {
            NewFactory<T, Args...> factory;
            return factory.get(args...);
        }

        // CloneableConcept any class T with a T::Ptr clone (); method. where T::Ptr is a shared_ptr <T>.
		template <typename T>
		concept CloneableConcept = requires (T t)
		{
			{ t.clone() } -> std::convertible_to<typename T::Ptr>;
		};

	} // namespace cpplib
} // namespace pensar_digital
#endif // CLONE_UTIL_HPP
