// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"

#include "../s.hpp"
#include "../byte_order.hpp"
#include "../array.hpp"

#include <span>


namespace pensar_digital
{
    using namespace pensar_digital::unit_test;
    namespace cpplib
    {
        
        template <typename U, typename T = U>
        void test_byte_order_conversion(Test& t, std::vector<U> a, std::vector<T> expected, uint_fast8_t count = 0)
        {
            std::vector<T> a6;
            for (auto x : a)
                a6.push_back (x);
            auto data = std::span{ a };
            // Get the bytes of the array as writable bytes.
            auto b = std::as_writable_bytes(data);
            auto size = sizeof(U);
            pd::convert<>(b, size, pd::native_byte_order, pd::big_address_8_byte_order);

            if (sizeof(U) > sizeof(std::byte))
            {
                if ((sizeof(T) == sizeof(U)))
                {
                    Array<T> c(expected.size());
                    std::memcpy(c.data(), b.data(), b.size());
                    std::vector<T> a3(c.data(), c.data() + c.size());

                    static_assert (IsLikeArrayOfV<pd::Array<T>, T>);
                    static_assert (IsLikeArrayOfV<std::vector<T>, T>);

                    t.check_equal_collection<std::vector<T>, std::vector<T>>(a3, expected, pd::to_string<decltype(count)>(count++) + W("."), sfile (), __LINE__);

                    pd::convert<>(b, size, pd::big_address_8_byte_order, pd::native_byte_order);
                    std::memcpy(c.data(), b.data(), b.size());
                    std::vector<T> a5(c.data(), c.data() + c.size());
                    t.check_equal_collection<std::vector<T>, std::vector<T>>(a5, a6, pd::to_string<decltype(count)>(count++) + W("."), sfile(), __LINE__);
                }
                else
                {
                    if (sizeof(T) > sizeof(U))
                    {
                        /*
                        size_t diff = sizeof(T) - sizeof(U);
                        Array<T> c(expected.size());
                        // Fill the array with the bytes.
                        for (auto i = 0; i < expected.size(); i++)
                        {
                            c[i] = 0x00000000;
                            std::byte* p = (std::byte*)(&c[i]) + i * sizeof(U);
                            *p = b[i];
                            *(p + 1) = b[i + 1];
                        }
                        t.check<pd::Array<T>, std::vector<T>>(c, expected, pd::to_string<decltype (count)>(count++) + ".", __FILE__, __LINE__);

                        Array<U> d(a.size());
                        pd::convert<>(b, size, pd::big_address_8_byte_order, pd::native_byte_order);
                        std::memcpy(d.data(), b.data(), b.size());
                        t.check<Array<U>, std::vector<T>>(d, a6, pd::to_string<decltype(count)>(count++) + ".", __FILE__, __LINE__);
                        */
                    }
                }
            }
        }

        TEST(ByteOrder1, true)
            std::vector<uint8_t> a1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
            std::vector<uint8_t> a2 = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
            test_byte_order_conversion<uint8_t > (*this, a1, a2, 0);

            std::vector<uint16_t> a3 = {0, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008};
            std::vector<uint16_t> expected = { 0, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 0x0800 };
			test_byte_order_conversion<uint16_t> (*this, a3, expected, 1);
            
            std::vector<uint32_t> a5 = { 0, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008 };
            std::vector<uint32_t> a6 = { 0, 0x01000000, 0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 0x08000000 };
            test_byte_order_conversion<uint32_t> (*this, a5, a6, 2);

            std::vector<uint64_t> a7 = { 0, 0x0000000000000001, 0x0000000000000002, 0x0000000000000003, 0x0000000000000004, 0x0000000000000005, 0x0000000000000006, 0x0000000000000007, 0x0000000000000008 };
            std::vector<uint64_t> a8 = { 0, 0x0100000000000000, 0x0200000000000000, 0x0300000000000000, 0x0400000000000000, 0x0500000000000000, 0x0600000000000000, 0x0700000000000000, 0x0800000000000000 };
			test_byte_order_conversion<uint64_t> (*this, a7, a8, 3);

            std::vector<int8_t>  a9 = { -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
            std::vector<int8_t> a10 = { -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2 ,3, 4, 5, 6, 7, 8 };
            test_byte_order_conversion<int8_t >(*this, a9, a10, 4);

            std::vector<int16_t > a11 = { -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
            std::vector<uint16_t> a12 = { 0xf8ff, 0xf9ff, 0xfaff, 0xfbff, 0xfcff, 0xfdff, 0xfeff, 0xffff, 0x0000, 0x0100, 0x0200 ,0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 0x0800 };
            test_byte_order_conversion<int16_t, uint16_t>(*this, a11, a12, 5);
            
            std::vector<int32_t > a13 = { -8, -7, -6, -5, -4, -3, -2, -1, 0x00000000, 0x00000001, 0x00000002 ,0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008 };
            std::vector<uint32_t> a14 = { 0xf8ffffff, 0xf9ffffff, 0xfaffffff, 0xfbffffff, 0xfcffffff, 0xfdffffff, 0xfeffffff, 0xffffffff, 0x0000000000000000, 0x01000000, 0x02000000 ,0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 0x08000000 };
            test_byte_order_conversion<int32_t, uint32_t>(*this, a13, a14, 6);

            std::vector<int64_t > a15 = { -8, -7, -6, -5, -4, -3, -2, -1, 0x00000000, 0x00000001, 0x00000002 ,0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008 };
            std::vector<uint64_t> a16 = { 0xf8ffffffffffffff, 0xf9ffffffffffffff, 0xfaffffffffffffff, 0xfbffffffffffffff, 0xfcffffffffffffff, 0xfdffffffffffffff, 0xfeffffffffffffff, 0xffffffffffffffff, 0x0000000000000000, 0x0100000000000000, 0x0200000000000000 ,0x0300000000000000, 0x0400000000000000, 0x0500000000000000, 0x0600000000000000, 0x0700000000000000, 0x0800000000000000 };
            test_byte_order_conversion<int64_t, uint64_t>(*this, a15, a16, 7);
            TEST_END(ByteOrder1)
    }
}
