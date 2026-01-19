// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"


#include "../memory_buffer.hpp"

namespace pensar_digital
{
    namespace test = pensar_digital::unit_test;
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        TEST(MemoryBuffer, true)
			MemoryBuffer mb(1024);
			CHECK(mb.wavailable() == 1024, W("1. Initial write available space should be 1024 bytes."));
			CHECK(mb.ravailable() == 0, W("2. Initial read available space should be 0 bytes."));

			// Write some data to the buffer.
			char sdata[] = "Hello World !";
			size_t size = strlen(sdata) + 1;
			const BytePtr data = reinterpret_cast<BytePtr>(sdata);
			mb.write(data, size); // +1 for null terminator
			CHECK(mb.wavailable() < 1024, W("3. Write available space should be less than 1024 after writing data."));
			CHECK(mb.ravailable() == strlen(sdata) + 1, W("4. Read available space should be equal to the size of written data."));
			// Read the data back.
			char buffer[50];
			mb.read(reinterpret_cast<BytePtr>(buffer), 0, size);
			CHECK(strcmp(buffer, sdata) == 0, W("5. Read data should match written data."));

			// Check offsets.
			CHECK(mb.woffset() == size, W("6. Write offset should match the size of written data."));
			CHECK(mb.roffset() == size, W("7. Read offset should match the size of read data."));

			// Check available space after reading.
			CHECK(mb.wavailable() == 1024 - size, W("8. Write available space should be 1024 minus the size of written data."));

			// Check data size.
			CHECK(mb.data_size() == size, W("9. Data size should match the size of written data."));

			// Check count of elements in the buffer.
			CHECK(mb.count() == 1, W("10. Count of elements in the buffer should be 1 after writing data."));

			// Check if the buffer size is correct.
			CHECK(mb.size() == 1024, W("11. Buffer size should be 1024 bytes."));

        TEST_END(MemoryBuffer)
    }
}
