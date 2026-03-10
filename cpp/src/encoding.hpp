#ifndef ENCODE_HPP
#define ENCODE_HPP

#include "cs.hpp"
#include "bool.hpp"
#include "array.hpp"
#include "endian.hpp"
#include "concept.hpp"

namespace pensar_digital
{
	namespace cpplib
	{
		using BomBytes = CArray<4, unsigned char>;
		inline static BomBytes NO_BOM = { 0, 0, 0, 0 };
		inline static BomBytes BOM_UTF_8     = { 0xEF, 0xBB, 0xBF, 0x00 };
		inline static BomBytes BOM_UTF_16_BE = { 0xFE, 0xFF, 0x00, 0x00 };
		inline static BomBytes BOM_UTF_16_LE = { 0xFF, 0xFE, 0x00, 0x00 };
		inline static BomBytes BOM_UTF_32_BE = { 0x00, 0x00, 0xFE, 0xFF };
		inline static BomBytes BOM_UTF_32_LE = { 0xFF, 0xFE, 0x00, 0x00 };

		

		struct Encoding
		{
			CS<> name;
			BomBytes bom; // Byte Order Mark.
			Endian endian; // std::endian::big if big endian, std::endian::little if little endian.
			Bool has_bom;
			uint8_t bom_size;

			constexpr Encoding() noexcept
				: name(), bom(NO_BOM), endian(Endian::NOT_APPLICABLE), has_bom(Bool::F), bom_size(0)
			{}

			constexpr Encoding(const C* name_value,
				const BomBytes& bom_value,
				Endian endian_value,
				Bool has_bom_value,
				uint8_t bom_size_value) noexcept
				: name(name_value), bom(bom_value), endian(endian_value), has_bom(has_bom_value), bom_size(bom_size_value)
			{}

			bool operator==(const Encoding& other) const noexcept
			{
				return name == other.name;
			}
		};
		// Asserts Encoding is compliant with StdLayoutTriviallyCopyableNoPadding.
		static_assert(StdLayoutTriviallyCopyableNoPadding<Encoding>, "Encoding must be a standard layout, trivially copyable, and have no padding bytes.");
		

		inline const Encoding ASCII         = { W("ASCII")         , NO_BOM       , Endian(Endian::NOT_APPLICABLE), Bool::F, 0};
		inline const Encoding UTF_8         = { W("UTF-8")         , NO_BOM       , Endian(Endian::NOT_APPLICABLE), Bool::F, 0};
		inline const Encoding UTF_8_BOM     = { W("UTF-8-BOM")     , BOM_UTF_8    , Endian(Endian::NOT_APPLICABLE), Bool::T, 3};
		inline const Encoding UTF_16_BE     = { W("UTF-16-BE")     , NO_BOM       , Endian(Endian::BIG)           , Bool::F, 2};
		inline const Encoding UTF_16_BE_BOM = { W("UTF-16-BE-BOM") , BOM_UTF_16_BE, Endian(Endian::BIG)           , Bool::T, 2};
		inline const Encoding UTF_16_LE     = { W("UTF-16-LE")     , NO_BOM       , Endian(Endian::LITTLE)        , Bool::F, 2};
		inline const Encoding UTF_16_LE_BOM = { W("UTF-16-LE-BOM") , BOM_UTF_16_LE, Endian(Endian::LITTLE)        , Bool::F, 2};
		inline const Encoding UTF_32_BE     = { W("UTF-32-BE")     , NO_BOM       , Endian(Endian::BIG)           , Bool::F, 4};
		inline const Encoding UTF_32_BE_BOM = { W("UTF-32-BE-BOM") , BOM_UTF_32_BE, Endian(Endian::BIG)           , Bool::T, 4};
		inline const Encoding UTF_32_LE     = { W("UTF-32-LE")     , NO_BOM       , Endian(Endian::LITTLE)        , Bool::F, 4};
		inline const Encoding UTF_32_LE_BOM = { W("UTF-32-LE-BOM") , BOM_UTF_32_LE, Endian(Endian::LITTLE)        , Bool::T, 4};
			}; // namespace cpplib
} // namespace pensar_digital

#endif // ENCODE_HPP
