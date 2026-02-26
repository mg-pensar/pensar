// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_ORG_DAO_HPP
#define CONTACT_ORG_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief DTO for T_OrgUnit rows.
struct OrgUnitDTO
{
    int64_t     id        = 0;
    int64_t     parent_id = 0;       // 0 → NULL (root)
    std::string org_type;            // Group, Company, Department, Division, Team
    std::string federal_tax_id;
    bool        is_active = true;
    std::string keywords;
};

/// \brief DTO for T_LocalizedOrgUnit rows.
struct LocalizedOrgUnitDTO
{
    int64_t     id          = 0;
    int64_t     org_unit_id = 0;
    int16_t     locale_id   = 0;
    std::string name;
};

/// \brief DTO for hierarchy query results.
struct OrgHierarchyDTO
{
    int64_t     id        = 0;
    int64_t     parent_id = 0;
    std::string name;
    std::string org_type;
    std::string keywords;
    int         depth     = 0;
};

/// \brief OrgUnit CRUD and hierarchy operations via SOCI.
class OrgDAO
{
public:
    explicit OrgDAO(DbConnection& conn) : conn_(conn) {}

    /// Insert a new org unit. Returns the auto-generated id.
    int64_t insert(const OrgUnitDTO& o, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        int v_active = o.is_active ? 1 : 0;

        if (o.parent_id == 0)
        {
            s.sql() <<
                "INSERT INTO t_orgunit (parent_id, org_type, federal_tax_id, is_active, keywords)"
                " VALUES (NULL, :org_type, NULLIF(:tax_id,''), :active::boolean, NULLIF(:keywords,''))"
                " RETURNING id",
                soci::use(o.org_type,       "org_type"),
                soci::use(o.federal_tax_id, "tax_id"),
                soci::use(v_active,         "active"),
                soci::use(o.keywords,       "keywords"),
                soci::into(new_id);
        }
        else
        {
            long long v_parent = static_cast<long long>(o.parent_id);
            s.sql() <<
                "INSERT INTO t_orgunit (parent_id, org_type, federal_tax_id, is_active, keywords)"
                " VALUES (:parent_id, :org_type, NULLIF(:tax_id,''), :active::boolean, NULLIF(:keywords,''))"
                " RETURNING id",
                soci::use(v_parent,         "parent_id"),
                soci::use(o.org_type,       "org_type"),
                soci::use(o.federal_tax_id, "tax_id"),
                soci::use(v_active,         "active"),
                soci::use(o.keywords,       "keywords"),
                soci::into(new_id);
        }

        return static_cast<int64_t>(new_id);
    }

    /// Insert a localized name for an org unit.
    int64_t insert_localized(const LocalizedOrgUnitDTO& loc, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        long long v_org_unit_id = static_cast<long long>(loc.org_unit_id);
        int v_locale_id = static_cast<int>(loc.locale_id);

        s.sql() <<
            "INSERT INTO t_localizedorgunit (org_unit_id, locale_id, name)"
            " VALUES (:org_unit_id, :locale_id, :name)"
            " RETURNING id",
            soci::use(v_org_unit_id, "org_unit_id"),
            soci::use(v_locale_id,   "locale_id"),
            soci::use(loc.name,      "name"),
            soci::into(new_id);

        return static_cast<int64_t>(new_id);
    }

    /// Find org unit by id.
    std::optional<OrgUnitDTO> find_by_id(int64_t id)
    {
        auto s = conn_.session();
        soci::row r;
        long long lid = static_cast<long long>(id);

        s.sql() <<
            "SELECT id, COALESCE(parent_id, 0), org_type,"
            " COALESCE(federal_tax_id, ''), is_active,"
            " COALESCE(keywords, '')"
            " FROM t_orgunit WHERE id = :id",
            soci::use(lid, "id"),
            soci::into(r);

        if (!s.sql().got_data())
            return std::nullopt;

        OrgUnitDTO o;
        o.id             = r.get<long long>(0);
        o.parent_id      = r.get<long long>(1);
        o.org_type       = r.get<std::string>(2);
        o.federal_tax_id = r.get<std::string>(3);
        o.is_active      = r.get<int>(4) != 0;
        o.keywords       = r.get<std::string>(5);
        return o;
    }

