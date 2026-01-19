// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"


#include "../code_util.hpp"

namespace pensar_digital
{
	namespace test = pensar_digital::unit_test;
	using namespace pensar_digital::unit_test;
	namespace cpplib
	{
		namespace code_util
		{
			using R = Result<int>;

			R f() { return 10; }
			R f1() { return W("err msg"); }

			TEST(Result, true)

				R r = f ();
				CHECK_EQ(R, r, 10, W("0. r != 10"));
				CHECK_EQ(Bool, r.mok, Bool::T, W("0. r != Bool::T"));

				R r1 = f1 ();
				CHECK (! r1, "r1 must be false.");
				CHECK_EQ(R::ErrorMessageType, r1.merror_message, W("err msg"), W("0. r1.merror_message != err msg"));
				CHECK_EQ(Bool, r1, Bool::F, W("1. r1.mok != Bool::F"));

			TEST_END(Result)

				TEST(ResultSerialization, true)
				using R = Result<int>;
				R r;
				// Starts a MemoryBuffer with a TriviallyCopiable object.
				MemoryBuffer mb((BytePtr)&r, sizeof(R));

				R r1(0, Bool::F, R::ErrorMessageType(W("err msg")));
				CHECK_NOT_EQ(R, r, r1, W("0. r == r1"));
				r1 = r;
				CHECK_EQ(R, r, r1, W("1. r != r1"));
			TEST_END(ResultSerialization)
		}
	}
}
