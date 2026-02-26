// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_ROLE_DAO_HPP
#define CONTACT_ROLE_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief DTO for T_Role rows.
struct RoleDTO
{
    int64_t     id          = 0;
    int64_t     org_unit_id = 0;
    bool        is_active   = true;
};

/// \brief DTO for T_LocalizedRole rows.
struct LocalizedRoleDTO
{
    int64_t     id          = 0;
    int64_t     role_id     = 0;
    int16_t     locale_id   = 0;
    std::string name;
    std::string description;
};

/// \brief DTO for T_PersonOrgRelation rows.
struct PersonOrgRelationDTO
{
    int64_t     id           = 0;
    int64_t     person_id    = 0;
    int64_t     org_unit_id  = 0;
    std::string relationship;    // Employee, Trainee, MinorApprentice, ThirdParty
    std::string start_date;      // ISO-8601
    std::string end_date;
    bool        is_active    = true;
};

/// \brief DTO for T_PersonRole rows.
struct PersonRoleDTO
{
    int64_t     id            = 0;
    int64_t     person_id     = 0;
    int64_t     role_id       = 0;
    std::string assigned_date;
    std::string end_date;
    bool        is_primary    = false;
};

/// \brief Role, PersonOrgRelation and PersonRole CRUD via SOCI.
class RoleDAO
{
public:
    explicit RoleDAO(DbConnection& conn) : conn_(conn) {}

    // ── Role ─────────────────────────────────────────────────────────────

    int64_t insert_role(const RoleDTO& r, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        int v_active = r.is_active ? 1 : 0;

        if (r.org_unit_id == 0)
        {
            s.sql() <<
                "INSERT INTO t_role (org_unit_id, is_active)"
                " VALUES (NULL, :active::boolean)"
                " RETURNING id",
                soci::use(v_active, "active"),
                soci::into(new_id);
        }
        else
        {
            long long v_oid = static_cast<long long>(r.org_unit_id);
            s.sql() <<
                "INSERT INTO t_role (org_unit_id, is_active)"
                " VALUES (:oid, :active::boolean)"
                " RETURNING id",
                soci::use(v_oid,    "oid"),
                soci::use(v_active, "active"),
                soci::into(new_id);
        }

        return static_cast<int64_t>(new_id);
    }

    int64_t insert_localized_role(const LocalizedRoleDTO& lr, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        long long v_rid  = static_cast<long long>(lr.role_id);
        int v_lid        = static_cast<int>(lr.locale_id);

        s.sql() <<
            "INSERT INTO t_localizedrole (role_id, locale_id, name, description)"
            " VALUES (:rid, :lid, :name, NULLIF(:desc,''))"
            " RETURNING id",
            soci::use(v_rid,          "rid"),
            soci::use(v_lid,          "lid"),
            soci::use(lr.name,        "name"),
            soci::use(lr.description, "desc"),
            soci::into(new_id);

        return static_cast<int64_t>(new_id);
    }

    std::optional<RoleDTO> find_role_by_id(int64_t id)
    {
        auto s = conn_.session();
        soci::row r;
        long long lid = static_cast<long long>(id);

        s.sql() <<
            "SELECT id, COALESCE(org_unit_id, 0), is_active"
            " FROM t_role WHERE id = :id",
            soci::use(lid, "id"),
            soci::into(r);

        if (!s.sql().got_data())
            return std::nullopt;

        RoleDTO dto;
        dto.id          = r.get<long long>(0);
        dto.org_unit_id = r.get<long long>(1);
        dto.is_active   = r.get<int>(2) != 0;
        return dto;
    }

