// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../factory.hpp"
#include "../s.hpp"
#include "../object.hpp"
#include <memory>

namespace pensar_digital::cpplib
{
    TEST_CASE("NewFactory", "[factory]")
    {
        std::vector<Object::Ptr> v;
        v.push_back(std::make_shared<Object>(1));
        INFO(W("0. use_count() should be 1 but is ") + pd::to_string((int)v[0].use_count()));
        CHECK(v[0].use_count() == 1);

        Object::Ptr ptr = v[0];
        INFO(W("1. use_count() should be 2 but is ") + pd::to_string((int)ptr.use_count()));
        CHECK(ptr.use_count() == 2);

        NewFactory<Object> factory;
        Object::Ptr o = factory.get();
        NewFactory<Object, pd::Id> factory1;
        Object::Ptr o1 = factory1.get(1);
        INFO(W("0. o != o1 should be true")); CHECK(*o != *o1);
        o.reset();
        INFO(W("1. managed object should have been deleted and assigned to nullptr."));
        CHECK(o.get() == nullptr);
    }

    TEST_CASE("SingletonFactory", "[factory]")
    {
        SingletonFactory<Object, pd::Id> factory(1);
        Object::Ptr o = factory.get(1);
        Object::Ptr o1 = factory.get(1);
        INFO(W("0. o == o1 should be true.")); CHECK(*o == *o1);
        o.reset();
        INFO(W("1. managed object should have been deleted and assigned to nullptr."));
        CHECK(o.get() == nullptr);
    }

    TEST_CASE("MockupFactory", "[factory]")
    {
        Object* mockup = new Object(1);
        MockupFactory<Object, pd::Id> factory(mockup);
        Object::Ptr o = factory.get(1);
        INFO(W("0. o == mockup should be true.")); CHECK(*o == *mockup);
    }

    TEST_CASE("PoolFactory", "[factory]")
    {
        PoolFactory<Object, Object::DataType> factory(3, 10, {1});
        {
            size_t count = factory.get_available_count();
            Object::Ptr ptr;
            for (size_t i = 0; i < count; i++)
            {
                ptr = factory.get({1});
                INFO(W("0."));
                CHECK(factory.get_available_count() == factory.get_pool_size() - i - 1);
            }
            INFO(W("0.1. available_count should be 0 but is ") + pd::to_string((int)factory.get_available_count()));
            CHECK(factory.get_available_count() == 0);
        }
        factory.reset(3, 10, 0);

        INFO(W("1. available_count should be 3 but is ") + pd::to_string((int)factory.get_available_count()));
        CHECK(factory.get_available_count() == 3);

        Object::Ptr o = factory.get({1});
        INFO(W("0. o->id () should be 1 but is ") + pd::to_string((int)o->id()));
        CHECK(o->id() == 1);
        INFO(W("2. available_count should be 2."));
        CHECK(factory.get_available_count() == 2);

        Object::Ptr o1 = factory.get({2});
        INFO(W("3. o1->id () should be 2 but is ") + pd::to_string((int)o->id()));
        CHECK(o1->id() == 2);
        INFO(W("4. available_count should be 1."));
        CHECK(factory.get_available_count() == 1);

        Object::Ptr o2 = factory.get({3});
        INFO(W("5. o2->id () should be 3 but is ") + pd::to_string((int)o->id()));
        CHECK(o2->id() == 3);
        INFO(W("6. available_count should be 0."));
        CHECK(factory.get_available_count() == 0);

        Object::Ptr o3 = factory.get({4});
        INFO(W("7. available_count should be 9 but is ") + pd::to_string((int)factory.get_available_count()));
        CHECK(factory.get_available_count() == 9);
        INFO(W("8. o3->id () should be 4 but is ") + pd::to_string((int)o->id()));
        CHECK(o3->id() == 4);
        INFO(W("9. *o != *o1 should be true.")); CHECK(*o != *o1);

        o.reset();
        INFO(W("10. managed object should have been deleted and assigned to nullptr."));
        CHECK(o.get() == nullptr);
    }
}
