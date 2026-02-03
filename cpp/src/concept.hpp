#ifndef CONCEPT
#define CONCEPT

#include "constant.hpp"
#include "factory.hpp" // for NewFactory

#include <concepts>
#include <iostream>
#include <ranges>
#include <memory> // for std::shared_ptr
#include <type_traits> // for std::is_same
#include <span>
#include <cstddef> // for std::byte



namespace pensar_digital
{
	namespace cpplib
	{
		// Checkable concept. Requires a member function ok() returning something convertible to bool.
		template <typename T, typename... Args>
		concept Checkable = requires (T t, Args&& ... args)
		{
			{t.ok(args ...)} -> std::convertible_to<bool>;
		};

		// Hashable concept. Requires a member function hash() returning something convertible to Hash type.
		template <typename T>
		concept Hashable = requires (T t)
		{
			{t.hash()} -> std::convertible_to<Hash>;
		};

		// RangeCheckable concept. Requires Checkable and operators >= and <=.
		template <typename T>
		concept RangeCheckable = requires (T t)
		{
			{t >= t} -> std::convertible_to<bool>;
			{t <= t} -> std::convertible_to<bool>;
		};
		
		// DefaultConstructible concept. Requires a default constructor.
		template <typename T>
		concept DefaultConstructible = requires
		{
			{T()} noexcept;
		};

		/// Concept for a class with a noexcept initialize method returning something convertible to bool.
		template <typename T, typename... Args>
		concept Initializable = requires (T t, Args... args)
		{
			{T(std::forward<Args>(args) ...)} noexcept;
			{t.initialize(std::forward<Args>(args) ...)} noexcept -> std::convertible_to<bool>;
		};

		// Factorable concept requires a public type defined named Factory and a descendent of NewFactory.
		template <typename T>
		concept Factorable = requires
		{
			typename T::Factory;
			std::derived_from<typename T::Factory, NewFactory<T>>;
		};


		// FactoryConstructible concept. Requires Initializable and a static factory method named get returning something convertible to T&.
		template <class T, typename... Args>
		concept FactoryConstructible = Initializable<T, Args...>&& requires (Args... args)
		{
			{T::get(args ...)} noexcept -> std::convertible_to<typename T::Factory::P>;
		} && Factorable<T>;
	
		// FactoryCloneable concept requires Assignable and FactoryConstructible.
		//template <typename T, typename... Args>
		//concept FactoryCloneable = Assignable<T> && FactoryConstructible<T, Args...>;

		// AFactory concept requires a virtual get(const Args& ... args) const public method that returns
		// something convertible to std::shared_ptr<T>.
		template <typename T, typename... Args>
		concept AFactory = requires (T t, Args... args)
		{
			{t.get(args ...)} noexcept -> std::convertible_to<std::shared_ptr<T>>;
		};

		// IsLikeArray concept. Requires T to be a container with a public T::value_type defined and with a public method at(size_t i) and an operator[size_t i] returning something convertible to T::value_type&. And a size () returning something convertible to size_t.
		template <typename T>
		concept IsLikeArray = requires (T t, size_t i)
		{
			typename T::value_type;
			{ t.at(i) } -> std::convertible_to<typename T::value_type&>;
			{ t[i] } -> std::convertible_to<typename T::value_type&>;
			{ t.size() } -> std::convertible_to<size_t>;
		};

		// IsLikeArrayOfV is a concept requiring IsLikeArray<T> and T::value_type to be of type V.
		template <typename T, typename V>
		concept IsLikeArrayOfV = IsLikeArray<T> && std::is_same<typename T::value_type, V>::value;
        
		template<class C>
        concept IsContainer =
            std::ranges::range<C> &&
            requires(C c, const C cc, typename C::value_type v, std::size_t i) {
                typename C::value_type;
                typename C::reference;
                typename C::const_reference;
                { c.begin() } -> std::same_as<typename C::iterator>;
                { cc.begin() } -> std::same_as<typename C::const_iterator>;
                { c.end() } -> std::same_as<typename C::iterator>;
                { cc.end() } -> std::same_as<typename C::const_iterator>;
                { c.size() } -> std::convertible_to<std::size_t>;
                { c.empty() } -> std::same_as<bool>;

                // index access checks
                { c[i] } -> std::same_as<typename C::reference>;
                { cc[i] } -> std::same_as<typename C::const_reference>;
                { c.at(i) } -> std::same_as<typename C::reference>;
                { cc.at(i) } -> std::same_as<typename C::const_reference>;
            };

			// IsContainerOfV is a concept requiring IsContainer<C> and C::value_type to be of type V.
			template <typename C, typename V>
			concept IsContainerOfV = IsContainer<C> && std::is_same<typename C::value_type, V>::value;

		// Interfaceable concept two public typedefs named i_type and i_type_ro.
		template <typename T>
		concept Interfaceable = requires
		{
			typename T::I;
			typename T::I_RO;
		};	