    bool remove_role(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_role WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

    // ── PersonOrgRelation ────────────────────────────────────────────────

    int64_t insert_person_org_relation(const PersonOrgRelationDTO& por,
                                       const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        long long v_pid  = static_cast<long long>(por.person_id);
        long long v_oid  = static_cast<long long>(por.org_unit_id);
        int v_active     = por.is_active ? 1 : 0;

        s.sql() <<
            "INSERT INTO t_personorgrelation"
            " (person_id, org_unit_id, relationship, start_date, end_date, is_active)"
            " VALUES"
            " (:pid, :oid, :rel::relationship_type, :sd::date,"
            "  NULLIF(:ed, '')::date, :active::boolean)"
            " RETURNING id",
            soci::use(v_pid,            "pid"),
            soci::use(v_oid,            "oid"),
            soci::use(por.relationship, "rel"),
            soci::use(por.start_date,   "sd"),
            soci::use(por.end_date,     "ed"),
            soci::use(v_active,         "active"),
            soci::into(new_id);

        return static_cast<int64_t>(new_id);
    }

    std::vector<PersonOrgRelationDTO> relations_for_person(int64_t person_id)
    {
        auto s = conn_.session();
        long long v_pid = static_cast<long long>(person_id);
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, person_id, org_unit_id, relationship::text, "
                "start_date::text, COALESCE(end_date::text,''), is_active "
                "FROM t_personorgrelation WHERE person_id = :pid ORDER BY start_date DESC",
                soci::use(v_pid, "pid"));

        std::vector<PersonOrgRelationDTO> result;
        for (auto& r : rs)
        {
            PersonOrgRelationDTO dto;
            dto.id           = r.get<long long>(0);
            dto.person_id    = r.get<long long>(1);
            dto.org_unit_id  = r.get<long long>(2);
            dto.relationship = r.get<std::string>(3);
            dto.start_date   = r.get<std::string>(4);
            dto.end_date     = r.get<std::string>(5);
            dto.is_active    = r.get<int>(6) != 0;
            result.push_back(std::move(dto));
        }
        return result;
    }

    // ── PersonRole ───────────────────────────────────────────────────────

    int64_t assign_role(const PersonRoleDTO& pr, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id  = 0;
        long long v_pid   = static_cast<long long>(pr.person_id);
        long long v_rid   = static_cast<long long>(pr.role_id);
        int v_primary     = pr.is_primary ? 1 : 0;

        s.sql() <<
            "INSERT INTO t_personrole"
            " (person_id, role_id, assigned_date, end_date, is_primary)"
            " VALUES"
            " (:pid, :rid,"
            "  COALESCE(NULLIF(:ad, ''), CURRENT_DATE::text)::date,"
            "  NULLIF(:ed, '')::date, :primary::boolean)"
            " RETURNING id",
            soci::use(v_pid,            "pid"),
            soci::use(v_rid,            "rid"),
            soci::use(pr.assigned_date, "ad"),
            soci::use(pr.end_date,      "ed"),
            soci::use(v_primary,        "primary"),
            soci::into(new_id);

        return static_cast<int64_t>(new_id);
    }

    std::vector<PersonRoleDTO> roles_for_person(int64_t person_id)
    {
        auto s = conn_.session();
        long long v_pid = static_cast<long long>(person_id);
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, person_id, role_id, assigned_date::text, "
                "COALESCE(end_date::text,''), is_primary "
                "FROM t_personrole WHERE person_id = :pid ORDER BY is_primary DESC, id",
                soci::use(v_pid, "pid"));

        std::vector<PersonRoleDTO> result;
        for (auto& r : rs)
        {
            PersonRoleDTO dto;
            dto.id            = r.get<long long>(0);
            dto.person_id     = r.get<long long>(1);
            dto.role_id       = r.get<long long>(2);
            dto.assigned_date = r.get<std::string>(3);
            dto.end_date      = r.get<std::string>(4);
            dto.is_primary    = r.get<int>(5) != 0;
            result.push_back(std::move(dto));
        }
        return result;
    }

    bool unassign_role(int64_t person_role_id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(person_role_id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_personrole WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_ROLE_DAO_HPP
