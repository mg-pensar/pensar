// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"
#include "../constant.hpp"
#include "../string_def.hpp"
#include "../cs.hpp"
#include "../object.hpp"
#include "../binary_buffer.hpp"
#include "../concept.hpp"

#include <span>


namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        
        TEST(BinaryBuffer, true)
            BinaryBuffer buffer;

            auto o = pd::Object::get(42);
            buffer.write(o->as_bytes ());

            pd::Object::Ptr o2 = pd::Object::get();
            buffer.read(o2->wbytes());

            CHECK_EQ(Object, *o, *o2, W("0"));

        TEST_END(BinaryBuffer)

        TEST(BinaryBufferFileSerialization, true)
            // writes 1000 objects to the buffer.
                BinaryBuffer buffer;

                for (Id id = 1; id <= 1000; ++id) 
                {
                    auto obj = pd::Object::get(id);
                    buffer.write(obj->as_bytes());
                }

                // Save to file
                auto save_result = buffer.save_to_file("test_objects.bin");
                CHECK (save_result.has_value(), W("Failed to save to file"));

                // Clear buffer
                buffer.clear();

                // Load from file
                auto load_result = buffer.load_from_file("test_objects.bin");
                CHECK (load_result.has_value(), W("Failed to load from file"));
                CHECK_EQ(size_t, buffer.size(), 1000 * pd::Object::DATA_SIZE, W("Buffer size mismatch after loading from file"));

                // Read back objects and verify ids
                for (Id id = 1; id <= 1000; ++id) 
                {
                    auto obj = pd::Object::get();
                    buffer.read(obj->wbytes());
                    SStream err_msg;
                    err_msg << W("Object ID mismatch at index ") << id;   
                    CHECK_EQ(Id, obj->id(), id, err_msg.str());   
                }
        TEST_END(BinaryBufferFileSerialization)
    }
}
