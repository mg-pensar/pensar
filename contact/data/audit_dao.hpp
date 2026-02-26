// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_AUDIT_DAO_HPP
#define CONTACT_AUDIT_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief DTO for audit log entries.
struct AuditLogDTO
{
    std::string source_table;   // only populated by search_all
    int64_t     log_id    = 0;
    std::string action;         // C, U, D
    std::string action_by;
    std::string action_at;      // ISO-8601 timestamptz as string
    std::string old_data;       // JSONB as string
    std::string new_data;       // JSONB as string
};

/// \brief DTO for entity timestamp derivation.
struct EntityTimestampDTO
{
    std::string created_at;
    std::string created_by;
    std::string updated_at;
    std::string updated_by;
};

/// \brief Audit log browsing, search and purge operations via SOCI.
class AuditDAO
{
public:
    explicit AuditDAO(DbConnection& conn) : conn_(conn) {}

    /// Search a single audit log table with optional filters.
    std::vector<AuditLogDTO> search(
        const std::string& log_table,
        const std::string& action     = {},
        const std::string& action_by  = {},
        const std::string& from       = {},
        const std::string& to         = {})
    {
        auto s = conn_.session();

        // Build WHERE clause dynamically based on provided filters
        std::string where = "WHERE 1=1";
        if (!action.empty())    where += " AND action = '" + action + "'";
        if (!action_by.empty()) where += " AND action_by = '" + action_by + "'";
        if (!from.empty())      where += " AND action_at >= '" + from + "'::timestamptz";
        if (!to.empty())        where += " AND action_at < '"  + to   + "'::timestamptz";

        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT log_id, action, action_by, action_at::text, "
                "COALESCE(old_data::text,'{}'), COALESCE(new_data::text,'{}') "
                "FROM " + log_table + " " + where + " ORDER BY action_at DESC");

        std::vector<AuditLogDTO> result;
        for (auto& r : rs)
        {
            AuditLogDTO dto;
            dto.log_id    = r.get<long long>(0);
            dto.action    = r.get<std::string>(1);
            dto.action_by = r.get<std::string>(2);
            dto.action_at = r.get<std::string>(3);
            dto.old_data  = r.get<std::string>(4);
            dto.new_data  = r.get<std::string>(5);
            result.push_back(std::move(dto));
        }
        return result;
    }

    /// Search across all audit log tables.
    std::vector<AuditLogDTO> search_all(
        const std::string& action     = {},
        const std::string& action_by  = {},
        const std::string& from       = {},
        const std::string& to         = {},
        int limit = 500)
    {
        auto s = conn_.session();
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT source_table, log_id, action, action_by, "
                "action_at::text, COALESCE(old_data::text,'{}'), "
                "COALESCE(new_data::text,'{}') "
                "FROM fn_search_all_audit_logs("
                "NULLIF(:act,'')::char, NULLIF(:aby,''), "
                "NULLIF(:fr,'')::timestamptz, NULLIF(:to2,'')::timestamptz, :lim)",
                soci::use(action,    "act"),
                soci::use(action_by, "aby"),
                soci::use(from,      "fr"),
                soci::use(to,        "to2"),
                soci::use(limit,     "lim"));

        std::vector<AuditLogDTO> result;
        for (auto& r : rs)
        {
            AuditLogDTO dto;
            dto.source_table = r.get<std::string>(0);
            dto.log_id       = r.get<long long>(1);
            dto.action        = r.get<std::string>(2);
            dto.action_by    = r.get<std::string>(3);
            dto.action_at    = r.get<std::string>(4);
            dto.old_data     = r.get<std::string>(5);
            dto.new_data     = r.get<std::string>(6);
            result.push_back(std::move(dto));
        }
        return result;
    }

    /// Purge audit log entries older than a given timestamp.
    /// Returns the number of deleted rows.
    int64_t purge(const std::string& log_table, const std::string& before)
    {
        auto s = conn_.session();
        long long deleted = 0;

        s.sql() << "SELECT fn_purge_audit_log(:tbl, :before::timestamptz)",
            soci::use(log_table, "tbl"),
            soci::use(before,    "before"),
            soci::into(deleted);

        return static_cast<int64_t>(deleted);
    }

    /// Derive created_at/updated_at timestamps from the audit log.
    std::optional<EntityTimestampDTO> entity_timestamps(
        const std::string& log_table,
        int64_t entity_id)
    {
        auto s = conn_.session();
        soci::row r;
        long long v_eid = static_cast<long long>(entity_id);

        s.sql() << R"(
            SELECT COALESCE(created_at::text,''),
                   COALESCE(created_by,''),
                   COALESCE(updated_at::text,''),
                   COALESCE(updated_by,'')
              FROM fn_entity_timestamps(:tbl, :eid)
        )",
            soci::use(log_table, "tbl"),
            soci::use(v_eid, "eid"),
            soci::into(r);

        if (!s.sql().got_data())
            return std::nullopt;

        EntityTimestampDTO dto;
        dto.created_at = r.get<std::string>(0);
        dto.created_by = r.get<std::string>(1);
        dto.updated_at = r.get<std::string>(2);
        dto.updated_by = r.get<std::string>(3);
        return dto;
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_AUDIT_DAO_HPP