		// Negatable. Requires the unary operator ! to be defined returning something convertible to bool.
		template <typename T>
		concept Negatable = requires (T t)
		{
			{!t} noexcept -> std::convertible_to<bool>;
		};

		// Andable. Requires the binary operator && to be defined returning something convertible to bool.
		template <typename T>
		concept Andable = requires (T t)
		{
			{t && t} noexcept -> std::convertible_to<bool>;
		};

		// Orable. Requires the binary operator || to be defined returning something convertible to bool.
		template <typename T>
		concept Orable = requires (T t)
		{
			{t || t} noexcept -> std::convertible_to<bool>;
		};

		// Xorable. Requires the binary operator ^ to be defined returning something convertible to bool.
		template <typename T>
		concept Xorable = requires (T t)
		{
			{t ^ t} noexcept -> std::convertible_to<bool>;
		};

		// NarrowOutputStreamable concept.
		template<typename T>
		concept NarrowOutputStreamable = requires(T a, std::ostream & os) { { os << a } -> std::same_as<std::ostream&>; };

		// WideOutputStreamable concept.
		template<typename T>
		concept WideOutputStreamable = requires(T a, std::wostream & os) { { os << a } -> std::same_as<std::wostream&>; };

		// OutputStreamable concept. NarrowOutputStreamable or WideOutputStreamable.
		template<typename T>
		concept OutputStreamable = NarrowOutputStreamable<T> || WideOutputStreamable<T>;

		// NarrowInputStreamable concept.
		template<typename T>
		concept NarrowInputStreamable = requires(T a, std::istream & is) 	{ { is >> a } -> std::same_as<std::istream&>; };

		// WideInputStreamable concept.
		template<typename T>
		concept WideInputStreamable = requires(T a, std::wistream & is) { { is >> a } -> std::same_as<std::wistream&>; };

		// InputStreamable concept. NarrowInputStreamable or WideInputStreamable.
		template<typename T>
		concept InputStreamable = NarrowInputStreamable<T> || WideInputStreamable<T>;

		// Streamable concept.
		template<typename T>
		concept Streamable = OutputStreamable<T> && InputStreamable<T>;

		// A concept requiring a type that supports reinterpret_cast<char*>(*t).
		template <typename T>
		concept CharCastable = requires(T * t) {
			{ reinterpret_cast<char*>(*t) } -> std::same_as<char*>;
		};

		// A concept requiring a type that is castable to std::byte.
		template <typename T>
		concept ByteCastable = requires(T * t) {
			{ reinterpret_cast<std::byte*>(*t) } -> std::same_as<std::byte*>;
		};	

		// Sizeofable, a concept requiring a type that supports sizeof(t).
		template <typename T>
		concept Sizeofable = requires(T t)
		{
			{ sizeof(t) } -> std::same_as<size_t>;
		};
		
		// Identifiable concept requires a public id () method returning something convertible to Id type.
		template <typename T>
		concept Identifiable = requires(T t) { { t.id() } -> std::convertible_to<Id>; };	

		// SizeableIdentifiable concept requires Identifiable and SizeableType.	
		template <typename T>
		concept SizeableIdentifiable = Identifiable<T> && Sizeofable<T>;

		// Countable requires a public count () method returning something convertible to size_t .
		template <typename T>
		concept Countable = requires(T t) { { t.count() } -> std::convertible_to<size_t>; };

		// Sizeable, requires SizeableType or SizeableObject.
		template <typename T>
		concept Sizeable = requires(T t) { { t.size() } -> std::convertible_to<size_t>; };

		// BinaryStreamable concept requires CharCastable, Sizeable and Streamable.
		template <typename T>
		concept BinaryStreamable = CharCastable<T> && Sizeofable<T> && Streamable<T>;

		// ConstByteSpanConvertible concept requires a public method bytes() returning something convertible to std::span<const std::byte>.
		template <typename T>
		concept ConstByteSpanConvertible = requires (T t)
		{
			{ t.bytes() } -> std::convertible_to<std::span<const std::byte>>;
		};

		// WritableByteSpanConvertible concept requires a public method wbytes() returning something convertible to std::span<std::byte>.
		template <typename T>
		concept WritableByteSpanConvertible = requires (T t)
		{
			{ t.wbytes() } -> std::convertible_to<std::span<std::byte>>;
		};
		// ByteSpanConvertible concept requires ConstByteSpanConvertible and WritableByteSpanConvertible.
		template <typename T>
		concept ByteSpanConvertible = ConstByteSpanConvertible<T> && WritableByteSpanConvertible<T>;
		
		// BinaryReadable concept requires a public method read (std::span<std::byte>& bytes) returning something convertible to T&.
		template <typename T>
		concept BinaryReadable = requires(T t, std::span<std::byte>& bytes) { { t.read (bytes) } -> std::convertible_to<void>; };

