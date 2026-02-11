// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "test_helpers.hpp"

#include "../s.hpp"
#include "../object.hpp"
#include "../io_util.hpp"
#include "../file.hpp"
#include "../binary_buffer.hpp"
#include "../test/dummy.hpp"
#include "../wire_double.hpp"
#include "../wire_int.hpp"
#include "../span_util.hpp"

namespace pensar_digital::cpplib
{
    using namespace test_helpers;

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

    TEST_CASE("ObjectClone", "[object]")
    {
        Object::FactoryType::P o = pd::Object::get(42);
        Object::FactoryType::P o1 = o->clone();
        INFO(W("0. o == o1 should be true"));
        CHECK(*o == *o1);

        Dummy::Ptr d = Dummy::get(42, W("d"));
        Dummy::Ptr d1 = d->clone();
        static_assert(OutputStreamable<Dummy>);
        INFO(W("1. d != d1"));
        CHECK(*d1 == *d);
    }

    TEST_CASE("ObjectSerialization", "[object]")
    {
        auto o = pd::Object::get(42);
        BinaryBuffer bb;
        o->write(bb);

        auto o1 = pd::Object::get();
        INFO(W("0. o == o1"));
        CHECK(*o != *o1);

        o1->read(bb);
        INFO(W("1. o != o1"));
        CHECK(*o == *o1);
    }

    TEST_CASE("ObjectBinaryFileStreaming", "[object]")
    {
        // Creates a vector with 1000 objects
        std::vector<Object::Ptr> objects(0);
        const Id N = 1000;
        for (Id i = 0; i < N; i++)
        {
            objects.emplace_back(pd::Object::get(i));
        }
        Path file = get_test_dir() / W("ObjectBinaryFileStreaming/");
        file.create_dir();
        file /= Path(W("test.bin"));
        BinaryBuffer bb;

        for (Id i = 0; i < N; i++)
        {
            objects[i]->write(bb);
        }

        bb.save_to_file(file.s());

        bb.clear();
        bb.load_from_file(file.s());

        for (Id i = 0; i < N; i++)
        {
            Object::Ptr o = pd::Object::get();
            o->read(bb);

            INFO(pd::to_string(i));
            CHECK(*o == *objects[i]);
        }
    }

    TEST_CASE("ObjectBinaryFileStreaming2", "[object]")
    {
        // Creates a vector with 1000 objects
        const Id N = 1000;
        std::vector<Object::DataType> v(0);
        v.reserve(N);
        for (Id i = 0; i < N; ++i)
            v.emplace_back(i);
        for (Id i = 0; i < N; ++i)
        {
            INFO(pd::to_string(i));
            CHECK(v[i].mid == i);
        }

        Path file = get_test_dir() / W("ObjectBinaryFileStreaming2/");
        file.create_dir();
        file /= Path(W("test.bin"));
        BinaryBuffer bb;
        {
            INFO(W("v.size()"));
            CHECK(v.size() == static_cast<size_t>(N));
        }
        {
            INFO(W("ccbytes(v).size()"));
            CHECK(ccbytes(v).size() == N * sizeof(Object::DataType));
        }
        bb.write(ccbytes(v));
        {
            INFO(W("bb.size() after write"));
            CHECK(bb.size() == ccbytes(v).size());
        }

        auto save_result = bb.save_to_file(file.s());
        INFO(W("save_to_file failed"));
        CHECK(save_result.has_value());

        bb.clear();
        auto load_result = bb.load_from_file(file.s());
        INFO(W("load_from_file failed"));
        CHECK(load_result.has_value());

        {
            INFO(W("unexpected file size"));
            CHECK(bb.size() == N * sizeof(Object::DataType));
        }
        std::vector<Object::DataType> v2(N);
        bb.read(ccbytes(v2));
        INFO(W("Data read from file should match original data"));
        CHECK(std::memcmp(v.data(), v2.data(), N * sizeof(Object::DataType)) == 0);
    }

    TEST_CASE("ObjectAssigns", "[object]")
    {
        AssignMoveBlob a{ { WireDouble(2.5), WireInt64(1) } };
        AssignMoveBlob b{ { WireDouble(9.5), WireInt64(7) } };

        assigns(b, a);

        INFO(W("0. assigns should copy int64"));
        CHECK(b.data()->a == 1);
        INFO(W("1. assigns should copy double"));
        CHECK(std::abs(static_cast<double>(b.data()->b) - 2.5) < DEFAULT_DELTA);
    }

    TEST_CASE("ObjectMoves", "[object]")
    {
        AssignMoveBlob a{ { WireDouble(4.25), WireInt64(3) } };
        AssignMoveBlob b{ { WireDouble(9.75), WireInt64(8) } };

        moves(b, a);

        INFO(W("0. moves should copy int64"));
        CHECK(b.data()->a == 3);
        INFO(W("1. moves should copy double"));
        CHECK(std::abs(static_cast<double>(b.data()->b) - 4.25) < DEFAULT_DELTA);
    }
} // namespace pensar_digital::cpplib
