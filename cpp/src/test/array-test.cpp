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
        const std::byte ZERO = std::byte{0};
        using DummyPadding = std::array<std::byte, 7>;
        constexpr DummyPadding empty_padding = {ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO};
        struct Dummy
        {
            
            int64_t i;          // 8‑aligned
            CS<0, 16> cs10;     // make CS size a multiple of 8 (change to 16)
            char c;
            DummyPadding pad; // explicit tail padding
            
            Dummy (int64_t i_ = 0, const C* str = W(""), char c_ = '\0', DummyPadding p = empty_padding) noexcept
                : i(i_), cs10(str), c(c_), pad(p)
            {
            }

            // add == operator using equal from equal.hpp so that it uses std::memcmp.
            bool operator==(const Dummy& other) const noexcept
            {
                return equal<Dummy>(*this, other);
			}

        };

        static_assert(StdLayoutTriviallyCopyable<Dummy>);

        TEST(CArray, true)
			using DummyArray = CArray<3, Dummy>;
    		static_assert(StdLayoutTriviallyCopyable<DummyArray>);

			const Dummy d0 = { 0, W("blah"), 'a' };
		    const Dummy d1 = { 1, W("bléh"), 'b' };
		    const Dummy d2 = { 2, W("blih"), 'c' };
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
            
            DummyArray d_array = {d0, d1, d2};
            CHECK(a.compare_range(d_array, 3), "raw pointer range compare must match");
            
            auto res = a.contains(d1);
            CHECK(res, "element must be found at index 1");
			CHECK(res.mresult == 1, "element must be at index 1");
            res = a.contains(Dummy{0, W("notfound"), 'x'});
			CHECK(!res, "element must not be found");

            std::array<Dummy, 3> std_array = {d0, d1, d2};
            CHECK(a.compare_range(std_array.data(), 3), "compare with std::array must match");
            static_assert(StdLayoutTriviallyCopyable<std::array<Dummy,3>>);
        TEST_END(CArray)
    }
}
