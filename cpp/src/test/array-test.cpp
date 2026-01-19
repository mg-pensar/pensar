// author : Mauricio Gomes


#include "../../../unit_test/src/test.hpp"

#include "../array.hpp"
#include "../data.hpp"
#include "../cs.hpp"
#include "../s.hpp"
#include "../io_util.hpp"
#include "../file.hpp"
#include "../equal.hpp"

namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        struct Dummy
        {
            CS<0, 10> cs10;
            int64_t i;
            char c;

            // add == operator using equal from equal.hpp so that it uses std::memcmp.
            bool operator==(const Dummy& other) const noexcept
            {
                return equal<Dummy>(*this, other);
			}

        };

        TEST(CArray, true)
			using DummyArray = CArray<3, Dummy>;
    		static_assert(StdLayoutTriviallyCopyable<DummyArray>);

			const Dummy d0 = { W("blah"), 0,'a' };
		    const Dummy d1 = { W("bléh"), 1, 'b' };
		    const Dummy d2 = { W("blih"), 2, 'c' };
            CArray<3, Dummy> a = {d0, d1, d2};
		    CHECK_EQ(size_t, a.size(), 3, "size must be 3");

			DummyArray a2 = a; // Copy constructor
			//CHECK_EQ(DummyArray, a2, a, "arrays must match");
			CHECK(a == a2, "arrays must match");

            DummyArray a3 = {d0, d1};
			CHECK(a != a3, "arrays must not match");

            CHECK(a.compare_range(a2, 3), "full range compare must match");
            CHECK(a.compare_range(a2, 0, 2), "partial range compare must match");
            CHECK(!a.compare_range(a2, 1, 3), "out-of-bounds range compare must not match");
            // DummyArray d_array = {d0, d1, d2};
            // CHECK(a.compare_range(d_array, 3), "raw pointer range compare must match");
            // auto res = a.contains(d1);
            // CHECK(res, "element must be found at index 1");
			// CHECK(res.mresult == 1, "element must be at index 1");
            // res = a.contains(Dummy{W("notfound"), 0, 'x'});
			// CHECK(!res, "element must not be found");
        TEST_END(CArray)
    }
}
