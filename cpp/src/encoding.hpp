#ifndef ENCODE_HPP
#define ENCODE_HPP

#include "cs.hpp"
#include "bool.hpp"
#include "array.hpp"
#include "endian.hpp"

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

		struct Encoding {};

		template <size_t NameSize, Bool HasBOM = Bool::T>
		struct EncodingBase : public Encoding
		{
			CS<NameSize> name;
			BomBytes bom; // Byte Order Mark.
			Endian endian; // std::endian::big if big endian, std::endian::little if little endian.
			uint8_t bom_size;

			constexpr EncodingBase(const C* name_value,
				const BomBytes& bom_value,
				Endian endian_value,
				uint8_t bom_size_value) noexcept
				: name(name_value), bom(bom_value), endian(endian_value), bom_size(bom_size_value)
			{}
		};

		const EncodingBase< 5, Bool::F> ASCII         = { W("ASCII")     	, NO_BOM       , Endian(Endian::NOT_APPLICABLE), 0};
		const EncodingBase< 5, Bool::F> UTF_8         = { W("UTF-8")     	, NO_BOM       , Endian(Endian::NOT_APPLICABLE), 0};
		const EncodingBase< 9, Bool::T> UTF_8_BOM     = { W("UTF-8-BOM") 	, BOM_UTF_8    , Endian(Endian::NOT_APPLICABLE), 3};
		const EncodingBase< 9, Bool::F> UTF_16_BE     = { W("UTF-16-BE") 	, NO_BOM       , Endian(Endian::BIG)           , 2};
		const EncodingBase<13, Bool::T> UTF_16_BE_BOM = { W("UTF-16-BE-BOM"), BOM_UTF_16_BE, Endian(Endian::BIG)           , 2};
		const EncodingBase< 9, Bool::F> UTF_16_LE     = { W("UTF-16-LE") 	, NO_BOM       , Endian(Endian::LITTLE)        , 2};
		const EncodingBase<13, Bool::T> UTF_16_LE_BOM = { W("UTF-16-LE-BOM"), BOM_UTF_16_LE, Endian(Endian::LITTLE)        , 2};
		const EncodingBase< 9, Bool::F> UTF_32_BE     = { W("UTF-32-BE") 	, NO_BOM       , Endian(Endian::BIG)           , 4};
		const EncodingBase<13, Bool::T> UTF_32_BE_BOM = { W("UTF-32-BE-BOM"), BOM_UTF_32_BE, Endian(Endian::BIG)           , 4};
		const EncodingBase< 9, Bool::F> UTF_32_LE     = { W("UTF-32-LE") 	, NO_BOM       , Endian(Endian::LITTLE)        , 4};
		const EncodingBase<13, Bool::T> UTF_32_LE_BOM = { W("UTF-32-LE-BOM"), BOM_UTF_32_LE, Endian(Endian::LITTLE)        , 4};
			}; // namespace cpplib
} // namespace pensar_digital

#endif // ENCODE_HPP
