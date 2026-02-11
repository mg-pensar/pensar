// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include <catch2/catch_test_macros.hpp>
#include "test_helpers.hpp"

#include "../generator.hpp"
#include "../io_util.hpp"
#include "../concept.hpp"
#include "../binary_buffer.hpp"

#include <sstream>

namespace pensar_digital::cpplib
{
    using namespace test_helpers;

    TEST_CASE("Get", "[generator]")
    {
        Generator<int> g;
        Id expected = 1;
        INFO(W("0")); CHECK(g.get_id() == expected++);
        INFO(W("1")); CHECK(g.get_id() == expected++);

        Generator<> g2(1, 1, 2);
        expected = 3;
        INFO(W("2")); CHECK(g2.get_id() == expected);
        expected = 5;
        INFO(W("3")); CHECK(g2.get_id() == expected);
    }

    TEST_CASE("GetNext", "[generator]")
    {
        Generator<int> g;
        Id expected = 1;
        INFO(W("0")); CHECK(g.next() == expected);
        INFO(W("1")); CHECK(g.next() == expected);

        Generator<> g2(1, 1, 2);
        expected = 3;
        INFO(W("2")); CHECK(g2.next() == expected);
        INFO(W("3")); CHECK(g2.next() == expected);
    }

    TEST_CASE("GetCurrent", "[generator]")
    {
        Generator<int> g;
        Id expected = 0;
        INFO(W("0")); CHECK(g.current() == expected);
        INFO(W("1")); CHECK(g.get_id() == ++expected);
        INFO(W("2")); CHECK(g.current() == expected);

        Generator<> g2(1, 1, 2);
        expected = 1;
        INFO(W("4")); CHECK(g2.current() == expected);
        INFO(W("5")); CHECK(g2.get_id() == 3);
        INFO(W("6")); CHECK(g2.current() == 3);
    }

    TEST_CASE("SetValue", "[generator]")
    {
        Generator<int> g;
        g.set_value(10);
        Id expected = 10;
        INFO(W("0")); CHECK(g.current() == 10);
        INFO(W("1")); CHECK(g.get_id() == ++expected);
    }

    TEST_CASE("SetStep", "[generator]")
    {
        Generator<int> g(1, 0, 2);
        Id expected = 0;
        INFO(W("0")); CHECK(g.current() == expected);
        INFO(W("1")); CHECK(g.get_id() == 2);
        INFO(W("2")); CHECK(g.get_id() == 4);
    }

    TEST_CASE("GeneratorSerialization", "[generator]")
    {
        using G = Generator<int>;
        G g;
        G g2(1);
        INFO(W("0")); CHECK(g2 != g);
        BinaryBuffer bb;
        bb.write(g);
        g2.read(bb);
        INFO(W("1")); CHECK(g2 == g);
    }

    TEST_CASE("GeneratorFileBinaryStreaming", "[generator]")
    {
        Path test_dir = get_test_dir() / W("GeneratorFileBinaryStreaming/");
        test_dir.create_dir();
        Path out = test_dir / Path(W("file_binary_streaming_test.bin"));
        typedef Generator<Object> G;
        G g(1);
        BinaryBuffer bb;
        bb.write(g);
        bb.save_to_file(out.s());

        G g2(2);
        INFO(W("0")); CHECK(g != g2);

        // Load from file
        bb.load_from_file(out.s());
        g2.read(bb);
        INFO(W("1")); CHECK(g == g2);
    }

    TEST_CASE("GeneratorBinaryStreaming", "[generator]")
    {
        typedef Generator<Object> G;
        static_assert(Identifiable<G>);
        static_assert(Hashable<G>);
        static_assert(TriviallyCopyable<G::DataType>);
        static_assert(StandardLayout<G::DataType>);
        static_assert(TriviallyCopyable<G::DataType>);
        static_assert(TriviallyPersistable<G>);
        static_assert(HasStdLayoutTriviallyCopyableData<G>);

        G g(1);
        BinaryBuffer bb;
        bb.write(g);
        G g2;
        INFO(W("0")); CHECK(g2 != g);
        bb.read(g2);
        INFO("1"); CHECK(g2 == g);
    }
}
