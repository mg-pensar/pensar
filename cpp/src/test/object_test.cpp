// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)


#include "../../../unit_test/src/test.hpp"

#include "../s.hpp"
#include "../object.hpp"
#include "../io_util.hpp"
#include "../file.hpp"
#include "../binary_buffer.hpp"
#include "../test/dummy.hpp"
#include "../wire_double.hpp"
#include "../wire_int.hpp"
#include "../span_util.hpp"

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        struct AssignMoveBlob
        {
            struct DataType
            {
                WireDouble b;
                WireInt64 a;
            };

            DataType mdata{};

            DataType* data() noexcept { return &mdata; }
            const DataType* data() const noexcept { return &mdata; }
        };

        static_assert(StdLayoutTriviallyCopyableNoPadding<AssignMoveBlob::DataType>);

        //static_assert(Assignable<Dummy>);

        TEST(ObjectClone, true)
            FactoryType::P o = pd::Object::get(42);
            FactoryType::P o1 = o->clone ();
            CHECK(*o == *o1,W("0. o == o1 should be true"));

            Dummy::Ptr d = Dummy::get (42,W("d"));
            Dummy::Ptr d1 = d->clone();
            static_assert(OutputStreamable<Dummy>);
            CHECK_EQ(Dummy, *d1, *d,W("1. d != d1"));
        TEST_END(ObjectClone)

        TEST(ObjectSerialization, true)
            auto o = pd::Object::get(42);
            BinaryBuffer bb;
            o->write (bb);

			auto o1 = pd::Object::get();
			CHECK_NOT_EQ(Object, *o, *o1, W("0. o == o1"));

			o1->read (bb);
			CHECK_EQ(Object, *o, *o1, W("1. o != o1"));

         TEST_END(ObjectSerialization)
            
         TEST(ObjectBinaryFileStreaming, true)
			// Creates a vector with 1000 objects
			std::vector<Object::Ptr> objects(0);
		    const Id N = 1000; 
            for (Id i = 0; i < N; i++)
            {
				objects.emplace_back (pd::Object::get(i));
			}
            Path file = test_dir () / W("ObjectBinaryFileStreaming/");
            file.create_dir ();
            file /= Path (W("test.bin"));
            BinaryBuffer bb;

            for (Id i = 0; i < N; i++)
            {
				objects[i]->write (bb);
            }

            bb.save_to_file(file.s ());

            bb.clear();
            bb.load_from_file(file.s ());

            for (Id i = 0; i < N; i++)
            {
				Object::Ptr o = pd::Object::get();
                o->read (bb);
                
                CHECK_EQ(Object, *o, *objects[i], pd::to_string(i));
            }
            TEST_END(ObjectBinaryFileStreaming)

            TEST(ObjectBinaryFileStreaming2, true)
                // Creates a vector with 1000 objects
                const Id N = 1000;
                std::vector<Object::DataType> v(0);
                v.reserve(N);
                for (Id i = 0; i < N; ++i)
                    v.emplace_back(i);
                for (Id i = 0; i < N; ++i)
                    CHECK_EQ(Id, v[i].mid, i, pd::to_string(i));
 
                Path file = test_dir () / W("ObjectBinaryFileStreaming2/");
                file.create_dir ();
                file /= Path (W("test.bin"));
                BinaryBuffer bb;
                CHECK_EQ(size_t, v.size(), static_cast<size_t>(N), W("v.size()"));
                CHECK_EQ(size_t, ccbytes(v).size(), N * sizeof(Object::DataType), W("ccbytes(v).size()"));
                bb.write(ccbytes(v));
                CHECK_EQ(size_t, bb.size(), ccbytes(v).size(), W("bb.size() after write"));

                auto save_result = bb.save_to_file(file.s());
                CHECK(save_result.has_value(), W("save_to_file failed"));

                bb.clear();
                auto load_result = bb.load_from_file(file.s());
                CHECK(load_result.has_value(), W("load_from_file failed"));

                CHECK_EQ(size_t, bb.size(), N * sizeof(Object::DataType), W("unexpected file size"));
                std::vector<Object::DataType> v2(N);
                bb.read (ccbytes(v2));
                CHECK (std::memcmp(v.data(), v2.data(), N * sizeof(Object::DataType)) == 0, W("Data read from file should match original data"));
            TEST_END(ObjectBinaryFileStreaming2)

            TEST(ObjectAssigns, true)
                AssignMoveBlob a{ { WireDouble(2.5), WireInt64(1) } };
                AssignMoveBlob b{ { WireDouble(9.5), WireInt64(7) } };

                assigns(b, a);

                CHECK_EQ(int64_t, b.data()->a, 1, W("0. assigns should copy int64"));
                CHECK(std::abs(static_cast<double>(b.data()->b) - 2.5) < Test::DEFAULT_DELTA, W("1. assigns should copy double"));
            TEST_END(ObjectAssigns)

            TEST(ObjectMoves, true)
                AssignMoveBlob a{ { WireDouble(4.25), WireInt64(3) } };
                AssignMoveBlob b{ { WireDouble(9.75), WireInt64(8) } };

                moves(b, a);

                CHECK_EQ(int64_t, b.data()->a, 3, W("0. moves should copy int64"));
                CHECK(std::abs(static_cast<double>(b.data()->b) - 4.25) < Test::DEFAULT_DELTA, W("1. moves should copy double"));
            TEST_END(ObjectMoves)
    }
}
