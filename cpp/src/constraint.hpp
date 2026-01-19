// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include "object.hpp"
#include "constant.hpp"

#include <cstdarg>
#include <regex> // std::regex, std::regex_match
#include <variant> // std::variant, std::get

namespace pensar_digital
{
	namespace cpplib
	{
		static const S ONLY_DIGITS_REGEX = W("^[0-9]*$");
		static const S AT_LEAST_ONE_DIGIT_REGEX = W("^[0-9]+$");
		static const S ONLY_LETTERS_REGEX = W("^[a-zA-Z]*$");
		static const S ONLY_LETTERS_AND_DIGITS_REGEX = W("^[a-zA-Z0-9]*$");
		static const S ONLY_LETTERS_AND_DIGITS_AND_UNDERSCORES_REGEX = W("^[a-zA-Z0-9_]*$");

		template <typename L, typename R, typename ResultType, class D>
		class Operand : public Object
		{
			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Operand"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				Operand (const Id aid = NULL_ID) : Object(aid) {}
				
				template <typename ... Args>
				ResultType operator () (L* left, R* right = nullptr, Args&& ... args) const
				{
					return static_cast<D*>(this)->::operator () (left, right, args ...);
				};
			virtual ~Operand() {}
		};

		template <typename T, typename ResultType, class D>
		class UnaryOperand : public Operand<T, T, ResultType, UnaryOperand<T, ResultType, D>>
		{
			private:
				typedef UnaryOperand<T, ResultType, D> UnaryOp;
				typedef Operand <T, T, ResultType, UnaryOp> Base;
			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("UnaryOperand"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				UnaryOperand (const Id aid = NULL_ID) : Base(aid) {}
				
				virtual ~UnaryOperand() {}

				template <typename ... Args>
				ResultType operator () (T* ptr, const T* not_used = nullptr, Args&& ... args) const
				{
					return static_cast<D*>(this)->::operator () (ptr, args ...);
				};
		};
		 
		template <Checkable T>
		class NotOperand : public UnaryOperand<T, bool, NotOperand<T>>
		{
			private:
				typedef UnaryOperand<T, bool, NotOperand<T>> Base;
			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("NotOperand"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				NotOperand (const Id aid = NULL_ID) : Base(aid) {}
			
				virtual ~NotOperand() {}

				template <typename ... Args>
				bool operator () (const T* ptr, Args&& ... args) const noexcept
				{
					return !ptr->ok (args ...);
				};
		};

		template <typename L, typename R, typename ResultType, class D>
		class BinaryOperand : public Operand<L, R, ResultType, BinaryOperand<L, R, ResultType, D>>
		{
			private:
				typedef Operand<L, R, ResultType, BinaryOperand<L, R, ResultType, D>> Base;
			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("BinaryOperand"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
				BinaryOperand (const Id aid = NULL_ID) : Base (aid) {}
				virtual ~BinaryOperand() {}

				template <typename ... Args>
				ResultType operator () (const L* left, const R* right = nullptr, Args&& ... args) const
				{
					return static_cast<D*>(this)->operator () (left, right, args ...);
				};
		};

		template <Checkable L, Checkable R>
		class AndOperand : public BinaryOperand<L, R, bool, AndOperand<L, R>>
		{
			private:
				typedef BinaryOperand<L, R, bool, AndOperand<L, R>> Base;
			
			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("AndOperand"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
				AndOperand (const Id aid = NULL_ID) noexcept : Base (aid) {}
			
				virtual ~AndOperand() {}

				template <typename ... Args>
				bool operator ()  (const L& left, const R& right, Args&& ... args) const noexcept
				{ 
					return (left.ok(args ...) && right.ok(args ...));
				}
		};


		template <Checkable L, Checkable R>
		class OrOperand : public BinaryOperand<L, R, bool, OrOperand<L, R>>
		{
		private:
			typedef BinaryOperand<L, R, bool, OrOperand<L, R>> Base;

		public:
			inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("OrOperand"), 1, 1, 1 };
			inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
			OrOperand(const Id aid = NULL_ID) noexcept : Base(aid) {}

			virtual ~OrOperand() {}

			template <typename ... Args>
			bool operator ()  (const L& left = nullptr, const R& right = nullptr, Args&& ... args) const noexcept
			{
				return (left.ok(args ...) || right.ok(args ...));
			}
		};


		template <Checkable L, Checkable R>
		class XorOperand : public BinaryOperand<L, R, bool, XorOperand<L, R>>
		{
		private:
			typedef BinaryOperand<L, R, bool, XorOperand<L, R>> Base;

		public:
			inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("XorOperand"), 1, 1, 1 };
			inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
			XorOperand(const Id aid = NULL_ID) noexcept : Base(aid) {}

			virtual ~XorOperand() {}

			template <typename ... Args>
			bool operator ()  (const L& left = nullptr, const R& right = nullptr, Args&& ... args) const noexcept
			{
				return (left.ok(args ...) ^ right.ok(args ...));
			}
		};

		/// Constraint class for the Constraint Programming Library
		/// \author Mauricio Gomes
		/// \date 2016-12-28
		/// \version 1.0
		/// \note Base class for all constraints. It must be Constrainable.
		/// DerivedConstraint is here to simulate a static polymorphism.
		template <class D>
		class Constraint : public Object
		{
			private:
			S name;

			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Constraint"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }
				//typedef IConstraint<Constraint>     I; // Interface type.
				//typedef IConstraintRO<Constraint> IRO; // Read only interface type.

