// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"
#include "../constant.hpp"
#include "../string_def.hpp"
#include "../cs.hpp"
#include "../object.hpp"
#include "../binary_buffer.hpp"
#include "../concept.hpp"

#include <span>


namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        
        TEST(BinaryBuffer, true)
            BinaryBuffer buffer;

            auto o = pd::Object::get(42);
            o->write (buffer);

            pd::Object::Ptr o2 = pd::Object::get();
            o2->read (buffer);

            CHECK_EQ(Object, *o, *o2, W("0"));

        TEST_END(BinaryBuffer)


    }
}
