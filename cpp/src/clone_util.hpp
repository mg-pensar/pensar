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

        // CloneableConcept any class T with a clone() method returning a shared_ptr.
		template <typename T>
		concept CloneableConcept = requires (T t)
		{
			{ t.clone() };
			requires requires { typename decltype(t.clone())::element_type; }; // Must be a smart pointer
		};

	} // namespace cpplib
} // namespace pensar_digital
#endif // CLONE_UTIL_HPP