				/// Default constructor
				Constraint(const Id aid = NULL_ID, const S& aname = "") : Object(aid), name(aname) {}
			
				virtual ~Constraint () {}

				/// Returns the name of the constraint
				inline const S& get_name () const noexcept { return name; }

				/// Sets the name of the constraint
				inline void set_name (const S& aname) { name = aname; }

				// Implements initialize method from Initializable concept.
				virtual bool initialize(const Id& aid = NULL_ID, const S& aname = "") noexcept
				{
					Object::set_id(aid);
					name = aname;
					return true;
				}

				template <typename... Args> bool ok (Args&& ... args) const noexcept
				{
					static_assert (Checkable<D, Args...>);
					return static_cast<D*>(this)->::ok (std::forward<Args>(args)...);
				};

				template <typename... Args>
				bool Or (const Constraint<D>& other, Args& ... args) const noexcept
				{
					return ok (args ...) || other.ok (args ...);
				};

				template <typename... Args>
				bool And (const Constraint<D>& other, Args& ... args) const noexcept
				{
					return ok(args ...) && other.ok(args ...);
				};

				template <typename... Args>
				bool Xor (const Constraint<D>& other, Args& ... args) const noexcept
				{
					return ok(args ...) ^ other.ok(args ...);
				};

				template <typename... Args>
				bool Not (Args& ... args) const noexcept
				{
					return !ok(args ...);
				};
		};
		
		/// Composite constraint class for the Constraint Programming Library
		/// \author Mauricio Gomes
		/// \date 2016-12-28
		/// \version 1.0
		template <Checkable L, Checkable R>
		class CompositeConstraint : public Constraint <CompositeConstraint<L, R>>
		{
			public:
				typedef std::variant <AndOperand <L, R>, OrOperand <L, R>, XorOperand <L, R>> OpType;
				typedef enum { AND, OR, XOR } OpIndex;
			private:
				typedef CompositeConstraint<L, R> C;
				typedef Constraint <C> Base;

				typedef std::shared_ptr<Base  >  BasePtr;
				typedef std::shared_ptr<L     >  LeftPtr;
				typedef std::shared_ptr<R     > RightPtr;
				const L&  left;
				const R& right;
				OpType op;
			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("CompositeConstraint"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				CompositeConstraint (const L&    aleft,
									 const R&  aright,
									 const OpType aop,
									 const Id aid = NULL_ID, 
									 const S& aname = W("")) : Base(aid, aname),
									 left(aleft), right(aright), op (aop)
				{
				}
				virtual ~CompositeConstraint() {}

				template <typename ...Args> 
				bool ok (Args&& ...args) const noexcept				
				{ 
					switch (op.index ())
{
						case AND:
							return std::get<AND>(op) (left, right, args ...);
						case OR:
							return std::get<OR>(op) (left, right, args ...);
						case XOR:
							return std::get<XOR>(op) (left, right, args ...);
					}
					return false;
				};
		};	// class CompositeConstraint

		
		template <Checkable L, Checkable R>
		CompositeConstraint<L, R> operator && (const L& left, const R& right)
		{
			typedef CompositeConstraint<L, R> C;
			return * new C (left, right);
		}

		template <Checkable L, Checkable R>
		CompositeConstraint<L, R> operator || (const L& left, const R& right)
		{
			typedef CompositeConstraint<L, R> C;
			return *new C(left, right, OrOperand<L,R>());
		}

		// S constraint.
		class StringConstraint : public Constraint <StringConstraint>
		{
			private:
				typedef Constraint<StringConstraint> Base;
				const std::basic_regex <C> regex;

			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("StringConstraint"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				/// Default constructor
				StringConstraint (const S& aregex = W(""), const Id aid = NULL_ID, const S& aname = W(""))
					: Base(aid, aname), regex (aregex) { }

				virtual ~StringConstraint() {}

				bool ok(const S& s = W("")) const noexcept
				{ 
					return std::regex_search (s, regex);
				};

		};	// class StringConstraint

		// Range constraint.
		template <RangeCheckable T>	
		class RangeConstraint : public Constraint <RangeConstraint<T>>
		{
			private:
				typedef Constraint<RangeConstraint<T>> Base;
				const T mmin;
				const T mmax;

			public:
				inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("RangeConstraint"), 1, 1, 1 };
				inline virtual const ClassInfo* info_ptr() const noexcept { return &INFO; }

				/// Default constructor
				RangeConstraint (const T& amin, const T& amax, const Id aid = NULL_ID, const S& aname = W("")) 
					: Base(aid, aname), mmin (amin), mmax (amax) { }

				RangeConstraint (const T& avalue, const Id aid = NULL_ID, const S& aname = W(""))
					: Base(aid, aname), mmin (avalue), mmax (avalue) { }	

				virtual ~RangeConstraint() {}

				bool ok(const T& t = 0) const noexcept
				{
					return (t >= mmin) && (t <= mmax);
				};

		};	// class RangeConstraint
	}	// namespace cpplib
}	// namespace pensar_digital



#endif // CONSTRAINT_HPP	
