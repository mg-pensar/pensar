// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../s.hpp"
#include "../object.hpp"
#include "../io_util.hpp"
#include "../file.hpp"
#include "../test/dummy.hpp"

#include <memory>
#include <fstream>
#include <filesystem>

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
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
            MemoryBuffer::Ptr mb = *o;

			auto o1 = pd::Object::get();
			CHECK_NOT_EQ(Object, *o, *o1, W("0. o == o1"));

			o1->object_assign (*mb);
			CHECK_EQ(Object, *o, *o1, W("1. o != o1"));

            mb->reset_read_offset();
			auto o2 = pd::Object::get(*mb);
            CHECK_EQ(Object, *o, *o2, W("1. o != o1"));

         TEST_END(ObjectSerialization)
            
         TEST(ObjectBinaryFileStreaming, true)
			// Creates a vector with 1000 objects
			std::vector<Object::Ptr> objects;
		    const Id N = 1000; 
            for (Id i = 0; i < N; i++)
            {
				objects.push_back(pd::Object::get(i));
			}
            std::ofstream out (W ("c:\\tmp\\test\\ObjectBinaryFileStreaming\\test.bin"), std::ios::binary);

            for (Id i = 0; i < N; i++)
            {
				objects[i]->binary_write(out);
            }
			out.close();

            std::ifstream in (W("c:\\tmp\\test\\ObjectBinaryFileStreaming\\test.bin"), std::ios::binary);
            for (Id i = 0; i < N; i++)
            {
				Object::Ptr o = pd::Object::get();
				o->binary_read(in);
                Object::Ptr o1 = pd::Object::get(i);
                CHECK_EQ(Object, *o, *o1, pd::to_string(i));
            }
            TEST_END(ObjectBinaryFileStreaming)

            TEST(ObjectBinaryFileStreaming2, true)
                // Creates a vector with 1000 objects
                std::vector<Object::Ptr> objects;
                const Id N = 1000;
                for (Id i = 0; i < N; i++)
                {
                    objects.push_back(pd::Object::get(i));
                }
                std::ofstream out(W("c:\\tmp\\test\\ObjectBinaryFileStreaming\\test.bin"), std::ios::binary);

                for (Id i = 0; i < N; i++)
                {
                    MemoryBuffer::Ptr mb = objects[i]->bytes();
                    out.write((const char*)mb->data(), mb->size());
                }
                out.close();

                std::ifstream in(W("c:\\tmp\\test\\ObjectBinaryFileStreaming\\test.bin"), std::ios::binary);
                for (Id i = 0; i < N; i++)
                {
                    
                    MemoryBuffer mb(Object::SIZE);
                    mb.write (in, mb.size());
                    Object o(mb);
                    
                    Object::Ptr o1 = pd::Object::get(i);
                    CHECK_EQ(Object, o, *o1, pd::to_string(i));
                }
            TEST_END(ObjectBinaryFileStreaming2)
    }
}
