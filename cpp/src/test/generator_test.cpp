// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)
#include "../../../unit_test/src/test.hpp"

#include "../generator.hpp"
#include "../io_util.hpp"
#include "../concept.hpp"
#include "../binary_buffer.hpp"

#include <sstream>

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
    
        TEST(Get, true)
            Generator<int> g;
            Id expected = 1;
            CHECK_EQ(Id, g.get_id (), expected++, W("0"));
            CHECK_EQ(Id, g.get_id (), expected++, W("1"));

            Generator<> g2(1, 1, 2);
            expected = 3;
            CHECK_EQ(Id, g2.get_id (), expected, W("2"));
            expected = 5;
            CHECK_EQ(Id, g2.get_id (), expected, W("3"));
        TEST_END(Get)

        TEST(GetNext, true)
            Generator<int> g;
            Id expected = 1;
            CHECK_EQ(Id, g.next(), expected, W("0"));
            CHECK_EQ(Id, g.next(), expected, W("1"));

            Generator<> g2(1, 1, 2);
            expected = 3;
            CHECK_EQ(Id, g2.next(), expected, W("2"));
            CHECK_EQ(Id, g2.next(), expected, W("3"));
        TEST_END(GetNext)

        TEST(GetCurrent, true)
            Generator<int> g;
            Id expected = 0;
            CHECK_EQ(Id, g.current(),   expected, W("0"));
            CHECK_EQ(Id, g.get_id ()        , ++expected, W("1"));
            CHECK_EQ(Id, g.current(),   expected, W("2"));

            Generator<> g2(1, 1, 2);
            expected = 1;
            CHECK_EQ(Id, g2.current(), expected, W("4"));
            CHECK_EQ(Id, g2.get_id ()        ,        3, W("5"));
            CHECK_EQ(Id, g2.current(),        3, W("6"));
        TEST_END(GetCurrent)
        
        TEST(SetValue, true)
            Generator<int> g;
            g.set_value (10);
            Id expected = 10;
            CHECK_EQ(Id, g.current (),         10, W("0"));
            CHECK_EQ(Id, g.get_id ()        , ++expected, W("1"));
        TEST_END(SetValue)

		TEST(SetStep, true)
            Generator<int> g (1, 0, 2);
            Id expected = 0;
            CHECK_EQ(Id, g.current (),   expected, W("0"));
            CHECK_EQ(Id, g.get_id ()        ,          2, W("1"));
            CHECK_EQ(Id, g.get_id ()        ,          4, W("2"));
        TEST_END(SetStep)

	    TEST(GeneratorSerialization, true)
			using G = Generator<int>;
            G g;
		    G g2 (1);
		    CHECK_NOT_EQ(G, g2, g, W("0"));
		    BinaryBuffer bb;
		    bb.write(g);
		    g2.read(bb);
		    CHECK_EQ(G, g2, g, W("1"));
		TEST_END(GeneratorSerialization)
	
        TEST(GeneratorFileBinaryStreaming, true)
            Path test_dir = test::Test::test_dir () / W("GeneratorFileBinaryStreaming/");
            test_dir.create_dir ();
            Path out = test_dir / Path (W("file_binary_streaming_test.bin"));
            typedef Generator<Object> G;
            G g(1);
            BinaryBuffer bb;
            bb.write (g);
            bb.save_to_file (out.s ());

            G g2(2);
            CHECK_NOT_EQ(G, g, g2, W("0"));

            // Load from file
            bb.load_from_file (out.s ());
            g2.read (bb);
            CHECK_EQ(G, g, g2, W("1"));
        TEST_END(GeneratorFileBinaryStreaming)

        TEST(GeneratorBinaryStreaming, true)
            typedef Generator<Object> G;
            static_assert (Identifiable <G>);
            static_assert (Hashable<G>);
            static_assert (TriviallyCopyable <G::DataType>);
            static_assert (StandardLayout <G::DataType>);
            static_assert (TriviallyCopyable <G::DataType>);
            static_assert (TriviallyPersistable<G>);
			static_assert (HasStdLayoutTriviallyCopyableData<G>);

            
            //G::Factory::P p = G::get (1, 0, 1);
            G g(1);
            BinaryBuffer bb;
            bb.write (g);
            G g2;
            CHECK_NOT_EQ(G, g2, g, W("0"));
            bb.read (g2);
            CHECK_EQ(G, g2, g, "1");
            
          TEST_END(GeneratorBinaryStreaming)
        
    }
}


