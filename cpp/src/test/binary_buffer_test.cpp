// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../constant.hpp"
#include "../string_def.hpp"
#include "../cs.hpp"
#include "../object.hpp"
#include "../binary_buffer.hpp"
#include "../concept.hpp"

#include <span>

namespace pensar_digital::cpplib
{
    TEST_CASE("BinaryBuffer", "[binary_buffer]")
    {
        BinaryBuffer buffer;

        auto o = pd::Object::get(42);
        o->write(buffer);

        pd::Object::Ptr o2 = pd::Object::get();
        o2->read(buffer);

        INFO(W("0"));
        CHECK(*o == *o2);
    }
}
