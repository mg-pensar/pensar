#ifndef DATA_HPP
#define DATA_HPP

#include <type_traits>
#include <cstring>
#include <new>
#include <cstddef>
#include <utility>   // for std::declval
#include <iostream>
#include <iomanip>
#include <string>
#include "concept.hpp" // for StdLayoutTriviallyCopyable

namespace pensar_digital
{
    namespace cpplib
    {
        // Helper: conditional tail padding
        template <std::size_t N>
        struct TailPad { char pad[N]; };

        template <>
        struct TailPad<0> {}; // empty if no padding needed

        // Runtime dump functions (only available in debug mode)
#ifndef NDEBUG
        // Helper to dump a single member
        template<typename T, typename MemberType>
        void dump_member_info(const std::string& struct_name,
            const std::string& member_name,
            std::size_t offset,
            std::size_t size) {
            std::cout << "    " << member_name << ": offset=" << offset
                << ", size=" << size << ", ends=" << (offset + size) << "\n";
        }

        // Internal implementation for dumping struct layout
        template<typename T>
        class StructLayoutDumper {
            std::string struct_name_;
            std::size_t prev_end_ = 0;
            std::string prev_name_;
            bool first_ = true;

        public:
            explicit StructLayoutDumper(const std::string& name) : struct_name_(name) {
                std::cout << "========================================\n";
                std::cout << "Structure Layout: " << struct_name_ << "\n";
                std::cout << "  Size: " << sizeof(T) << " bytes\n";
                std::cout << "  Alignment: " << alignof(T) << " bytes\n";
                std::cout << "========================================\n";
            }

            void add_member(const std::string& member_name,
                std::size_t offset,
                std::size_t size) {
                if (!first_) {
                    // Check internal padding
                    std::size_t padding = offset - prev_end_;
                    if (padding > 0) {
                        std::cout << "  >>> INTERNAL PADDING: " << padding
                            << " bytes between " << prev_name_
                            << " and " << member_name << " <<<\n";
                    }
                }

                std::cout << "  " << member_name << ": offset=" << offset
                    << ", size=" << size << ", ends=" << (offset + size) << "\n";

                prev_end_ = offset + size;
                prev_name_ = member_name;
                first_ = false;
            }

            ~StructLayoutDumper() {
                // Check tail padding
                std::size_t struct_size = sizeof(T);
                std::size_t struct_align = alignof(T);
                std::size_t tail_pad = (struct_align - (prev_end_ % struct_align)) % struct_align;
                std::size_t tail_delta = struct_size - prev_end_;

                if (tail_pad > 0 || tail_delta > 0) {
                    std::cout << "  >>> TAIL PADDING: alignment-based=" << tail_pad
                        << " bytes, total delta=" << tail_delta << " bytes <<<\n";
                }
                std::cout << "========================================\n\n";
            }
        };
#endif // NDEBUG

    } // namespace cpplib
} // namespace pensar_digital

// ============================================================================
// COMPILE-TIME VERIFICATION MACROS
// ============================================================================

// Helper to get member offset - uses offsetof for standard layout types
#define MEMBER_OFFSET(T, member) offsetof(T, member)

// Size of a member - create a temporary instance to get the size
#define MEMBER_SIZE(T, member) sizeof(std::declval<T>().member)

// End offset of a member within T
#define END_OF_MEMBER(T, member) \
    (MEMBER_OFFSET(T, member) + MEMBER_SIZE(T, member))

// True tail padding needed due to alignof(T)
#define TAIL_PAD_ONLY_N(T, last) \
    ((alignof(T) - (END_OF_MEMBER(T, last) % alignof(T))) % alignof(T))

// Total size difference (includes tail padding)
#define TAIL_DELTA_N(T, last) \
    (sizeof(T) - END_OF_MEMBER(T, last))

