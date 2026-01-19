// author : Mauricio Gomes

#ifndef CLASS_INFO_HPP
#define CLASS_INFO_HPP

namespace pensar_digital
{
	namespace cpplib
	{
        struct ClassInfo
        {
            inline static const size_t MAX_IDENTIFIER_SIZE = 100; ///< Maximum size for identifier strings.
            using Identifier = CS<0, MAX_IDENTIFIER_SIZE>; ///< Type for identifiers, using a fixed-size character string.
            Identifier mnamespace;
            Identifier mclass_name;
            VersionInt mpublic_interface_version;
            VersionInt mprotected_interface_version;
            VersionInt mprivate_interface_version;

            inline static const VersionInt NULL_VERSION = -1; ///< Null version constant.

            ClassInfo(const S& ns = EMPTY, const S& cn = EMPTY, VersionInt pub_ver = NULL_VERSION, VersionInt pro_ver = NULL_VERSION, VersionInt pri_ver = NULL_VERSION) noexcept
                : mnamespace(ns),
                mclass_name(cn),
                mpublic_interface_version(pub_ver),
                mprotected_interface_version(pro_ver),
                mprivate_interface_version(pri_ver) {
            }

            inline void write(MemoryBuffer& mb) const noexcept
            {
                mb.write((BytePtr)this, sizeof(ClassInfo));
            }

            inline void read(MemoryBuffer& mb)
            {
                mb.read_known_size((BytePtr)this, sizeof(ClassInfo));
            }

            MemoryBuffer::Ptr bytes() const noexcept
            {
                MemoryBuffer::Ptr mb = std::make_unique<MemoryBuffer>(sizeof(ClassInfo));
                write(*mb);
                return mb;
            }

            inline bool operator==(const ClassInfo& other) const noexcept
            {
                return equal<ClassInfo>(*this, other);
            }

            inline bool operator!=(const ClassInfo& other) const noexcept
            {
                return !equal<ClassInfo>(*this, other);
            }

            inline void test_class_name_and_version(MemoryBuffer& mb) const
            {
                if (mb.size() < sizeof(ClassInfo))
                    log_throw(W("MemoryBuffer size is smaller than ClassInfo size."));

                ClassInfo info;
                info.read(mb);

                if (info != *this)
                    log_throw(W("Version mismatch."));
            }

            inline const S to_s() const noexcept
            {
                SStream ss;
                ss << mnamespace << W("::") << mclass_name
                    << W(" v") << mpublic_interface_version
                    << W(".") << mprotected_interface_version
                    << W(".") << mprivate_interface_version;
                return ss.str();
            }

            inline std::istream& binary_read(std::istream& is, const std::endian& byte_order)
            {
                return is.read((char*)(this), sizeof(ClassInfo));
            }

            inline std::ostream& binary_write(std::ostream& os, const std::endian& byte_order) const
            {
                return os.write((const char*)this, sizeof(ClassInfo));
            }

            inline void test_class_name_and_version(std::istream& is, const std::endian& byte_order = std::endian::native) const
            {
                ClassInfo info;
                info.binary_read(is, byte_order);
                if (info != *this)
                    log_throw(W("Version mismatch."));
            }
        };

        // concept HasClassInfo 
        template <typename T>
        concept HasClassInfo = requires
        {
            // Check for static const ClassInfo* members
            { T::INFO } -> std::same_as<const ClassInfo>;
            // Ensure they are static and constant (implicit in the requires clause with ::)
                requires std::is_same_v<decltype(T::INFO), const ClassInfo>;
        };
    } // namespace cpplib
} // namespace pensar_digital
#endif // CLASS_INFO_HPP