    /// Update an existing org unit. Returns true if a row was updated.
    bool update(const OrgUnitDTO& o, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(o.id);
        long long ret_id = 0;
        int v_active = o.is_active ? 1 : 0;

        if (o.parent_id == 0)
        {
            s.sql() <<
                "UPDATE t_orgunit SET"
                " parent_id = NULL,"
                " org_type = :org_type,"
                " federal_tax_id = NULLIF(:tax_id,''),"
                " is_active = :active::boolean,"
                " keywords = NULLIF(:keywords,'')"
                " WHERE id = :id RETURNING id",
                soci::use(o.org_type,       "org_type"),
                soci::use(o.federal_tax_id, "tax_id"),
                soci::use(v_active,         "active"),
                soci::use(o.keywords,       "keywords"),
                soci::use(lid,              "id"),
                soci::into(ret_id);
        }
        else
        {
            long long v_parent = static_cast<long long>(o.parent_id);
            s.sql() <<
                "UPDATE t_orgunit SET"
                " parent_id = :parent_id,"
                " org_type = :org_type,"
                " federal_tax_id = NULLIF(:tax_id,''),"
                " is_active = :active::boolean,"
                " keywords = NULLIF(:keywords,'')"
                " WHERE id = :id RETURNING id",
                soci::use(v_parent,         "parent_id"),
                soci::use(o.org_type,       "org_type"),
                soci::use(o.federal_tax_id, "tax_id"),
                soci::use(v_active,         "active"),
                soci::use(o.keywords,       "keywords"),
                soci::use(lid,              "id"),
                soci::into(ret_id);
        }

        return s.sql().got_data();
    }

    /// Delete an org unit by id. Returns true if a row was deleted.
    bool remove(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_orgunit WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

    /// Get org hierarchy using the live recursive function.
    std::vector<OrgHierarchyDTO> hierarchy(const std::string& locale = "pt-BR")
    {
        auto s = conn_.session({}, locale);

        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, COALESCE(parent_id, 0), name, org_type, "
                "COALESCE(keywords, ''), depth "
                "FROM fn_org_hierarchy(:loc)",
                soci::use(locale, "loc"));

        std::vector<OrgHierarchyDTO> result;
        for (auto& r : rs)
        {
            OrgHierarchyDTO h;
            h.id        = r.get<long long>(0);
            h.parent_id = r.get<long long>(1);
            h.name      = r.get<std::string>(2);
            h.org_type  = r.get<std::string>(3);
            h.keywords  = r.get<std::string>(4);
            h.depth     = r.get<int>(5);
            result.push_back(std::move(h));
        }
        return result;
    }

    /// Find children of a given parent (direct children only).
    std::vector<OrgUnitDTO> find_children(int64_t parent_id)
    {
        auto s = conn_.session();
        long long v_pid = static_cast<long long>(parent_id);

        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, COALESCE(parent_id, 0), org_type, "
                "COALESCE(federal_tax_id, ''), is_active, COALESCE(keywords, '') "
                "FROM t_orgunit WHERE parent_id = :pid ORDER BY id",
                soci::use(v_pid, "pid"));

        std::vector<OrgUnitDTO> result;
        for (auto& r : rs)
        {
            OrgUnitDTO o;
            o.id             = r.get<long long>(0);
            o.parent_id      = r.get<long long>(1);
            o.org_type       = r.get<std::string>(2);
            o.federal_tax_id = r.get<std::string>(3);
            o.is_active      = r.get<int>(4) != 0;
            o.keywords       = r.get<std::string>(5);
            result.push_back(std::move(o));
        }
        return result;
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_ORG_DAO_HPP
