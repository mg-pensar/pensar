// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_LOCALE_DAO_HPP
#define CONTACT_LOCALE_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief DTO for T_Locale rows.
struct LocaleDTO
{
    int16_t     id         = 0;
    std::string code;               // BCP-47 e.g. "pt-BR"
    bool        is_default = false;
};

/// \brief Locale management operations via SOCI.
class LocaleDAO
{
public:
    explicit LocaleDAO(DbConnection& conn) : conn_(conn) {}

    /// List all locales.
    std::vector<LocaleDTO> list_all()
    {
        auto s = conn_.session();
        soci::rowset<soci::row> rs =
            (s.sql().prepare << "SELECT id, code, is_default FROM t_locale ORDER BY id");

        std::vector<LocaleDTO> result;
        for (auto& r : rs)
        {
            LocaleDTO dto;
            dto.id         = static_cast<int16_t>(r.get<int>(0));
            dto.code       = r.get<std::string>(1);
            dto.is_default = r.get<int>(2) != 0;
            result.push_back(std::move(dto));
        }
        return result;
    }

    /// Find locale by code (e.g. "pt-BR").
    std::optional<LocaleDTO> find_by_code(const std::string& code)
    {
        auto s = conn_.session();
        soci::row r;

        s.sql() << "SELECT id, code, is_default FROM t_locale WHERE code = :c",
            soci::use(code, "c"),
            soci::into(r);

        if (!s.sql().got_data())
            return std::nullopt;

        LocaleDTO dto;
        dto.id         = static_cast<int16_t>(r.get<int>(0));
        dto.code       = r.get<std::string>(1);
        dto.is_default = r.get<int>(2) != 0;
        return dto;
    }

    /// Get the default locale.
    std::optional<LocaleDTO> get_default()
    {
        auto s = conn_.session();
        soci::row r;

        s.sql() << "SELECT id, code, is_default FROM t_locale WHERE is_default = TRUE LIMIT 1",
            soci::into(r);

        if (!s.sql().got_data())
            return std::nullopt;

        LocaleDTO dto;
        dto.id         = static_cast<int16_t>(r.get<int>(0));
        dto.code       = r.get<std::string>(1);
        dto.is_default = true;
        return dto;
    }

    /// Refresh all locale-dependent materialized views.
    void refresh_materialized_views()
    {
        auto s = conn_.session();
        s.sql() << "SELECT fn_refresh_locale_mvs()";
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_LOCALE_DAO_HPP
