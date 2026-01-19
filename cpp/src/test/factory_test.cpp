
// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"


#include "../factory.hpp"
#include "../s.hpp"
#include "../object.hpp"
#include <memory>

namespace pensar_digital
{
    using namespace pensar_digital::cpplib;
    namespace unit_test
    {
        TEST(NewFactory, true)
            {
                std::vector<Object::Ptr> v;
                v.push_back (std::make_shared<Object>(1));      
                CHECK(v[0].use_count() == 1, W("0. use_count() should be 1 but is ") + pd::to_string((int)v[0].use_count()));

                Object::Ptr ptr = v[0]; 
                CHECK(ptr.use_count() == 2, W("1. use_count() should be 2 but is ") + pd::to_string((int)ptr.use_count()));


                NewFactory <Object> factory;
                Object::Ptr o  = factory.get ();
                NewFactory <Object, pd::Id> factory1;
                Object::Ptr o1 = factory1.get (1);
                CHECK(*o != *o1, W("0. o != o1 should be true"));
                o.reset();
                CHECK(o.get () == nullptr, W("1. managed object should have been deleted and assigned to nullptr."));
        }
        TEST_END(NewFactory)
        
        TEST(SingletonFactory, true)
        {
				SingletonFactory <Object, pd::Id> factory (1);
				Object::Ptr o  = factory.get (1);
				Object::Ptr o1 = factory.get (1);
				CHECK(*o == *o1, W("0. o == o1 should be true."));
				o.reset();
				CHECK(o.get () == nullptr, W("1. managed object should have been deleted and assigned to nullptr."));
		}
        TEST_END(SingletonFactory)

        TEST(MockupFactory, true)
		{
            Object* mockup = new Object (1);
            MockupFactory<Object, pd::Id> factory (mockup);
            Object::Ptr o  = factory.get (1);
            CHECK(*o == *mockup, W("0. o == mockup should be true."));
        }
        TEST_END(MockupFactory)

        TEST(PoolFactory, true)
        {
			PoolFactory<Object, Object::DataType> factory (3, 10, {1});
            {
                size_t count = factory.get_available_count();
                Object::Ptr ptr;
                for (size_t i = 0; i < count; i++)
                {
                    ptr = factory.get({1});
                    CHECK(factory.get_available_count () == factory.get_pool_size () - i - 1, W("0."));
                }
                CHECK(factory.get_available_count() == 0, W("0.1. available_count should be 0 but is ") + pd::to_string((int)factory.get_available_count()));
            }
            factory.reset(3, 10, 0);

            CHECK(factory.get_available_count() == 3, W("1. available_count should be 3 but is ") + pd::to_string((int)factory.get_available_count ()));
			Object::Ptr o  = factory.get ({ 1 });
            CHECK(o->id () == 1, W("0. o->id () should be 1 but is ") + pd::to_string((int)o->id ()))
            CHECK(factory.get_available_count() == 2, W("2. available_count should be 2."));   
            Object::Ptr o1 = factory.get({ 2 });
            CHECK(o1->id() == 2, W("3. o1->id () should be 2 but is ") + pd::to_string((int)o->id()))
            CHECK(factory.get_available_count() == 1, W("4. available_count should be 1."));
            Object::Ptr o2 = factory.get({ 3 });
            CHECK(o2->id() == 3, W("5. o2->id () should be 3 but is ") + pd::to_string((int)o->id()))
            CHECK(factory.get_available_count() == 0, W("6. available_count should be 0."));
            Object::Ptr o3 = factory.get({ 4 });
            CHECK(factory.get_available_count() == 9, W("7. available_count should be 9 but is ") + pd::to_string((int)factory.get_available_count()));
            CHECK(o3->id() == 4, W("8. o3->id () should be 4 but is ") + pd::to_string((int)o->id()))
            CHECK(*o != *o1, W("9. *o != *o1 should be true."));
			o.reset();
			CHECK(o.get () == nullptr, W("10. managed object should have been deleted and assigned to nullptr."));
        }
        TEST_END(PoolFactory)
    }
}
