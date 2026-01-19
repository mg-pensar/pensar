// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../version.hpp"

namespace pensar_digital
{
	namespace test = pensar_digital::unit_test;
	using namespace pensar_digital::unit_test;
	namespace cpplib
	{
		TEST(Version, true)
			Version::Ptr v = Version::get(2, 3, 4);
		CHECK_EQ(Version, v->get_public(), 2, W("0. public    != 2"));
		CHECK_EQ(Version, v->get_protected(), 3, W("1. protected != 3"));
		CHECK_EQ(Version, v->get_private(), 4, W("2. private   != 4"));
		TEST_END(Version)

		TEST(VersionSerialization, true)
			Version::Ptr v = Version::get(2, 3, 4);
			MemoryBuffer::Ptr mb = v->bytes();
			Version::Ptr v1 = Version::get();
			CHECK_NOT_EQ(Version, *v, *v1, W("0. v == v1"));
			v1->assign(*mb);
			CHECK_EQ(Version, *v, *v1, W("1. v != v1"));
			CHECK_EQ(Version::Int, v->get_public(), 2, W("2. public    != 2"));
			CHECK_EQ(Version::Int, v->get_protected(), 3, W("3. protected != 3"));
			CHECK_EQ(Version::Int, v->get_private(), 4, W("4. private   != 4"));
		TEST_END(VersionSerialization)

	}
}
