# Copilot instructions for pensar

## Big picture
- This repo is the **cpplib** C++ utility library used across Pensar Digital projects; most code lives in cpp/src (headers only).
- Public types are under the `pensar_digital::cpplib` namespace (see cpp/src/s.hpp, cpp/src/code_util.hpp, cpp/src/bool.hpp).
- String handling is dual narrow/wide: `C`/`S` types and `W("...")` literal macro come from cpp/src/string_def.hpp; code should use these rather than raw `std::string` literals.
- ICU is a core dependency for Unicode utilities (see cpp/src/icu_util.hpp and cpp/src/cs.hpp for accent-insensitive comparisons).

## Build & test workflow
- Build is CMake-based; `CMAKE_CXX_STANDARD=23` and compile_commands.json is generated (see CMakeLists.txt).
- Tests are a custom framework; the `pensar_tests` target is built from cpp/src/test/*.cpp (see CMakeLists.txt).
- Test entrypoint: cpp/src/test/main.cpp calls `pensar_digital::unit_test::all_tests().run()`.

## Project-specific conventions
- Use `Bool` (tri-state) instead of `bool` when the API expects cpplib types (cpp/src/bool.hpp).
- Use `Result<T>` for error-aware return values (cpp/src/code_util.hpp) instead of exceptions in lightweight helpers.
- Logging uses `LOG(...)` macro from cpp/src/log.hpp; default log file path is Windows-style `C:\out\log.txt`.

## Deriving from `Object`
- Define a nested `Data` type with standard layout, trivially copyable, and no padding.
- Add `using DataType = Data;` and a `data()` accessor returning `Data*`.
- Add `using Ptr = std::shared_ptr<Derived>;` for consistent smart pointers.
- Call `initialize()` (or `assign()`) from constructors to set the data.
- Override `info_ptr()` and keep `INFO` in sync for serialization; use `W("...")` for class names.
- If you need factory construction, add a `Factory` typedef and a static factory instance like `Object`.
- If you customize binary serialization, override `binary_read()` and `binary_write()` and keep class/version checks consistent with `INFO`.

Minimal example:
```cpp
class Person : public pd::Object
{
public:
	using Ptr = std::shared_ptr<Person>;
	struct Data : public pd::Object::DataType
	{
		pd::Id age;
		Data(const pd::Id& id = pd::NULL_ID, const pd::Id& a = 0) noexcept
			: pd::Object::DataType(id), age(a) {}
	};
	static_assert(pd::TriviallyCopyable<Data>, "Data must be trivially copyable");
	static_assert(pd::StandardLayout<Data>, "Data must be standard layout");
	static_assert(pd::NoPadding<Data>, "Data must have no padding");
	using DataType = Data;
	inline static const ClassInfo INFO = { CPPLIB_NAMESPACE, W("Person"), 1, 1, 1 };
	const ClassInfo* info_ptr() const noexcept override { return &INFO; }
	using Factory = pd::Factory<Person, DataType>;
	inline static Factory mfactory = { 3, 10, DataType{} };

private:
	Data mdata;

public:
	const pd::Data* data() const noexcept override { return &mdata; }
	Person(const Data& d = Data{}) noexcept { initialize(d); }

	std::istream& binary_read(std::istream& is, const std::endian& order = std::endian::native) override
	{
		INFO.test_class_name_and_version(is, order);
		return is.read((char*)(&mdata), DATA_SIZE);
	}

	std::ostream& binary_write(std::ostream& os, const std::endian& order = std::endian::native) const override
	{
		INFO.binary_write(os, order);
		return os.write((const char*)(&mdata), DATA_SIZE);
	}
};
```

## Custom test patterns
- Tests are registered via macros in unit_test/src/test.hpp: `TEST(name, is_enabled) ... TEST_END(name)` and `CHECK*` macros.
- New tests should live in cpp/src/test and include unit_test/src/test.hpp.

## External dependencies & integration
- ICU must be available and configured via `ICU_ROOT` (CMakeLists.txt); CMake auto-detects from Conan cache when `ICU_AUTO_DETECT=ON`.
- ICU headers/libraries are required: `icuuc`, `icui18n`, `icudata`, `icuio`.
