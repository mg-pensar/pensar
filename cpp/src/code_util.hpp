#ifndef CODE_UTIL_HPP
#define CODE_UTIL_HPP

#include <ostream>

#include "cs.hpp"
#include "bool.hpp"

namespace pensar_digital 
{
    namespace cpplib 
    {
        template <typename T>
        struct Result 
        {
			using ErrorMessageType = CS<0, 256>;
			using ResultType = T;
            T mresult;
            Bool mok = Bool::T;
            ErrorMessageType merror_message = EMPTY;

			// Default constructor
            Result() = default;
            
			// Default copy constructor
			Result(const Result&) = default;
			
			// Default move constructor
			Result(Result&&) = default;
			
			Result(ResultType rt, Bool ok = Bool::T, ErrorMessageType err = EMPTY) : mresult(rt), mok(ok), merror_message(err) {}

			Result(ErrorMessageType err, ResultType r = T{}) : mresult(r), mok(Bool::F), merror_message(err) {}

			// Default copy assignment operator
			Result& operator=(const Result&) = default;

			// Default move assignment operator
			Result& operator=(Result&&) = default;
			
			// Default destructor
			~Result() = default;
            
			// Implicit conversion from ErrorMessageType
			Result(const ErrorMessageType& error_message) : merror_message(error_message), mok(Bool::F) {}
			Result(const C* error_message) : merror_message(CS<0, 256>(error_message)), mok(Bool::F) {}

            // Implicit conversion to Bool
            operator Bool() const { return mok; }
			
			// Explicit conversion to native bool for boolean contexts (if/while, etc.)
			explicit operator bool() const noexcept { return mok == Bool{ Bool::T }; }

			// Implicit conversion to ResultType. SFINAE out when U is native bool or Bool to avoid redefinition.
			template <typename U = T, typename std::enable_if<
				!std::is_same<U, bool>::value && !std::is_same<U, Bool>::value, int>::type = 0>
			operator T() const noexcept { return mresult; }

            // Implicit conversion to ErrorMessageType
			operator ErrorMessageType() const { return merror_message; }
            

			// == operator
			bool operator==(const T& other) const
			{
				return mresult == other;
			}

			// != operator
			bool operator!=(const T& other) const
			{
				return !(*this == other);
			}

			// < operator
			bool operator<(const T& other) const
			{
				return mresult < other;
			}

			// = operator for ResultType.
			Result& operator=(const T& other)
			{
				mresult = other;
				return *this;
			}

			// = operator for Bool.
			Result& operator=(const Bool& other)
			{
				mok = other;
				return *this;
			}

			// = operator for ErrorMessageType.
			Result& operator=(const ErrorMessageType& other)
			{
				merror_message = other;
				mok = Bool::F;
				return *this;
			}

			// ! operator
			bool operator!() const
			{
				return !mok;
			}
        };
		static_assert(std::is_trivially_copyable<Result<int>>::value, "Result<int> must be trivially copyable.");

        template <typename T>
            requires OutputStreamable<T>
        inline std::ostream& operator<<(std::ostream& os, const Result<T>& r)
        {
            return os << r.mresult;
        }

        template <typename T>
            requires OutputStreamable<T>
        inline std::wostream& operator<<(std::wostream& os, const Result<T>& r)
        {
            return os << r.mresult;
		}        
	}  // namespace cpplib
}  // namespace pensar_digital

#endif // CODE_UTIL_HPP