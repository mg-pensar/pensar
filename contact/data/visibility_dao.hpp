// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_VISIBILITY_DAO_HPP
#define CONTACT_VISIBILITY_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief DTO for T_DataVisibility rows.
struct DataVisibilityDTO
{
    int64_t     id                 = 0;
    int64_t     owner_person_id    = 0;
    std::string data_category;           // Phone, Email, Address, Personal, All
    int64_t     viewer_person_id   = 0;  // 0 → NULL
    int64_t     viewer_role_id     = 0;
    int64_t     viewer_org_unit_id = 0;
    bool        can_view           = true;
    bool        can_edit           = false;
};

/// \brief Access control queries via SOCI.
class VisibilityDAO
{
public:
    explicit VisibilityDAO(DbConnection& conn) : conn_(conn) {}

    /// Insert a new visibility rule. Exactly one viewer field must be non-zero.
    int64_t insert(const DataVisibilityDTO& v, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        long long v_owner = static_cast<long long>(v.owner_person_id);
        int v_cv = v.can_view ? 1 : 0;
        int v_ce = v.can_edit ? 1 : 0;

        // Determine which viewer field to set
        if (v.viewer_person_id != 0)
        {
            long long v_vp = static_cast<long long>(v.viewer_person_id);
            s.sql() <<
                "INSERT INTO t_datavisibility"
                " (owner_person_id, data_category, viewer_person_id,"
                "  viewer_role_id, viewer_org_unit_id, can_view, can_edit)"
                " VALUES (:owner, :cat, :vp, NULL, NULL, :cv::boolean, :ce::boolean)"
                " RETURNING id",
                soci::use(v_owner,         "owner"),
                soci::use(v.data_category, "cat"),
                soci::use(v_vp,            "vp"),
                soci::use(v_cv,            "cv"),
                soci::use(v_ce,            "ce"),
                soci::into(new_id);
        }
        else if (v.viewer_role_id != 0)
        {
            long long v_vr = static_cast<long long>(v.viewer_role_id);
            s.sql() <<
                "INSERT INTO t_datavisibility"
                " (owner_person_id, data_category, viewer_person_id,"
                "  viewer_role_id, viewer_org_unit_id, can_view, can_edit)"
                " VALUES (:owner, :cat, NULL, :vr, NULL, :cv::boolean, :ce::boolean)"
                " RETURNING id",
                soci::use(v_owner,         "owner"),
                soci::use(v.data_category, "cat"),
                soci::use(v_vr,            "vr"),
                soci::use(v_cv,            "cv"),
                soci::use(v_ce,            "ce"),
                soci::into(new_id);
        }
        else if (v.viewer_org_unit_id != 0)
        {
            long long v_vo = static_cast<long long>(v.viewer_org_unit_id);
            s.sql() <<
                "INSERT INTO t_datavisibility"
                " (owner_person_id, data_category, viewer_person_id,"
                "  viewer_role_id, viewer_org_unit_id, can_view, can_edit)"
                " VALUES (:owner, :cat, NULL, NULL, :vo, :cv::boolean, :ce::boolean)"
                " RETURNING id",
                soci::use(v_owner,         "owner"),
                soci::use(v.data_category, "cat"),
                soci::use(v_vo,            "vo"),
                soci::use(v_cv,            "cv"),
                soci::use(v_ce,            "ce"),
                soci::into(new_id);
        }

        return static_cast<int64_t>(new_id);
    }

    /// Get all visibility rules for a person's data.
    std::vector<DataVisibilityDTO> rules_for_owner(int64_t owner_person_id)
    {
        auto s = conn_.session();
        long long v_oid = static_cast<long long>(owner_person_id);
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, owner_person_id, data_category, "
                "COALESCE(viewer_person_id,0), COALESCE(viewer_role_id,0), "
                "COALESCE(viewer_org_unit_id,0), can_view, can_edit "
                "FROM t_datavisibility WHERE owner_person_id = :oid ORDER BY id",
                soci::use(v_oid, "oid"));

        std::vector<DataVisibilityDTO> result;
        for (auto& r : rs)
        {
            DataVisibilityDTO dto;
            dto.id                 = r.get<long long>(0);
            dto.owner_person_id    = r.get<long long>(1);
            dto.data_category      = r.get<std::string>(2);
            dto.viewer_person_id   = r.get<long long>(3);
            dto.viewer_role_id     = r.get<long long>(4);
            dto.viewer_org_unit_id = r.get<long long>(5);
            dto.can_view           = r.get<int>(6) != 0;
            dto.can_edit           = r.get<int>(7) != 0;
            result.push_back(std::move(dto));
        }
        return result;
    }

    /// Check if a viewer (person) can view a specific category of another person's data.
    bool can_view(int64_t owner_person_id,
                  int64_t viewer_person_id,
                  const std::string& data_category)
    {
        auto s = conn_.session();
        int count = 0;
        long long v_oid = static_cast<long long>(owner_person_id);
        long long v_vid = static_cast<long long>(viewer_person_id);

        s.sql() <<
            "SELECT COUNT(*) FROM t_datavisibility"
            " WHERE owner_person_id  = :oid"
            "   AND viewer_person_id = :vid"
            "   AND (data_category   = :cat OR data_category = 'All')"
            "   AND can_view = TRUE",
            soci::use(v_oid,         "oid"),
            soci::use(v_vid,         "vid"),
            soci::use(data_category, "cat"),
            soci::into(count);

        return count > 0;
    }

    /// Delete a visibility rule.
    bool remove(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_datavisibility WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_VISIBILITY_DAO_HPP
