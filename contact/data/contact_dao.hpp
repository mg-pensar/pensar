// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_CONTACT_DAO_HPP
#define CONTACT_CONTACT_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

// ─── Phone DTO ───────────────────────────────────────────────────────────────

struct PhoneDTO
{
    int64_t     id           = 0;
    int64_t     person_id    = 0;     // 0 → NULL
    int64_t     org_unit_id  = 0;     // 0 → NULL
    std::string country_code;
    std::string area_code;
    std::string number;
    std::string extension;
    bool        is_mobile    = false;
    int16_t     qualifier_id = 3;     // default: Other
    bool        is_preferred = false;
};

// ─── Email DTO ───────────────────────────────────────────────────────────────

struct EmailDTO
{
    int64_t     id           = 0;
    int64_t     person_id    = 0;
    int64_t     org_unit_id  = 0;
    std::string email;
    int16_t     qualifier_id = 3;
    bool        is_preferred = false;
    bool        is_verified  = false;
};

// ─── Address DTO ─────────────────────────────────────────────────────────────

struct AddressDTO
{
    int64_t     id             = 0;
    int64_t     person_id      = 0;
    int64_t     org_unit_id    = 0;
    std::string street_line1;
    std::string street_line2;
    std::string city;
    std::string state_province;
    std::string postal_code;
    std::string country;
    int16_t     qualifier_id   = 3;
    bool        is_preferred   = false;
    double      latitude       = 0.0;
    double      longitude      = 0.0;
};

// ─── Contact DAO — Phone, Email, Address CRUD ────────────────────────────────

class ContactDAO
{
public:
    explicit ContactDAO(DbConnection& conn) : conn_(conn) {}

    // ── Phone ────────────────────────────────────────────────────────────

    int64_t insert_phone(const PhoneDTO& p, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        int v_mobile    = p.is_mobile    ? 1 : 0;
        int v_preferred = p.is_preferred ? 1 : 0;
        int v_qid       = static_cast<int>(p.qualifier_id);

        if (p.person_id != 0 && p.org_unit_id == 0)
        {
            long long v_pid = static_cast<long long>(p.person_id);
            s.sql() <<
                "INSERT INTO t_phone"
                " (person_id, org_unit_id, country_code, area_code, number,"
                "  extension, is_mobile, qualifier_id, is_preferred)"
                " VALUES"
                " (:pid, NULL, :cc, :ac, :num, NULLIF(:ext,''),"
                "  :mob::boolean, :qid, :pref::boolean)"
                " RETURNING id",
                soci::use(v_pid,          "pid"),
                soci::use(p.country_code, "cc"),
                soci::use(p.area_code,    "ac"),
                soci::use(p.number,       "num"),
                soci::use(p.extension,    "ext"),
                soci::use(v_mobile,       "mob"),
                soci::use(v_qid,          "qid"),
                soci::use(v_preferred,    "pref"),
                soci::into(new_id);
        }
        else if (p.person_id == 0 && p.org_unit_id != 0)
        {
            long long v_oid = static_cast<long long>(p.org_unit_id);
            s.sql() <<
                "INSERT INTO t_phone"
                " (person_id, org_unit_id, country_code, area_code, number,"
                "  extension, is_mobile, qualifier_id, is_preferred)"
                " VALUES"
                " (NULL, :oid, :cc, :ac, :num, NULLIF(:ext,''),"
                "  :mob::boolean, :qid, :pref::boolean)"
                " RETURNING id",
                soci::use(v_oid,          "oid"),
                soci::use(p.country_code, "cc"),
                soci::use(p.area_code,    "ac"),
                soci::use(p.number,       "num"),
                soci::use(p.extension,    "ext"),
                soci::use(v_mobile,       "mob"),
                soci::use(v_qid,          "qid"),
                soci::use(v_preferred,    "pref"),
                soci::into(new_id);
        }

        return static_cast<int64_t>(new_id);
    }

