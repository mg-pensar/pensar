// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_SEARCH_DAO_HPP
#define CONTACT_SEARCH_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief DTO for full-text search results.
struct SearchResultDTO
{
    std::string entity_type;     // "Person" or "OrgUnit"
    int64_t     entity_id  = 0;
    std::string display_name;
    std::string keywords;
    float       rank       = 0.0f;
};

/// \brief DTO for person search results (more detailed).
struct PersonSearchResultDTO
{
    int64_t     id = 0;
    std::string first_name;
    std::string last_name;
    std::string keywords;
    float       rank = 0.0f;
};

/// \brief DTO for org unit search results (more detailed).
struct OrgSearchResultDTO
{
    int64_t     id = 0;
    std::string name;
    std::string org_type;
    std::string keywords;
    float       rank = 0.0f;
};

/// \brief Full-text and keyword search operations via SOCI.
///
/// Wraps the server-side search functions fn_search_persons,
/// fn_search_orgunits, and fn_search_contacts.
class SearchDAO
{
public:
    explicit SearchDAO(DbConnection& conn) : conn_(conn) {}

    /// Search persons by free-text query using websearch_to_tsquery.
    std::vector<PersonSearchResultDTO> search_persons(
        const std::string& query,
        int limit = 50,
        const std::string& ts_config = "simple")
    {
        auto s = conn_.session();

        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, first_name, last_name, "
                "COALESCE(keywords,''), rank "
                "FROM fn_search_persons(:q, :lim, :cfg::regconfig)",
                soci::use(query,     "q"),
                soci::use(limit,     "lim"),
                soci::use(ts_config, "cfg"));

        std::vector<PersonSearchResultDTO> result;
        for (auto& r : rs)
        {
            PersonSearchResultDTO dto;
            dto.id         = r.get<long long>(0);
            dto.first_name = r.get<std::string>(1);
            dto.last_name  = r.get<std::string>(2);
            dto.keywords   = r.get<std::string>(3);
            dto.rank       = static_cast<float>(r.get<double>(4));
            result.push_back(std::move(dto));
        }
        return result;
    }

    /// Search org units by free-text query.
    std::vector<OrgSearchResultDTO> search_orgunits(
        const std::string& query,
        int limit = 50,
        const std::string& ts_config = "simple")
    {
        auto s = conn_.session();

        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, name, org_type, "
                "COALESCE(keywords,''), rank "
                "FROM fn_search_orgunits(:q, :lim, :cfg::regconfig)",
                soci::use(query,     "q"),
                soci::use(limit,     "lim"),
                soci::use(ts_config, "cfg"));

        std::vector<OrgSearchResultDTO> result;
        for (auto& r : rs)
        {
            OrgSearchResultDTO dto;
            dto.id       = r.get<long long>(0);
            dto.name     = r.get<std::string>(1);
            dto.org_type = r.get<std::string>(2);
            dto.keywords = r.get<std::string>(3);
            dto.rank     = static_cast<float>(r.get<double>(4));
            result.push_back(std::move(dto));
        }
        return result;
    }

    /// Unified contact search (persons + org units).
    std::vector<SearchResultDTO> search_contacts(
        const std::string& query,
        int limit = 50,
        const std::string& ts_config = "simple")
    {
        auto s = conn_.session();

        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT entity_type, entity_id, display_name, "
                "COALESCE(keywords,''), rank "
                "FROM fn_search_contacts(:q, :lim, :cfg::regconfig)",
                soci::use(query,     "q"),
                soci::use(limit,     "lim"),
                soci::use(ts_config, "cfg"));

        std::vector<SearchResultDTO> result;
        for (auto& r : rs)
        {
            SearchResultDTO dto;
            dto.entity_type  = r.get<std::string>(0);
            dto.entity_id    = r.get<long long>(1);
            dto.display_name = r.get<std::string>(2);
            dto.keywords     = r.get<std::string>(3);
            dto.rank         = static_cast<float>(r.get<double>(4));
            result.push_back(std::move(dto));
        }
        return result;
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_SEARCH_DAO_HPP
