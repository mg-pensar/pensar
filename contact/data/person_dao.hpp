// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_PERSON_DAO_HPP
#define CONTACT_PERSON_DAO_HPP

#include "db_connection.hpp"

#include <soci/soci.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace pensar_digital::cpplib::contact::db
{

/// \brief Data Transfer Object for T_Person rows.
struct PersonDTO
{
    int64_t     id                    = 0;
    std::string first_name;
    std::string middle_name;
    std::string last_name;
    std::string federal_tax_id;
    std::string identification_number;
    std::string passport_number;
    std::string passport_country;
    std::string passport_expiry;          // ISO-8601 date string or empty
    std::string date_of_birth;            // ISO-8601 date string or empty
    std::string keywords;

    PersonDTO() = default;

    PersonDTO(std::string first, std::string last)
        : first_name(std::move(first)), last_name(std::move(last)) {}
};

/// \brief Person CRUD operations via SOCI.
class PersonDAO
{
public:
    explicit PersonDAO(DbConnection& conn) : conn_(conn) {}

    /// Insert a new person. Returns the auto-generated id.
    int64_t insert(const PersonDTO& p, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long new_id = 0;

        s.sql() <<
            "INSERT INTO t_person"
            " (first_name, middle_name, last_name, federal_tax_id,"
            "  identification_number, passport_number, passport_country,"
            "  passport_expiry, date_of_birth, keywords)"
            " VALUES"
            " (:first_name, NULLIF(:middle_name,''), :last_name,"
            "  NULLIF(:federal_tax_id,''), NULLIF(:identification_number,''),"
            "  NULLIF(:passport_number,''), NULLIF(:passport_country,''),"
            "  NULLIF(:passport_expiry,'')::date, NULLIF(:date_of_birth,'')::date,"
            "  NULLIF(:keywords,''))"
            " RETURNING id",
            soci::use(p.first_name,            "first_name"),
            soci::use(p.middle_name,           "middle_name"),
            soci::use(p.last_name,             "last_name"),
            soci::use(p.federal_tax_id,        "federal_tax_id"),
            soci::use(p.identification_number, "identification_number"),
            soci::use(p.passport_number,       "passport_number"),
            soci::use(p.passport_country,      "passport_country"),
            soci::use(p.passport_expiry,       "passport_expiry"),
            soci::use(p.date_of_birth,         "date_of_birth"),
            soci::use(p.keywords,              "keywords"),
            soci::into(new_id);

        return static_cast<int64_t>(new_id);
    }

    /// Find person by id. Returns std::nullopt if not found.
    std::optional<PersonDTO> find_by_id(int64_t id)
    {
        auto s = conn_.session();
        soci::row r;
        long long lid = static_cast<long long>(id);

        s.sql() <<
            "SELECT id, first_name, COALESCE(middle_name,'') AS middle_name,"
            " last_name, COALESCE(federal_tax_id,'') AS federal_tax_id,"
            " COALESCE(identification_number,'') AS identification_number,"
            " COALESCE(passport_number,'') AS passport_number,"
            " COALESCE(passport_country,'') AS passport_country,"
            " COALESCE(passport_expiry::text,'') AS passport_expiry,"
            " COALESCE(date_of_birth::text,'') AS date_of_birth,"
            " COALESCE(keywords,'') AS keywords"
            " FROM t_person WHERE id = :id",
            soci::use(lid, "id"),
            soci::into(r);

        if (!s.sql().got_data())
            return std::nullopt;

        PersonDTO p;
        p.id                    = r.get<long long>(0);
        p.first_name            = r.get<std::string>(1);
        p.middle_name           = r.get<std::string>(2);
        p.last_name             = r.get<std::string>(3);
        p.federal_tax_id        = r.get<std::string>(4);
        p.identification_number = r.get<std::string>(5);
        p.passport_number       = r.get<std::string>(6);
        p.passport_country      = r.get<std::string>(7);
        p.passport_expiry       = r.get<std::string>(8);
        p.date_of_birth         = r.get<std::string>(9);
        p.keywords              = r.get<std::string>(10);

        return p;
    }

    /// Update an existing person. Returns true if a row was updated.
    bool update(const PersonDTO& p, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(p.id);
        long long ret_id = 0;

        s.sql() <<
            "UPDATE t_person SET"
            " first_name = :first_name,"
            " middle_name = NULLIF(:middle_name,''),"
            " last_name = :last_name,"
            " federal_tax_id = NULLIF(:federal_tax_id,''),"
            " identification_number = NULLIF(:identification_number,''),"
            " passport_number = NULLIF(:passport_number,''),"
            " passport_country = NULLIF(:passport_country,''),"
            " passport_expiry = NULLIF(:passport_expiry,'')::date,"
            " date_of_birth = NULLIF(:date_of_birth,'')::date,"
            " keywords = NULLIF(:keywords,'')"
            " WHERE id = :id RETURNING id",
            soci::use(p.first_name,            "first_name"),
            soci::use(p.middle_name,           "middle_name"),
            soci::use(p.last_name,             "last_name"),
            soci::use(p.federal_tax_id,        "federal_tax_id"),
            soci::use(p.identification_number, "identification_number"),
            soci::use(p.passport_number,       "passport_number"),
            soci::use(p.passport_country,      "passport_country"),
            soci::use(p.passport_expiry,       "passport_expiry"),
            soci::use(p.date_of_birth,         "date_of_birth"),
            soci::use(p.keywords,              "keywords"),
            soci::use(lid,                     "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

    /// Delete a person by id. Returns true if a row was deleted.
    bool remove(int64_t id, const std::string& username = {})
    {
        auto s = conn_.session(username);
        long long lid = static_cast<long long>(id);
        long long ret_id = 0;

        s.sql() << "DELETE FROM t_person WHERE id = :id RETURNING id",
            soci::use(lid, "id"),
            soci::into(ret_id);

        return s.sql().got_data();
    }

    /// List all persons (with optional pagination).
    std::vector<PersonDTO> list_all(int limit = 100, int offset = 0)
    {
        auto s = conn_.session();
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, first_name, COALESCE(middle_name,''), last_name, "
                "COALESCE(federal_tax_id,''), COALESCE(identification_number,''), "
                "COALESCE(passport_number,''), COALESCE(passport_country,''), "
                "COALESCE(passport_expiry::text,''), COALESCE(date_of_birth::text,''), "
                "COALESCE(keywords,'') "
                "FROM t_person ORDER BY last_name, first_name "
                "LIMIT :lim OFFSET :off",
                soci::use(limit, "lim"), soci::use(offset, "off"));

        std::vector<PersonDTO> result;
        for (auto& r : rs)
        {
            PersonDTO p;
            p.id                    = r.get<long long>(0);
            p.first_name            = r.get<std::string>(1);
            p.middle_name           = r.get<std::string>(2);
            p.last_name             = r.get<std::string>(3);
            p.federal_tax_id        = r.get<std::string>(4);
            p.identification_number = r.get<std::string>(5);
            p.passport_number       = r.get<std::string>(6);
            p.passport_country      = r.get<std::string>(7);
            p.passport_expiry       = r.get<std::string>(8);
            p.date_of_birth         = r.get<std::string>(9);
            p.keywords              = r.get<std::string>(10);
            result.push_back(std::move(p));
        }
        return result;
    }

    /// Find persons by last name (case-insensitive ILIKE).
    std::vector<PersonDTO> find_by_last_name(const std::string& pattern)
    {
        auto s = conn_.session();
        std::string like_pattern = "%" + pattern + "%";
        soci::rowset<soci::row> rs =
            (s.sql().prepare <<
                "SELECT id, first_name, COALESCE(middle_name,''), last_name, "
                "COALESCE(federal_tax_id,''), COALESCE(identification_number,''), "
                "COALESCE(passport_number,''), COALESCE(passport_country,''), "
                "COALESCE(passport_expiry::text,''), COALESCE(date_of_birth::text,''), "
                "COALESCE(keywords,'') "
                "FROM t_person WHERE last_name ILIKE :pat "
                "ORDER BY last_name, first_name",
                soci::use(like_pattern, "pat"));

        std::vector<PersonDTO> result;
        for (auto& r : rs)
        {
            PersonDTO p;
            p.id                    = r.get<long long>(0);
            p.first_name            = r.get<std::string>(1);
            p.middle_name           = r.get<std::string>(2);
            p.last_name             = r.get<std::string>(3);
            p.federal_tax_id        = r.get<std::string>(4);
            p.identification_number = r.get<std::string>(5);
            p.passport_number       = r.get<std::string>(6);
            p.passport_country      = r.get<std::string>(7);
            p.passport_expiry       = r.get<std::string>(8);
            p.date_of_birth         = r.get<std::string>(9);
            p.keywords              = r.get<std::string>(10);
            result.push_back(std::move(p));
        }
        return result;
    }

private:
    DbConnection& conn_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_PERSON_DAO_HPP