    std::vector<PhoneDTO> phones_for_person(int64_t person_id)
    {
        auto s = conn_.session();
        long long v_pid = static_cast<long long>(person_id);
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, COALESCE(person_id,0), COALESCE(org_unit_id,0), "
                "country_code, COALESCE(area_code,''), number, "
                "COALESCE(extension,''), is_mobile, qualifier_id, is_preferred "
                "FROM t_phone WHERE person_id = :pid ORDER BY is_preferred DESC, id",
                soci::use(v_pid, "pid"));

        std::vector<PhoneDTO> result;
        for (auto& r : rs)
        {
            PhoneDTO p;
            p.id           = r.get<long long>(0);
            p.person_id    = r.get<long long>(1);
            p.org_unit_id  = r.get<long long>(2);
            p.country_code = r.get<std::string>(3);
            p.area_code    = r.get<std::string>(4);
            p.number       = r.get<std::string>(5);
            p.extension    = r.get<std::string>(6);
            p.is_mobile    = r.get<int>(7) != 0;
            p.qualifier_id = static_cast<int16_t>(r.get<int>(8));
            p.is_preferred = r.get<int>(9) != 0;
            result.push_back(std::move(p));
        }
        return result;
    }

    bool remove_phone(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_phone WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

    // ── Email ────────────────────────────────────────────────────────────

    int64_t insert_email(const EmailDTO& e, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        int v_preferred = e.is_preferred ? 1 : 0;
        int v_verified  = e.is_verified  ? 1 : 0;
        int v_qid       = static_cast<int>(e.qualifier_id);

        if (e.person_id != 0 && e.org_unit_id == 0)
        {
            long long v_pid = static_cast<long long>(e.person_id);
            s.sql() <<
                "INSERT INTO t_email"
                " (person_id, org_unit_id, email, qualifier_id, is_preferred, is_verified)"
                " VALUES (:pid, NULL, :email, :qid, :pref::boolean, :ver::boolean)"
                " RETURNING id",
                soci::use(v_pid,       "pid"),
                soci::use(e.email,     "email"),
                soci::use(v_qid,       "qid"),
                soci::use(v_preferred, "pref"),
                soci::use(v_verified,  "ver"),
                soci::into(new_id);
        }
        else if (e.person_id == 0 && e.org_unit_id != 0)
        {
            long long v_oid = static_cast<long long>(e.org_unit_id);
            s.sql() <<
                "INSERT INTO t_email"
                " (person_id, org_unit_id, email, qualifier_id, is_preferred, is_verified)"
                " VALUES (NULL, :oid, :email, :qid, :pref::boolean, :ver::boolean)"
                " RETURNING id",
                soci::use(v_oid,       "oid"),
                soci::use(e.email,     "email"),
                soci::use(v_qid,       "qid"),
                soci::use(v_preferred, "pref"),
                soci::use(v_verified,  "ver"),
                soci::into(new_id);
        }

        return static_cast<int64_t>(new_id);
    }

    std::vector<EmailDTO> emails_for_person(int64_t person_id)
    {
        auto s = conn_.session();
        long long v_pid = static_cast<long long>(person_id);
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, COALESCE(person_id,0), COALESCE(org_unit_id,0), "
                "email, qualifier_id, is_preferred, is_verified "
                "FROM t_email WHERE person_id = :pid ORDER BY is_preferred DESC, id",
                soci::use(v_pid, "pid"));

        std::vector<EmailDTO> result;
        for (auto& r : rs)
        {
            EmailDTO em;
            em.id           = r.get<long long>(0);
            em.person_id    = r.get<long long>(1);
            em.org_unit_id  = r.get<long long>(2);
            em.email        = r.get<std::string>(3);
            em.qualifier_id = static_cast<int16_t>(r.get<int>(4));
            em.is_preferred = r.get<int>(5) != 0;
            em.is_verified  = r.get<int>(6) != 0;
            result.push_back(std::move(em));
        }
        return result;
    }

    bool remove_email(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_email WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

    // ── Address ──────────────────────────────────────────────────────────

    int64_t insert_address(const AddressDTO& a, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;
        int v_preferred = a.is_preferred ? 1 : 0;
        int v_qid       = static_cast<int>(a.qualifier_id);

        if (a.person_id != 0 && a.org_unit_id == 0)
        {
            long long v_pid = static_cast<long long>(a.person_id);
            s.sql() <<
                "INSERT INTO t_address"
                " (person_id, org_unit_id, street_line1, street_line2, city,"
                "  state_province, postal_code, country, qualifier_id, is_preferred,"
                "  latitude, longitude)"
                " VALUES"
                " (:pid, NULL, :sl1, NULLIF(:sl2,''), :city,"
                "  NULLIF(:state,''), NULLIF(:pc,''), :ctry, :qid, :pref::boolean,"
                "  :lat, :lng)"
                " RETURNING id",
                soci::use(v_pid,            "pid"),
                soci::use(a.street_line1,   "sl1"),
                soci::use(a.street_line2,   "sl2"),
                soci::use(a.city,           "city"),
                soci::use(a.state_province, "state"),
                soci::use(a.postal_code,    "pc"),
                soci::use(a.country,        "ctry"),
                soci::use(v_qid,            "qid"),
                soci::use(v_preferred,      "pref"),
                soci::use(a.latitude,       "lat"),
                soci::use(a.longitude,      "lng"),
                soci::into(new_id);
        }
        else if (a.person_id == 0 && a.org_unit_id != 0)
        {
            long long v_oid = static_cast<long long>(a.org_unit_id);
            s.sql() <<
                "INSERT INTO t_address"
                " (person_id, org_unit_id, street_line1, street_line2, city,"
                "  state_province, postal_code, country, qualifier_id, is_preferred,"
                "  latitude, longitude)"
                " VALUES"
                " (NULL, :oid, :sl1, NULLIF(:sl2,''), :city,"
                "  NULLIF(:state,''), NULLIF(:pc,''), :ctry, :qid, :pref::boolean,"
                "  :lat, :lng)"
                " RETURNING id",
                soci::use(v_oid,            "oid"),
                soci::use(a.street_line1,   "sl1"),
                soci::use(a.street_line2,   "sl2"),
                soci::use(a.city,           "city"),
                soci::use(a.state_province, "state"),
                soci::use(a.postal_code,    "pc"),
                soci::use(a.country,        "ctry"),
                soci::use(v_qid,            "qid"),
                soci::use(v_preferred,      "pref"),
                soci::use(a.latitude,       "lat"),
                soci::use(a.longitude,      "lng"),
                soci::into(new_id);
        }

        return static_cast<int64_t>(new_id);
    }

    std::vector<AddressDTO> addresses_for_person(int64_t person_id)
    {
        auto s = conn_.session();
        long long v_pid = static_cast<long long>(person_id);
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, COALESCE(person_id,0), COALESCE(org_unit_id,0), "
                "street_line1, COALESCE(street_line2,''), city, "
                "COALESCE(state_province,''), COALESCE(postal_code,''), country, "
                "qualifier_id, is_preferred, "
                "COALESCE(latitude,0), COALESCE(longitude,0) "
                "FROM t_address WHERE person_id = :pid ORDER BY is_preferred DESC, id",
                soci::use(v_pid, "pid"));

        std::vector<AddressDTO> result;
        for (auto& r : rs)
        {
            AddressDTO a;
            a.id             = r.get<long long>(0);
            a.person_id      = r.get<long long>(1);
            a.org_unit_id    = r.get<long long>(2);
            a.street_line1   = r.get<std::string>(3);
            a.street_line2   = r.get<std::string>(4);
            a.city           = r.get<std::string>(5);
            a.state_province = r.get<std::string>(6);
            a.postal_code    = r.get<std::string>(7);
            a.country        = r.get<std::string>(8);
            a.qualifier_id   = static_cast<int16_t>(r.get<int>(9));
            a.is_preferred   = r.get<int>(10) != 0;
            a.latitude       = r.get<double>(11);
            a.longitude      = r.get<double>(12);
            result.push_back(std::move(a));
        }
        return result;
    }

    bool remove_address(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_address WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_CONTACT_DAO_HPP
