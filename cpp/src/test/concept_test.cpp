// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>

#include "../s.hpp"
#include "../concept.hpp"
#include "../constraint.hpp"

namespace pensar_digital::cpplib
{
    template <typename... Args>
    class Checkable0
    {
        public:
            virtual bool ok(Args& ... args) const { return true; }
    };

    class Checkable1
    {
        public:
            virtual bool ok() const noexcept { return true; }
    };

    TEST_CASE("CheckableConcept", "[concept]")
    {
        static_assert(Checkable<Checkable0<int>, int>);
        static_assert(Checkable<Checkable0<S>, S>);
        static_assert(Checkable<Checkable0<>>);
        static_assert(Checkable<Checkable1>);
        static_assert(Checkable<StringConstraint, S>);
    }
}
