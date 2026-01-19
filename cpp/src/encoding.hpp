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
		};

		const EncodingBase< 5, Bool::F> ASCII         = { Encoding (),     "ASCII"    , NO_BOM        , Endian::NOT_APPLICABLE, 0};
		const EncodingBase< 5, Bool::F> UTF_8         = { Encoding (),     "UTF-8"    , NO_BOM        , Endian::NOT_APPLICABLE, 0};
		const EncodingBase< 9, Bool::T> UTF_8_BOM     = { Encoding (), "UTF-8-BOM"    , BOM_UTF_8     , Endian::NOT_APPLICABLE, 3};
		const EncodingBase< 9, Bool::F> UTF_16_BE     = { Encoding (), "UTF-16-BE"    , NO_BOM        , Endian::BIG           , 2};
		const EncodingBase<13, Bool::T> UTF_16_BE_BOM = { Encoding (), "UTF-16-BE-BOM", BOM_UTF_16_BE , Endian::BIG           , 2};
		const EncodingBase< 9, Bool::F> UTF_16_LE     = { Encoding (), "UTF-16-LE"    , NO_BOM        , Endian::LITTLE        , 2};
		const EncodingBase<13, Bool::T> UTF_16_LE_BOM = { Encoding (), "UTF-16-LE-BOM", BOM_UTF_16_LE , Endian::LITTLE        , 2};
		const EncodingBase< 9, Bool::F> UTF_32_BE     = { Encoding (), "UTF-32-BE"    , NO_BOM        , Endian::BIG           , 4};
		const EncodingBase<13, Bool::T> UTF_32_BE_BOM = { Encoding (), "UTF-32-BE-BOM", BOM_UTF_32_BE , Endian::BIG           , 4};
		const EncodingBase< 9, Bool::F> UTF_32_LE     = { Encoding (), "UTF-32-LE"    , NO_BOM        , Endian::LITTLE        , 4};
		const EncodingBase<13, Bool::T> UTF_32_LE_BOM = { Encoding (), "UTF-32-LE-BOM", BOM_UTF_32_LE, Endian::LITTLE         , 4};
			}; // namespace cpplib
} // namespace pensar_digital

#endif // ENCODE_HPP