// Internal padding check (between consecutive members)
#define ASSERT_NO_INTERNAL_PADDING(T, m1, m2) \
    static_assert(MEMBER_OFFSET(T, m2) == END_OF_MEMBER(T, m1), \
                  "Internal padding detected between " #m1 " and " #m2 ".")

// Alignment-based tail padding must be zero
#define ASSERT_NO_TAIL_PADDING(T, last) \
    static_assert(TAIL_PAD_ONLY_N(T, last) == 0, \
                  "Tail padding detected after " #last ".")

// ============================================================================
// AUTOMATIC SIZE CALCULATION FROM MEMBERS
// ============================================================================

// Calculate sizeof for a single member - use simple approach
#define SIZEOF_MEMBER(T, member) sizeof(std::declval<T>().member)

// Helper macros for iterating over members to sum their sizes
#define FOR_EACH_SIZEOF_1(T, m1) \
    SIZEOF_MEMBER(T, m1)
#define FOR_EACH_SIZEOF_2(T, m1, m2) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2)
#define FOR_EACH_SIZEOF_3(T, m1, m2, m3) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3)
#define FOR_EACH_SIZEOF_4(T, m1, m2, m3, m4) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4)
#define FOR_EACH_SIZEOF_5(T, m1, m2, m3, m4, m5) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4) + SIZEOF_MEMBER(T, m5)
#define FOR_EACH_SIZEOF_6(T, m1, m2, m3, m4, m5, m6) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4) + SIZEOF_MEMBER(T, m5) + SIZEOF_MEMBER(T, m6)
#define FOR_EACH_SIZEOF_7(T, m1, m2, m3, m4, m5, m6, m7) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4) + SIZEOF_MEMBER(T, m5) + SIZEOF_MEMBER(T, m6) + SIZEOF_MEMBER(T, m7)
#define FOR_EACH_SIZEOF_8(T, m1, m2, m3, m4, m5, m6, m7, m8) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4) + SIZEOF_MEMBER(T, m5) + SIZEOF_MEMBER(T, m6) + SIZEOF_MEMBER(T, m7) + SIZEOF_MEMBER(T, m8)
#define FOR_EACH_SIZEOF_9(T, m1, m2, m3, m4, m5, m6, m7, m8, m9) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4) + SIZEOF_MEMBER(T, m5) + SIZEOF_MEMBER(T, m6) + SIZEOF_MEMBER(T, m7) + SIZEOF_MEMBER(T, m8) + SIZEOF_MEMBER(T, m9)
#define FOR_EACH_SIZEOF_10(T, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10) \
    SIZEOF_MEMBER(T, m1) + SIZEOF_MEMBER(T, m2) + SIZEOF_MEMBER(T, m3) + SIZEOF_MEMBER(T, m4) + SIZEOF_MEMBER(T, m5) + SIZEOF_MEMBER(T, m6) + SIZEOF_MEMBER(T, m7) + SIZEOF_MEMBER(T, m8) + SIZEOF_MEMBER(T, m9) + SIZEOF_MEMBER(T, m10)

// Sum up all member sizes
#define EXPECTED_SIZE_OF_MEMBERS(T, ...) \
    (CONCAT(FOR_EACH_SIZEOF_, COUNT_ARGS(__VA_ARGS__))(T, __VA_ARGS__))

// ============================================================================
// HELPER MACROS
// ============================================================================

// Get the last argument from a variadic list
#define LAST_1(a) a
#define LAST_2(a, b) b
#define LAST_3(a, b, c) c
#define LAST_4(a, b, c, d) d
#define LAST_5(a, b, c, d, e) e
#define LAST_6(a, b, c, d, e, f) f
#define LAST_7(a, b, c, d, e, f, g) g
#define LAST_8(a, b, c, d, e, f, g, h) h
#define LAST_9(a, b, c, d, e, f, g, h, i) i
#define LAST_10(a, b, c, d, e, f, g, h, i, j) j

// Count number of arguments
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define COUNT_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N

// Concatenation helpers
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define CONCAT_IMPL(a, b) a##b

// Get last argument
#define LAST(...) CONCAT(LAST_, COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

// ============================================================================
// INTERNAL PADDING VERIFICATION (all consecutive pairs)
// ============================================================================

#define VERIFY_NO_INTERNAL_PADDING_1(T, m1) /* single member, nothing to check */

#define VERIFY_NO_INTERNAL_PADDING_2(T, m1, m2) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2);

#define VERIFY_NO_INTERNAL_PADDING_3(T, m1, m2, m3) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3);

#define VERIFY_NO_INTERNAL_PADDING_4(T, m1, m2, m3, m4) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4);

#define VERIFY_NO_INTERNAL_PADDING_5(T, m1, m2, m3, m4, m5) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4); \
    ASSERT_NO_INTERNAL_PADDING(T, m4, m5);

#define VERIFY_NO_INTERNAL_PADDING_6(T, m1, m2, m3, m4, m5, m6) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4); \
    ASSERT_NO_INTERNAL_PADDING(T, m4, m5); \
    ASSERT_NO_INTERNAL_PADDING(T, m5, m6);

#define VERIFY_NO_INTERNAL_PADDING_7(T, m1, m2, m3, m4, m5, m6, m7) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4); \
    ASSERT_NO_INTERNAL_PADDING(T, m4, m5); \
    ASSERT_NO_INTERNAL_PADDING(T, m5, m6); \
    ASSERT_NO_INTERNAL_PADDING(T, m6, m7);

#define VERIFY_NO_INTERNAL_PADDING_8(T, m1, m2, m3, m4, m5, m6, m7, m8) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4); \
    ASSERT_NO_INTERNAL_PADDING(T, m4, m5); \
    ASSERT_NO_INTERNAL_PADDING(T, m5, m6); \
    ASSERT_NO_INTERNAL_PADDING(T, m6, m7); \
    ASSERT_NO_INTERNAL_PADDING(T, m7, m8);

#define VERIFY_NO_INTERNAL_PADDING_9(T, m1, m2, m3, m4, m5, m6, m7, m8, m9) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4); \
    ASSERT_NO_INTERNAL_PADDING(T, m4, m5); \
    ASSERT_NO_INTERNAL_PADDING(T, m5, m6); \
    ASSERT_NO_INTERNAL_PADDING(T, m6, m7); \
    ASSERT_NO_INTERNAL_PADDING(T, m7, m8); \
    ASSERT_NO_INTERNAL_PADDING(T, m8, m9);

#define VERIFY_NO_INTERNAL_PADDING_10(T, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10) \
    ASSERT_NO_INTERNAL_PADDING(T, m1, m2); \
    ASSERT_NO_INTERNAL_PADDING(T, m2, m3); \
    ASSERT_NO_INTERNAL_PADDING(T, m3, m4); \
    ASSERT_NO_INTERNAL_PADDING(T, m4, m5); \
    ASSERT_NO_INTERNAL_PADDING(T, m5, m6); \
    ASSERT_NO_INTERNAL_PADDING(T, m6, m7); \
    ASSERT_NO_INTERNAL_PADDING(T, m7, m8); \
    ASSERT_NO_INTERNAL_PADDING(T, m8, m9); \
    ASSERT_NO_INTERNAL_PADDING(T, m9, m10);

#define VERIFY_NO_INTERNAL_PADDING(T, ...) \
    CONCAT(VERIFY_NO_INTERNAL_PADDING_, COUNT_ARGS(__VA_ARGS__))(T, __VA_ARGS__)

// ============================================================================
// TAIL PADDING WRAPPER - FIXED FOR PROPER MACRO EXPANSION
// ============================================================================

// Helper to get the actual last member name expanded
#define ASSERT_NO_TAIL_PADDING_EXPANDED(T, last_member) \
    ASSERT_NO_TAIL_PADDING(T, last_member)

// Two-level expansion to ensure LAST is fully expanded
#define ASSERT_NO_TAIL_PADDING_WRAPPER_IMPL(T, last_member) \
    ASSERT_NO_TAIL_PADDING_EXPANDED(T, last_member)

#define ASSERT_NO_TAIL_PADDING_WRAPPER(T, ...) \
    ASSERT_NO_TAIL_PADDING_WRAPPER_IMPL(T, LAST(__VA_ARGS__))

// ============================================================================
// MAIN VERIFICATION MACRO - COMPREHENSIVE CHECK
// ============================================================================

/**
 * VERIFY_DATA_STRUCT - Comprehensive compile-time verification for data structures
 *
 * This macro performs the following checks:
 * 1. Size matches sum of member sizes (no padding)
 * 2. Standard layout compliance
 * 3. Trivially copyable (safe for memcpy)
 * 4. Unique object representations (no padding bits)
 * 5. Proper array stride
 * 6. No tail padding
 * 7. No internal padding between members
 *
 * Usage:
 *   struct MyData {
 *       double a, b;
 *       int c, d;
 *   };
 *   VERIFY_DATA_STRUCT(MyData, a, b, c, d);
 */
#define VERIFY_DATA_STRUCT(T, ...) \
    static_assert(sizeof(T) == EXPECTED_SIZE_OF_MEMBERS(T, __VA_ARGS__), \
                  #T " has padding - sizeof(" #T ") != sum of member sizes"); \
    static_assert(std::is_standard_layout_v<T>, \
                  #T " must be standard layout"); \
    static_assert(std::is_trivially_copyable_v<T>, \
                  #T " must be trivially copyable"); \
    static_assert(std::has_unique_object_representations_v<T>, \
                  #T " has padding bits or non-unique representations"); \
    static_assert(sizeof(T[2]) == 2 * sizeof(T), \
                  #T " has unexpected array stride"); \
    VERIFY_NO_INTERNAL_PADDING(T, __VA_ARGS__); \
    ASSERT_NO_TAIL_PADDING_WRAPPER(T, __VA_ARGS__);

 // ============================================================================
 // OPTIONAL: GRANULAR VERIFICATION MACROS (for advanced users)
 // ============================================================================

 // Check only size and padding
#define VERIFY_DATA_STRUCT_SIZE_ONLY(T, ...) \
    static_assert(sizeof(T) == EXPECTED_SIZE_OF_MEMBERS(T, __VA_ARGS__), \
                  #T " has padding - sizeof(" #T ") != sum of member sizes")

// Check only type traits
#define VERIFY_DATA_STRUCT_TRAITS_ONLY(T) \
    static_assert(std::is_standard_layout_v<T>, \
                  #T " must be standard layout"); \
    static_assert(std::is_trivially_copyable_v<T>, \
                  #T " must be trivially copyable"); \
    static_assert(std::has_unique_object_representations_v<T>, \
                  #T " has padding bits or non-unique representations")

// ============================================================================
// RUNTIME DEBUGGING MACROS
// ============================================================================

#ifndef NDEBUG
/**
 * DUMP_STRUCT_LAYOUT - Runtime dump of structure memory layout
 *
 * Only available in debug builds (when NDEBUG is not defined).
 * Prints detailed information about member offsets and padding.
 *
 * Usage:
 *   DUMP_STRUCT_LAYOUT(MyData, a, b, c, d);
 */
#define DUMP_STRUCT_LAYOUT(T, ...) \
    do { \
        using namespace pensar_digital::cpplib; \
        StructLayoutDumper<T> dumper(#T); \
        DUMP_MEMBERS_IMPL(T, __VA_ARGS__) \
    } while(0)

#define DUMP_MEMBER(T, member) \
    dumper.add_member(#member, MEMBER_OFFSET(T, member), MEMBER_SIZE(T, member));

#define DUMP_MEMBERS_IMPL(T, ...) \
    DUMP_MEMBERS_APPLY(T, __VA_ARGS__)

#define DUMP_MEMBERS_APPLY(T, ...) \
    FOR_EACH_MEMBER(DUMP_MEMBER, T, __VA_ARGS__)

#define FOR_EACH_MEMBER(macro, T, member, ...) \
    macro(T, member) \
    __VA_OPT__(FOR_EACH_MEMBER_AGAIN PARENS (macro, T, __VA_ARGS__))

#define FOR_EACH_MEMBER_AGAIN() FOR_EACH_MEMBER

#else
#define DUMP_STRUCT_LAYOUT(T, ...) /* No-op in release builds */
#endif

#endif // DATA_HPP