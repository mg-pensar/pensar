// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)
//
// Integration tests for the Contact Management database layer.
// Requires a running PostgreSQL with contact_db deployed via contact_ddl.sql.
//
// Build target: contact_db_tests
// Run:          ./build/contact_db_tests --colour-mode ansi

#include <catch2/catch_test_macros.hpp>

#include "../data/db_connection.hpp"
#include "../data/person_dao.hpp"
#include "../data/org_dao.hpp"
#include "../data/contact_dao.hpp"
#include "../data/role_dao.hpp"
#include "../data/search_dao.hpp"
#include "../data/visibility_dao.hpp"
#include "../data/audit_dao.hpp"
#include "../data/locale_dao.hpp"

#include <string>
#include <optional>

using namespace pensar_digital::cpplib::contact::db;

// ── Connection string ────────────────────────────────────────────────────────
// Adjust if your PostgreSQL uses a different user or database name.
static const std::string CONN_STR = "dbname=contact_db user=mg";

// ─────────────────────────────────────────────────────────────────────────────
// Locale DAO
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("LocaleDAO - list and lookup", "[db][locale]")
{
    DbConnection pool(CONN_STR, 2);
    LocaleDAO dao(pool);

    SECTION("list_all returns seeded locales")
    {
        auto locales = dao.list_all();
        REQUIRE(locales.size() >= 5);

        bool found_ptBR = false;
        for (auto& l : locales)
        {
            if (l.code == "pt-BR")
            {
                found_ptBR = true;
                CHECK(l.is_default == true);
            }
        }
        CHECK(found_ptBR);
    }

    SECTION("find_by_code")
    {
        auto en = dao.find_by_code("en-US");
        REQUIRE(en.has_value());
        CHECK(en->code == "en-US");
        CHECK(en->is_default == false);

        auto bad = dao.find_by_code("xx-XX");
        CHECK_FALSE(bad.has_value());
    }

    SECTION("get_default returns pt-BR")
    {
        auto def = dao.get_default();
        REQUIRE(def.has_value());
        CHECK(def->code == "pt-BR");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Person DAO — CRUD
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("PersonDAO - CRUD", "[db][person]")
{
    DbConnection pool(CONN_STR, 2);
    PersonDAO dao(pool);

    PersonDTO p;
    p.first_name  = "Joao";
    p.middle_name = "Carlos";
    p.last_name   = "Silva";
    p.keywords    = "developer engineer";

    SECTION("insert and find_by_id")
    {
        int64_t id = dao.insert(p, "test_user");
        REQUIRE(id > 0);

        auto found = dao.find_by_id(id);
        REQUIRE(found.has_value());
        CHECK(found->first_name == "Joao");
        CHECK(found->middle_name == "Carlos");
        CHECK(found->last_name == "Silva");
        CHECK(found->keywords == "developer engineer");

        // Cleanup
        dao.remove(id, "test_user");
    }

    SECTION("update")
    {
        int64_t id = dao.insert(p, "test_user");
        REQUIRE(id > 0);

        auto found = dao.find_by_id(id);
        REQUIRE(found.has_value());

        found->last_name = "Santos";
        found->keywords  = "manager";
        bool updated = dao.update(*found, "test_user");
        CHECK(updated);

        auto refound = dao.find_by_id(id);
        REQUIRE(refound.has_value());
        CHECK(refound->last_name == "Santos");
        CHECK(refound->keywords == "manager");

        dao.remove(id, "test_user");
    }

    SECTION("remove")
    {
        int64_t id = dao.insert(p, "test_user");
        REQUIRE(id > 0);

        bool deleted = dao.remove(id, "test_user");
        CHECK(deleted);

        auto gone = dao.find_by_id(id);
        CHECK_FALSE(gone.has_value());
    }

    SECTION("find_by_last_name")
    {
        int64_t id = dao.insert(p, "test_user");
        REQUIRE(id > 0);

        auto results = dao.find_by_last_name("Silv");
        CHECK(results.size() >= 1);

        bool found_ours = false;
        for (auto& r : results)
        {
            if (r.id == id) found_ours = true;
        }
        CHECK(found_ours);

        dao.remove(id, "test_user");
    }

    SECTION("list_all with pagination")
    {
        int64_t id1 = dao.insert(p, "test_user");
        PersonDTO p2;
        p2.first_name = "Maria";
        p2.last_name  = "Souza";
        int64_t id2 = dao.insert(p2, "test_user");

        auto all = dao.list_all(100, 0);
        CHECK(all.size() >= 2);

        dao.remove(id1, "test_user");
        dao.remove(id2, "test_user");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OrgDAO — CRUD + hierarchy
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("OrgDAO - CRUD and hierarchy", "[db][org]")
{
    DbConnection pool(CONN_STR, 2);
    OrgDAO dao(pool);

    SECTION("insert root org and child, query hierarchy")
    {
        OrgUnitDTO root;
        root.org_type = "Company";
        root.keywords = "headquarters";
        int64_t root_id = dao.insert(root, "test_user");
        REQUIRE(root_id > 0);

        // Add localized name
        LocalizedOrgUnitDTO loc;
        loc.org_unit_id = root_id;
        loc.locale_id   = 1; // pt-BR
        loc.name        = "Pensar Digital";
        int64_t loc_id = dao.insert_localized(loc, "test_user");
        REQUIRE(loc_id > 0);

        // Add child department
        OrgUnitDTO dept;
        dept.parent_id = root_id;
        dept.org_type  = "Department";
        dept.keywords  = "engineering";
        int64_t dept_id = dao.insert(dept, "test_user");
        REQUIRE(dept_id > 0);

        LocalizedOrgUnitDTO dept_loc;
        dept_loc.org_unit_id = dept_id;
        dept_loc.locale_id   = 1;
        dept_loc.name        = "Engenharia";
        dao.insert_localized(dept_loc, "test_user");

        // Find by id
        auto found = dao.find_by_id(root_id);
        REQUIRE(found.has_value());
        CHECK(found->org_type == "Company");

        // Children
        auto children = dao.find_children(root_id);
        CHECK(children.size() >= 1);

        // Hierarchy
        auto hier = dao.hierarchy("pt-BR");
        CHECK(hier.size() >= 2);

        // Cleanup (child first due to FK)
        dao.remove(dept_id, "test_user");
        dao.remove(root_id, "test_user");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ContactDAO — Phone, Email, Address
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("ContactDAO - Phone, Email, Address for Person", "[db][contact]")
{
    DbConnection pool(CONN_STR, 2);
    PersonDAO person_dao(pool);
    ContactDAO contact_dao(pool);

    PersonDTO person;
    person.first_name = "Test";
    person.last_name  = "Contact";
    int64_t pid = person_dao.insert(person, "test_user");
    REQUIRE(pid > 0);

    SECTION("Phone CRUD")
    {
        PhoneDTO ph;
        ph.person_id    = pid;
        ph.country_code = "+55";
        ph.area_code    = "11";
        ph.number       = "999887766";
        ph.is_mobile    = true;
        ph.qualifier_id = 2; // Business
        ph.is_preferred = true;

        int64_t ph_id = contact_dao.insert_phone(ph, "test_user");
        REQUIRE(ph_id > 0);

        auto phones = contact_dao.phones_for_person(pid);
        REQUIRE(phones.size() >= 1);
        CHECK(phones[0].country_code == "+55");
        CHECK(phones[0].is_mobile == true);

        contact_dao.remove_phone(ph_id, "test_user");
    }

    SECTION("Email CRUD")
    {
        EmailDTO em;
        em.person_id    = pid;
        em.email        = "test@example.com";
        em.qualifier_id = 2;
        em.is_preferred = true;

        int64_t em_id = contact_dao.insert_email(em, "test_user");
        REQUIRE(em_id > 0);

        auto emails = contact_dao.emails_for_person(pid);
        REQUIRE(emails.size() >= 1);
        CHECK(emails[0].email == "test@example.com");

        contact_dao.remove_email(em_id, "test_user");
    }

    SECTION("Address CRUD")
    {
        AddressDTO addr;
        addr.person_id    = pid;
        addr.street_line1 = "Rua Teste 123";
        addr.city         = "Sao Paulo";
        addr.state_province = "SP";
        addr.postal_code  = "01234-567";
        addr.country      = "BR";
        addr.qualifier_id = 1; // Home
        addr.is_preferred = true;
        addr.latitude     = -23.550520;
        addr.longitude    = -46.633308;

        int64_t addr_id = contact_dao.insert_address(addr, "test_user");
        REQUIRE(addr_id > 0);

        auto addrs = contact_dao.addresses_for_person(pid);
        REQUIRE(addrs.size() >= 1);
        CHECK(addrs[0].city == "Sao Paulo");
        CHECK(addrs[0].country == "BR");

        contact_dao.remove_address(addr_id, "test_user");
    }

    // Cleanup
    person_dao.remove(pid, "test_user");
}

// ─────────────────────────────────────────────────────────────────────────────
// RoleDAO — Role, PersonOrgRelation, PersonRole
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("RoleDAO - Role assignment flow", "[db][role]")
{
    DbConnection pool(CONN_STR, 2);
    PersonDAO person_dao(pool);
    OrgDAO    org_dao(pool);
    RoleDAO   role_dao(pool);

    // Setup: person + org
    PersonDTO person;
    person.first_name = "Test";
    person.last_name  = "Role";
    int64_t pid = person_dao.insert(person, "test_user");
    REQUIRE(pid > 0);

    OrgUnitDTO org;
    org.org_type = "Company";
    int64_t oid = org_dao.insert(org, "test_user");
    REQUIRE(oid > 0);

    SECTION("Create role with localized name and assign to person")
    {
        RoleDTO role;
        role.org_unit_id = oid;
        int64_t rid = role_dao.insert_role(role, "test_user");
        REQUIRE(rid > 0);

        LocalizedRoleDTO lrole;
        lrole.role_id     = rid;
        lrole.locale_id   = 1; // pt-BR
        lrole.name        = "Desenvolvedor";
        lrole.description = "Desenvolvedor de software";
        int64_t lrid = role_dao.insert_localized_role(lrole, "test_user");
        REQUIRE(lrid > 0);

        // Assign person to org
        PersonOrgRelationDTO rel;
        rel.person_id    = pid;
        rel.org_unit_id  = oid;
        rel.relationship = "Employee";
        rel.start_date   = "2025-01-15";
        int64_t rel_id = role_dao.insert_person_org_relation(rel, "test_user");
        REQUIRE(rel_id > 0);

        // Assign role to person
        PersonRoleDTO pr;
        pr.person_id = pid;
        pr.role_id   = rid;
        pr.is_primary = true;
        int64_t pr_id = role_dao.assign_role(pr, "test_user");
        REQUIRE(pr_id > 0);

        // Query
        auto roles = role_dao.roles_for_person(pid);
        REQUIRE(roles.size() >= 1);
        CHECK(roles[0].is_primary == true);

        auto rels = role_dao.relations_for_person(pid);
        REQUIRE(rels.size() >= 1);
        CHECK(rels[0].relationship == "Employee");

        // Cleanup
        role_dao.unassign_role(pr_id, "test_user");
        role_dao.remove_role(rid, "test_user");
    }

    // Cleanup
    org_dao.remove(oid, "test_user");
    person_dao.remove(pid, "test_user");
}

// ─────────────────────────────────────────────────────────────────────────────
// SearchDAO — Full-text search
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("SearchDAO - Full-text search", "[db][search]")
{
    DbConnection pool(CONN_STR, 2);
    PersonDAO  person_dao(pool);
    SearchDAO  search_dao(pool);

    PersonDTO p1;
    p1.first_name = "Ricardo";
    p1.last_name  = "Albuquerque";
    p1.keywords   = "architect senior";
    int64_t id1 = person_dao.insert(p1, "test_user");
    REQUIRE(id1 > 0);

    PersonDTO p2;
    p2.first_name = "Ana";
    p2.last_name  = "Ferreira";
    p2.keywords   = "designer junior";
    int64_t id2 = person_dao.insert(p2, "test_user");
    REQUIRE(id2 > 0);

    SECTION("search_persons by keyword")
    {
        auto results = search_dao.search_persons("architect");
        REQUIRE(results.size() >= 1);
        CHECK(results[0].first_name == "Ricardo");
    }

    SECTION("search_persons by name")
    {
        auto results = search_dao.search_persons("Ferreira");
        REQUIRE(results.size() >= 1);
        CHECK(results[0].last_name == "Ferreira");
    }

    SECTION("search_contacts returns persons")
    {
        auto results = search_dao.search_contacts("senior");
        REQUIRE(results.size() >= 1);
        CHECK(results[0].entity_type == "Person");
    }

    // Cleanup
    person_dao.remove(id1, "test_user");
    person_dao.remove(id2, "test_user");
}

// ─────────────────────────────────────────────────────────────────────────────
// VisibilityDAO
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("VisibilityDAO - access control", "[db][visibility]")
{
    DbConnection pool(CONN_STR, 2);
    PersonDAO      person_dao(pool);
    VisibilityDAO  vis_dao(pool);

    PersonDTO owner;
    owner.first_name = "Owner";
    owner.last_name  = "Person";
    int64_t owner_id = person_dao.insert(owner, "test_user");
    REQUIRE(owner_id > 0);

    PersonDTO viewer;
    viewer.first_name = "Viewer";
    viewer.last_name  = "Person";
    int64_t viewer_id = person_dao.insert(viewer, "test_user");
    REQUIRE(viewer_id > 0);

    SECTION("grant and check visibility")
    {
        DataVisibilityDTO v;
        v.owner_person_id  = owner_id;
        v.data_category    = "Phone";
        v.viewer_person_id = viewer_id;
        v.can_view         = true;
        v.can_edit         = false;

        int64_t vid = vis_dao.insert(v, "test_user");
        REQUIRE(vid > 0);

        CHECK(vis_dao.can_view(owner_id, viewer_id, "Phone") == true);
        CHECK(vis_dao.can_view(owner_id, viewer_id, "Email") == false);

        auto rules = vis_dao.rules_for_owner(owner_id);
        CHECK(rules.size() >= 1);

        vis_dao.remove(vid, "test_user");
    }

    person_dao.remove(owner_id, "test_user");
    person_dao.remove(viewer_id, "test_user");
}

// ─────────────────────────────────────────────────────────────────────────────
// AuditDAO
// ─────────────────────────────────────────────────────────────────────────────
TEST_CASE("AuditDAO - audit trail", "[db][audit]")
{
    DbConnection pool(CONN_STR, 2);
    PersonDAO  person_dao(pool);
    AuditDAO   audit_dao(pool);

    PersonDTO p;
    p.first_name = "Audit";
    p.last_name  = "Test";
    int64_t pid = person_dao.insert(p, "audit_tester");
    REQUIRE(pid > 0);

    SECTION("audit log captures insert")
    {
        auto logs = audit_dao.search("t_log_person", "C");
        REQUIRE(logs.size() >= 1);

        // The most recent 'C' entry should contain our person
        bool found = false;
        for (auto& l : logs)
        {
            if (l.new_data.find("\"Audit\"") != std::string::npos ||
                l.new_data.find("Audit") != std::string::npos)
            {
                found = true;
                CHECK(l.action == "C");
                break;
            }
        }
        CHECK(found);
    }

    SECTION("entity_timestamps returns created info")
    {
        auto ts = audit_dao.entity_timestamps("t_log_person", pid);
        REQUIRE(ts.has_value());
        CHECK_FALSE(ts->created_at.empty());
    }

    person_dao.remove(pid, "audit_tester");
}