		// ObjectBinaryReadable concept requires a type T with a public method template <class Obj> void read(Obj* p). Where Obj must comply with BinaryReadable.
		template <typename T, typename Obj>
		concept ObjectBinaryReadable = requires(T t, Obj* p)
		{
			{ t.template read<Obj> (p) } -> std::convertible_to<void>;
		} && BinaryReadable<T>;

		// FactoryObjBinaryReadable concept requires a type T reader of U objects. T must have a public method read with signature: U::Factory::P read<U, Args...> () . U must comply with BinaryInputtable && FactoryConstructible.
		template <typename T, typename Obj, typename... Args>
		concept FactoryObjBinaryReadable = requires(T t)
		{
			{ t.template read<Obj, Args...> () } -> std::convertible_to<typename Obj::Factory::P>;
		} && ObjectBinaryReadable<T, Obj>&& FactoryConstructible<Obj, Args ...>;
		
		// Pointable concept requires a type T that supports operator-> returning something convertible to T* and supports *T returning something convertible to T&.
		template<typename T>
		concept Pointable = requires(T t) 
		{ 
			{ t.operator->() } -> std::convertible_to<T*>; 
			{ *t             } -> std::convertible_to<T&>; 
		};

		// StandardLayout concept requires a type T that is standard layout.
		template<typename T>
		concept StandardLayout = std::is_standard_layout_v<T>;

		// TriviallyCopyable concept requires a type T that is trivially copyable.
		template<typename T>
		concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

		// StdLayoutTriviallyCopyable concept requires a type T that is standard layout and trivially copyable.
		template<typename T>
		concept StdLayoutTriviallyCopyable = StandardLayout<T> && TriviallyCopyable<T>;	

		// NoPadding concept requires a type T with no padding bits (unique object representations).
		template<typename T>
		concept NoPadding = std::has_unique_object_representations_v<T>;

		// StdLayoutTriviallyCopyableNoPadding concept requires standard layout, trivially copyable, and no padding.
		template<typename T>
		concept StdLayoutTriviallyCopyableNoPadding = StdLayoutTriviallyCopyable<T> && NoPadding<T>;
		// Memcpyasble concept. Requires a function data() returning something convertible to void*. Usually a pointer to std::byte. And a data_size() returning something convertible to size_t.
        
		template<typename T>
            concept IntegerLike =
            std::is_integral_v<T> &&
            !std::is_same_v<T, bool>;
        
		template<typename T>
    		concept WireSafe =
        	std::is_trivially_copyable_v<T> &&
        	std::has_unique_object_representations_v<T>;
		
		template<typename T>
    		concept IEEE754Binary =
        	std::is_floating_point_v<T> &&
        	std::numeric_limits<T>::is_iec559 &&
        	(sizeof(T) == 4 || sizeof(T) == 8);
			
		template <typename T>
		concept HasStdLayoutTriviallyCopyableData = requires (T t)
		{
			{ t.data() } -> std::convertible_to<const Data*>;
			{ t.data_size() } -> std::convertible_to<size_t>;
			{ T::DATA_SIZE } -> std::convertible_to<size_t>;
			{ T::SIZE } -> std::convertible_to<size_t>;
		}&& TriviallyCopyable<typename T::DataType>&& StdLayoutTriviallyCopyable<typename T::DataType>;

		template <class T>
		concept TriviallyPersistable = requires (T t)
		{
			{t.data()     } -> std::convertible_to<const Data*>;
			{t.data_size()} -> std::convertible_to<size_t>;
		} && TriviallyCopyable<typename T::DataType> && Identifiable<T> && Hashable<T>;

		// TriviallyDestructible concept requires a type T that is trivially destructible.
		template<typename T>
		concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

		// TriviallyConstructible concept requires a type T that is trivially constructible.
		template<typename T>
		concept TriviallyConstructible = std::is_trivially_constructible_v<T>;

		// TriviallyAssignable concept requires a type T that is trivially assignable.
		template<typename T, typename U>
		concept TriviallyAssignable = std::is_trivially_assignable_v<T, U>;

		// TriviallyCopyAssignable concept requires a type T that is trivially copy assignable.
		template<typename T>
		concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;

		// TriviallyMoveAssignable concept requires a type T that is trivially move assignable.
		template<typename T>
		concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;

		// TriviallyMoveConstructible concept requires a type T that is trivially move constructible.
		template<typename T>
		concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;

		// TriviallyCopyConstructible concept requires a type T that is trivially copy constructible.
		template<typename T>
		concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

		// TriviallyMovable concept requires a type T that is trivially movable.
		template<typename T>
		concept TriviallyMovable = TriviallyMoveConstructible<T> && TriviallyMoveAssignable<T>;

	} // namespace cpplib
} // namespace pensar_digital	
#endif // CONCEPT
