// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)
#include "../../../unit_test/src/test.hpp"

#include "../generator.hpp"
#include "../memory_buffer.hpp"

#include "../concept.hpp"

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
		    MemoryBuffer::Ptr mb = g.bytes ();
		    G g2 (1);
		    CHECK_NOT_EQ(G, g2, g, W("0"));
			g2.assign(*mb);
		    CHECK_EQ(G, g2, g, W("1"));
		TEST_END(GeneratorSerialization)
	
        TEST(GeneratorFileBinaryStreaming, true)
            std::ofstream out(W("c:\\tmp\\test\\GeneratorFileBinaryStreaming\\file_binary_streaming_test.bin"), std::ios::binary);
            typedef Generator<Object> G;
            typedef std::shared_ptr<G> GP;
            G g(1);
            g.binary_write(out);
            out.close ();
            out.flush ();

            std::ifstream  in(W("c:\\tmp\\test\\GeneratorFileBinaryStreaming\\file_binary_streaming_test.bin"), std::ios::binary);
            GP pg2 = G::get (1);
            pg2->binary_read  (in);
            in.close();
            G g3(3);
            CHECK_NOT_EQ(G, g3, g, W("0"));
            CHECK_EQ(G, *pg2, g, W("1"));
            CHECK_EQ(VersionInt, pg2->INFO.mpublic_interface_version, 2, W("2"));
            CHECK_EQ(VersionInt, pg2->INFO.mprotected_interface_version, 1, W("3"));
            CHECK_EQ(VersionInt, pg2->INFO.mprivate_interface_version, 1, W("4"));

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
            //Constructs a MemoryBuffer using constructor for StdLayoutTriviallyCopyableData types.
            MemoryBuffer::Ptr buffer_ptr = g.generator_bytes ();
 
            G g2;
            CHECK_NOT_EQ(G, g2, g, W("0"));
            g2.assign (*buffer_ptr);
            CHECK_EQ(G, g2, g, "1");
            
          TEST_END(GeneratorBinaryStreaming)
        
    }
}
