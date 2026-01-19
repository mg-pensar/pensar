// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef DUMMY_FACTORY_HPP
#define DUMMY_FACTORY_HPP

#include "../factory.hpp"
#include "dummy.hpp"

namespace pensar_digital
{
	namespace cpplib
	{
		class DummyFactory2 : public Factory<Dummy, Id, S>
		{
		public:
			inline static const VersionPtr VERSION = pd::Version::get (1, 1, 1);
			DummyFactory2(const Id& aid = NULL_ID, const S& aname = W("")) : Factory<Dummy, Id, S>(3, 10, aid, aname) { };
			virtual ~DummyFactory2() { };
			using P = Factory<Dummy, Id, S>::P;
			virtual P get(const Id& aid = NULL_ID, const S& aname = W(""))
			{
				return Factory<Dummy, Id, S>::get(aid, aname);
			};

			P clone (const Dummy& adummy)
			{
				return get (adummy.id (), adummy.get_name ());
			};
			P clone(const DummyPtr& ptr) { return clone (*ptr);}
		};

		extern DummyFactory2 dummyf;
	}
}

#endif // DUMMY_FACTORY_HPP    

