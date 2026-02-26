-- =============================================================================
-- Contact Management System — Complete DDL
-- PostgreSQL 18+
-- Author : Pensar Digital Desenvolvimento de Software
-- License: MIT
-- =============================================================================
-- Usage:
--   createdb -U mg contact_db
--   psql -U mg -d contact_db -f contact_ddl.sql
-- =============================================================================

BEGIN;

-- ─────────────────────────────────────────────────────────────────────────────
-- 0. ROLES (idempotent)
-- ─────────────────────────────────────────────────────────────────────────────
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'contact_admin') THEN
        CREATE ROLE contact_admin NOLOGIN;
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_roles WHERE rolname = 'contact_app') THEN
        CREATE ROLE contact_app NOLOGIN;
    END IF;
END $$;

-- ─────────────────────────────────────────────────────────────────────────────
-- 1. DOMAINS — Single Source of Truth for Shared Column Types
-- ─────────────────────────────────────────────────────────────────────────────

-- Identity & References
CREATE DOMAIN D_EntityId          AS BIGINT;
CREATE DOMAIN D_QualifierId       AS SMALLINT;
CREATE DOMAIN D_FederalTaxId      AS VARCHAR(20);
CREATE DOMAIN D_CountryCode       AS CHAR(2);

-- Text / Name Fields
CREATE DOMAIN D_ShortName         AS VARCHAR(50);
CREATE DOMAIN D_MediumName        AS VARCHAR(100);
CREATE DOMAIN D_LongName          AS VARCHAR(200);
CREATE DOMAIN D_Description       AS TEXT;
CREATE DOMAIN D_EmailAddress      AS VARCHAR(254);
CREATE DOMAIN D_Url               AS VARCHAR(500);
CREATE DOMAIN D_Keywords          AS TEXT;

-- Phone Segments
CREATE DOMAIN D_PhoneCountryCode  AS VARCHAR(5);
CREATE DOMAIN D_PhoneAreaCode     AS VARCHAR(5);
CREATE DOMAIN D_PhoneNumber       AS VARCHAR(20);
CREATE DOMAIN D_PhoneExtension    AS VARCHAR(10);

-- Address Segments
CREATE DOMAIN D_PostalCode        AS VARCHAR(20);
CREATE DOMAIN D_GeoCoordinate     AS DECIMAL(9,6);

-- Misc Flags
CREATE DOMAIN D_Flag              AS BOOLEAN NOT NULL DEFAULT FALSE;
CREATE DOMAIN D_ActiveFlag        AS BOOLEAN NOT NULL DEFAULT TRUE;
CREATE DOMAIN D_SessionToken      AS VARCHAR(64);
CREATE DOMAIN D_IdentNumber       AS VARCHAR(30);
CREATE DOMAIN D_DataCategory      AS VARCHAR(50);
CREATE DOMAIN D_OrgType           AS VARCHAR(20);

-- Audit Log
CREATE DOMAIN D_AuditAction       AS CHAR(1) NOT NULL CHECK (VALUE IN ('C','U','D'));
CREATE DOMAIN D_AuditUser         AS VARCHAR(100) NOT NULL;


-- ─────────────────────────────────────────────────────────────────────────────
-- 2. LOCALE REFERENCE TABLE
-- ─────────────────────────────────────────────────────────────────────────────

CREATE TABLE T_Locale (
    id         SMALLSERIAL PRIMARY KEY,
    code       VARCHAR(10) NOT NULL UNIQUE,        -- BCP-47: 'pt-BR', 'en-US' …
    is_default D_Flag      DEFAULT FALSE
);

INSERT INTO T_Locale (code, is_default) VALUES
    ('pt-BR', TRUE),
    ('en-US', FALSE),
    ('es-AR', FALSE),
    ('es-MX', FALSE),
    ('fr-FR', FALSE);


-- ─────────────────────────────────────────────────────────────────────────────
-- 3. GENERIC AUDIT-LOG INFRASTRUCTURE
-- ─────────────────────────────────────────────────────────────────────────────

-- 3.1 Trigger function (reused by every audited table)
CREATE OR REPLACE FUNCTION fn_audit_log()
RETURNS TRIGGER AS $$
DECLARE
    v_action  CHAR(1);
    v_old     JSONB := NULL;
    v_new     JSONB := NULL;
    v_user    TEXT  := current_setting('app.current_user', TRUE);
BEGIN
    IF v_user IS NULL OR v_user = '' THEN
        v_user := session_user::text;
    END IF;

    IF TG_OP = 'INSERT' THEN
        v_action := 'C';
        v_new    := to_jsonb(NEW);
    ELSIF TG_OP = 'UPDATE' THEN
        v_action := 'U';
        v_old    := to_jsonb(OLD);
        v_new    := to_jsonb(NEW);
    ELSIF TG_OP = 'DELETE' THEN
        v_action := 'D';
        v_old    := to_jsonb(OLD);
    END IF;

    EXECUTE format(
        'INSERT INTO %I (action, action_by, action_at, old_data, new_data)
         VALUES ($1, $2, NOW(), $3, $4)',
        TG_ARGV[0]
    ) USING v_action, v_user, v_old, v_new;

    RETURN COALESCE(NEW, OLD);
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- 3.2 Helper: create log table + trigger for a given table
--     Accepts display-case names like 'T_Person'; internally lowercases
--     to match PostgreSQL's default unquoted-identifier folding.
CREATE OR REPLACE FUNCTION create_audit_log(p_table TEXT)
RETURNS VOID AS $$
DECLARE
    v_tbl  TEXT := lower(p_table);                            -- actual PG name
    v_base TEXT := substring(v_tbl FROM 3);                   -- strip 't_'
    v_log  TEXT := 't_log_' || v_base;
BEGIN
    EXECUTE format(
        'CREATE TABLE IF NOT EXISTS %I (
            log_id    BIGSERIAL PRIMARY KEY,
            action    D_AuditAction,
            action_by D_AuditUser,
            action_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
            old_data  JSONB,
            new_data  JSONB
        )', v_log);

    EXECUTE format(
        'CREATE INDEX IF NOT EXISTS idx_%s_at        ON %I (action_at)',  v_base, v_log);
    EXECUTE format(
        'CREATE INDEX IF NOT EXISTS idx_%s_action     ON %I (action)',    v_base, v_log);
    EXECUTE format(
        'CREATE INDEX IF NOT EXISTS idx_%s_action_by  ON %I (action_by)', v_base, v_log);

    EXECUTE format(
        'CREATE TRIGGER trg_audit_%s
         AFTER INSERT OR UPDATE OR DELETE ON %I
         FOR EACH ROW EXECUTE FUNCTION fn_audit_log(%L)',
        v_base, v_tbl, v_log);
END;
$$ LANGUAGE plpgsql;

-- 3.3 Admin: search a single audit log table
CREATE OR REPLACE FUNCTION fn_search_audit_log(
    p_log_table  TEXT,
    p_action     CHAR(1)       DEFAULT NULL,
    p_action_by  TEXT           DEFAULT NULL,
    p_from       TIMESTAMPTZ   DEFAULT NULL,
    p_to         TIMESTAMPTZ   DEFAULT NULL
) RETURNS TABLE(
    log_id    BIGINT,
    action    CHAR(1),
    action_by TEXT,
    action_at TIMESTAMPTZ,
    old_data  JSONB,
    new_data  JSONB
) AS $$
BEGIN
    RETURN QUERY EXECUTE format(
        'SELECT log_id, action, action_by, action_at, old_data, new_data
         FROM %I
         WHERE ($1 IS NULL OR action    = $1)
           AND ($2 IS NULL OR action_by = $2)
           AND ($3 IS NULL OR action_at >= $3)
           AND ($4 IS NULL OR action_at <  $4)
         ORDER BY action_at DESC',
        p_log_table
    ) USING p_action, p_action_by, p_from, p_to;
END;
$$ LANGUAGE plpgsql STABLE SECURITY DEFINER;

REVOKE ALL ON FUNCTION fn_search_audit_log(TEXT, CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ) FROM PUBLIC;
GRANT EXECUTE ON FUNCTION fn_search_audit_log(TEXT, CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ)
      TO contact_admin;

-- 3.4 Admin: purge old audit log entries
CREATE OR REPLACE FUNCTION fn_purge_audit_log(
    p_log_table TEXT,
    p_before    TIMESTAMPTZ
) RETURNS BIGINT AS $$
DECLARE
    v_count BIGINT;
BEGIN
    EXECUTE format('DELETE FROM %I WHERE action_at < $1', p_log_table)
        USING p_before;
    GET DIAGNOSTICS v_count = ROW_COUNT;
    RETURN v_count;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

REVOKE ALL ON FUNCTION fn_purge_audit_log(TEXT, TIMESTAMPTZ) FROM PUBLIC;
GRANT EXECUTE ON FUNCTION fn_purge_audit_log(TEXT, TIMESTAMPTZ) TO contact_admin;

-- 3.5 Admin: search across ALL log tables
CREATE OR REPLACE FUNCTION fn_search_all_audit_logs(
    p_action     CHAR(1)       DEFAULT NULL,
    p_action_by  TEXT           DEFAULT NULL,
    p_from       TIMESTAMPTZ   DEFAULT NULL,
    p_to         TIMESTAMPTZ   DEFAULT NULL,
    p_limit      INT           DEFAULT 500
) RETURNS TABLE(
    source_table TEXT,
    log_id       BIGINT,
    action       CHAR(1),
    action_by    TEXT,
    action_at    TIMESTAMPTZ,
    old_data     JSONB,
    new_data     JSONB
) AS $$
DECLARE
    v_tbl  TEXT;
    v_sql  TEXT := '';
    v_sep  TEXT := '';
BEGIN
    FOR v_tbl IN
        SELECT table_name FROM information_schema.tables
        WHERE table_schema = 'public' AND table_name LIKE 'T_LOG_%'
        ORDER BY table_name
    LOOP
        v_sql := v_sql || v_sep || format(
            'SELECT %L::text AS source_table,
                    log_id, action, action_by, action_at, old_data, new_data
             FROM %I
             WHERE ($1 IS NULL OR action    = $1)
               AND ($2 IS NULL OR action_by = $2)
               AND ($3 IS NULL OR action_at >= $3)
               AND ($4 IS NULL OR action_at <  $4)',
            v_tbl, v_tbl);
        v_sep := ' UNION ALL ';
    END LOOP;

    IF v_sql = '' THEN RETURN; END IF;

    v_sql := 'SELECT * FROM (' || v_sql || ') sub ORDER BY action_at DESC LIMIT $5';

    RETURN QUERY EXECUTE v_sql USING p_action, p_action_by, p_from, p_to, p_limit;
END;
$$ LANGUAGE plpgsql STABLE SECURITY DEFINER;

REVOKE ALL ON FUNCTION fn_search_all_audit_logs(CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ, INT) FROM PUBLIC;
GRANT EXECUTE ON FUNCTION fn_search_all_audit_logs(CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ, INT)
      TO contact_admin;


-- ─────────────────────────────────────────────────────────────────────────────
-- 4. CORE ENTITY TABLES
-- ─────────────────────────────────────────────────────────────────────────────

-- 4.1 Person
CREATE TABLE T_Person (
    id                    BIGSERIAL PRIMARY KEY,
    first_name            D_ShortName  NOT NULL,
    middle_name           D_ShortName,
    last_name             D_MediumName NOT NULL,
    federal_tax_id        D_FederalTaxId,
    identification_number D_IdentNumber,
    passport_number       D_IdentNumber,
    passport_country      D_CountryCode,
    passport_expiry       DATE,
    photo                 BYTEA,
    date_of_birth         DATE,
    keywords              D_Keywords,
    search_vector         TSVECTOR
);

CREATE INDEX idx_person_name     ON T_Person (last_name, first_name);
CREATE UNIQUE INDEX idx_person_tax_id ON T_Person (federal_tax_id)
       WHERE federal_tax_id IS NOT NULL;
CREATE INDEX idx_person_search   ON T_Person USING GIN (search_vector);

SELECT create_audit_log('T_Person');

-- 4.2 Organization Unit (composite pattern — self-referencing)
CREATE TABLE T_OrgUnit (
    id             BIGSERIAL PRIMARY KEY,
    parent_id      D_EntityId REFERENCES T_OrgUnit(id) ON DELETE SET NULL,
    org_type       D_OrgType  NOT NULL
                   CHECK (org_type IN ('Group','Company','Department','Division','Team')),
    federal_tax_id D_FederalTaxId,
    is_active      D_ActiveFlag,
    keywords       D_Keywords,
    search_vector  TSVECTOR
);

CREATE INDEX idx_orgunit_parent  ON T_OrgUnit (parent_id);
CREATE INDEX idx_orgunit_type    ON T_OrgUnit (org_type);
CREATE INDEX idx_orgunit_search  ON T_OrgUnit USING GIN (search_vector);

SELECT create_audit_log('T_OrgUnit');

-- 4.3 Localized OrgUnit names
CREATE TABLE T_LocalizedOrgUnit (
    id          BIGSERIAL PRIMARY KEY,
    org_unit_id D_EntityId NOT NULL REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    locale_id   SMALLINT   NOT NULL REFERENCES T_Locale(id),
    name        D_LongName NOT NULL,
    UNIQUE (org_unit_id, locale_id)
);

CREATE INDEX idx_loc_orgunit ON T_LocalizedOrgUnit (org_unit_id);


-- ─────────────────────────────────────────────────────────────────────────────
-- 5. FULL-TEXT SEARCH TRIGGERS
-- ─────────────────────────────────────────────────────────────────────────────

-- 5.1 Person search vector
CREATE OR REPLACE FUNCTION fn_person_search_vector()
RETURNS TRIGGER AS $$
BEGIN
    NEW.search_vector :=
        setweight(to_tsvector('simple', COALESCE(NEW.first_name, '')),             'A') ||
        setweight(to_tsvector('simple', COALESCE(NEW.middle_name, '')),            'B') ||
        setweight(to_tsvector('simple', COALESCE(NEW.last_name, '')),              'A') ||
        setweight(to_tsvector('simple', COALESCE(NEW.federal_tax_id, '')),         'C') ||
        setweight(to_tsvector('simple', COALESCE(NEW.identification_number, '')),  'C') ||
        setweight(to_tsvector('simple', COALESCE(NEW.keywords, '')),               'B');
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_person_search_vector
    BEFORE INSERT OR UPDATE OF first_name, middle_name, last_name,
                                federal_tax_id, identification_number, keywords
    ON T_Person
    FOR EACH ROW EXECUTE FUNCTION fn_person_search_vector();

-- 5.2 OrgUnit search vector
CREATE OR REPLACE FUNCTION fn_orgunit_search_vector()
RETURNS TRIGGER AS $$
DECLARE
    v_name TEXT;
BEGIN
    SELECT loc.name INTO v_name
      FROM T_LocalizedOrgUnit loc
      JOIN T_Locale l ON l.id = loc.locale_id AND l.is_default = TRUE
     WHERE loc.org_unit_id = NEW.id;

    NEW.search_vector :=
        setweight(to_tsvector('simple', COALESCE(v_name, '')),             'A') ||
        setweight(to_tsvector('simple', COALESCE(NEW.federal_tax_id, '')), 'C') ||
        setweight(to_tsvector('simple', COALESCE(NEW.keywords, '')),       'B');
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_orgunit_search_vector
    BEFORE INSERT OR UPDATE OF federal_tax_id, keywords
    ON T_OrgUnit
    FOR EACH ROW EXECUTE FUNCTION fn_orgunit_search_vector();

-- 5.3 When a localized name changes, refresh parent OrgUnit vector
CREATE OR REPLACE FUNCTION fn_localized_orgunit_refresh_search()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE T_OrgUnit
       SET keywords = keywords
     WHERE id = COALESCE(NEW.org_unit_id, OLD.org_unit_id);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_localized_orgunit_refresh
    AFTER INSERT OR UPDATE OR DELETE ON T_LocalizedOrgUnit
    FOR EACH ROW EXECUTE FUNCTION fn_localized_orgunit_refresh_search();


-- ─────────────────────────────────────────────────────────────────────────────
-- 6. CONTACT INFORMATION TABLES
-- ─────────────────────────────────────────────────────────────────────────────

-- 6.1 Contact Qualifier (id only; label is localized)
CREATE TABLE T_ContactQualifier (
    id SMALLINT PRIMARY KEY
);

INSERT INTO T_ContactQualifier VALUES (1), (2), (3);

-- 6.2 Localized qualifier labels
CREATE TABLE T_LocalizedContactQualifier (
    id           BIGSERIAL PRIMARY KEY,
    qualifier_id D_QualifierId NOT NULL REFERENCES T_ContactQualifier(id) ON DELETE CASCADE,
    locale_id    SMALLINT      NOT NULL REFERENCES T_Locale(id),
    name         D_ShortName   NOT NULL,
    UNIQUE (qualifier_id, locale_id)
);

-- Seeds: 1=Home/Casa, 2=Business/Comercial, 3=Other/Outro
INSERT INTO T_LocalizedContactQualifier (qualifier_id, locale_id, name) VALUES
    (1, 1, 'Casa'),       (1, 2, 'Home'),     (1, 3, 'Hogar'),    (1, 4, 'Hogar'),    (1, 5, 'Maison'),
    (2, 1, 'Comercial'),  (2, 2, 'Business'), (2, 3, 'Comercial'),(2, 4, 'Comercial'),(2, 5, 'Professionnel'),
    (3, 1, 'Outro'),      (3, 2, 'Other'),    (3, 3, 'Otro'),     (3, 4, 'Otro'),     (3, 5, 'Autre');

-- 6.3 Phone Numbers
CREATE TABLE T_Phone (
    id           BIGSERIAL PRIMARY KEY,
    person_id    D_EntityId REFERENCES T_Person(id)  ON DELETE CASCADE,
    org_unit_id  D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    country_code D_PhoneCountryCode NOT NULL,
    area_code    D_PhoneAreaCode,
    number       D_PhoneNumber NOT NULL,
    extension    D_PhoneExtension,
    is_mobile    D_Flag DEFAULT FALSE,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    CONSTRAINT chk_phone_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL     AND org_unit_id IS NOT NULL)
    )
);

CREATE INDEX idx_phone_person ON T_Phone (person_id);
CREATE INDEX idx_phone_org    ON T_Phone (org_unit_id);

SELECT create_audit_log('T_Phone');

-- 6.4 Email Addresses
CREATE TABLE T_Email (
    id           BIGSERIAL PRIMARY KEY,
    person_id    D_EntityId REFERENCES T_Person(id)  ON DELETE CASCADE,
    org_unit_id  D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    email        D_EmailAddress NOT NULL,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    is_verified  D_Flag DEFAULT FALSE,
    CONSTRAINT chk_email_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL     AND org_unit_id IS NOT NULL)
    )
);

CREATE UNIQUE INDEX idx_email_unique ON T_Email (email);
CREATE INDEX idx_email_person        ON T_Email (person_id);

SELECT create_audit_log('T_Email');

-- 6.5 Physical Addresses
CREATE TABLE T_Address (
    id             BIGSERIAL PRIMARY KEY,
    person_id      D_EntityId REFERENCES T_Person(id)  ON DELETE CASCADE,
    org_unit_id    D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    street_line1   D_LongName   NOT NULL,
    street_line2   D_LongName,
    city           D_MediumName NOT NULL,
    state_province D_MediumName,
    postal_code    D_PostalCode,
    country        D_CountryCode NOT NULL,
    qualifier_id   D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred   D_Flag DEFAULT FALSE,
    latitude       D_GeoCoordinate,
    longitude      D_GeoCoordinate,
    CONSTRAINT chk_address_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL     AND org_unit_id IS NOT NULL)
    )
);

CREATE INDEX idx_address_person ON T_Address (person_id);
CREATE INDEX idx_address_org    ON T_Address (org_unit_id);

SELECT create_audit_log('T_Address');

-- 6.6 Social Media
CREATE TABLE T_SocialMedia (
    id           BIGSERIAL PRIMARY KEY,
    person_id    D_EntityId REFERENCES T_Person(id)  ON DELETE CASCADE,
    org_unit_id  D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    platform     D_ShortName  NOT NULL,
    handle       D_MediumName NOT NULL,
    url          D_Url,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    CONSTRAINT chk_social_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL     AND org_unit_id IS NOT NULL)
    )
);

SELECT create_audit_log('T_SocialMedia');

-- 6.7 Websites
CREATE TABLE T_Website (
    id           BIGSERIAL PRIMARY KEY,
    person_id    D_EntityId REFERENCES T_Person(id)  ON DELETE CASCADE,
    org_unit_id  D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    url          D_Url        NOT NULL,
    title        D_MediumName,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    CONSTRAINT chk_website_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL     AND org_unit_id IS NOT NULL)
    )
);

SELECT create_audit_log('T_Website');


-- ─────────────────────────────────────────────────────────────────────────────
-- 7. ROLES AND RELATIONSHIPS
-- ─────────────────────────────────────────────────────────────────────────────

-- 7.1 Role definition
CREATE TABLE T_Role (
    id          BIGSERIAL PRIMARY KEY,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    is_active   D_ActiveFlag
);

CREATE INDEX idx_role_org ON T_Role (org_unit_id);

SELECT create_audit_log('T_Role');

-- 7.2 Role localized fields
CREATE TABLE T_LocalizedRole (
    id          BIGSERIAL PRIMARY KEY,
    role_id     D_EntityId NOT NULL REFERENCES T_Role(id) ON DELETE CASCADE,
    locale_id   SMALLINT   NOT NULL REFERENCES T_Locale(id),
    name        D_MediumName NOT NULL,
    description D_Description,
    UNIQUE (role_id, locale_id)
);

CREATE INDEX idx_loc_role ON T_LocalizedRole (role_id);

-- 7.3 Relationship type enum
CREATE TYPE relationship_type AS ENUM (
    'Employee', 'Trainee', 'MinorApprentice', 'ThirdParty'
);

-- 7.4 Person–Organization relationship
CREATE TABLE T_PersonOrgRelation (
    id           BIGSERIAL PRIMARY KEY,
    person_id    D_EntityId        NOT NULL REFERENCES T_Person(id)  ON DELETE CASCADE,
    org_unit_id  D_EntityId        NOT NULL REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    relationship relationship_type NOT NULL,
    start_date   DATE              NOT NULL,
    end_date     DATE,
    is_active    D_ActiveFlag,
    UNIQUE (person_id, org_unit_id, relationship)
);

CREATE INDEX idx_personorg_person ON T_PersonOrgRelation (person_id);
CREATE INDEX idx_personorg_org    ON T_PersonOrgRelation (org_unit_id);

SELECT create_audit_log('T_PersonOrgRelation');

-- 7.5 Person–Role assignment (many-to-many)
CREATE TABLE T_PersonRole (
    id            BIGSERIAL PRIMARY KEY,
    person_id     D_EntityId NOT NULL REFERENCES T_Person(id) ON DELETE CASCADE,
    role_id       D_EntityId NOT NULL REFERENCES T_Role(id)   ON DELETE CASCADE,
    assigned_date DATE DEFAULT CURRENT_DATE,
    end_date      DATE,
    is_primary    D_Flag DEFAULT FALSE,
    UNIQUE (person_id, role_id)
);

CREATE INDEX idx_personrole_person ON T_PersonRole (person_id);
CREATE INDEX idx_personrole_role   ON T_PersonRole (role_id);

SELECT create_audit_log('T_PersonRole');


-- ─────────────────────────────────────────────────────────────────────────────
-- 8. ACCESS CONTROL
-- ─────────────────────────────────────────────────────────────────────────────

CREATE TABLE T_DataVisibility (
    id                 BIGSERIAL PRIMARY KEY,
    owner_person_id    D_EntityId NOT NULL REFERENCES T_Person(id) ON DELETE CASCADE,
    data_category      D_DataCategory NOT NULL,   -- 'Phone','Email','Address','Personal','All'
    viewer_person_id   D_EntityId REFERENCES T_Person(id)  ON DELETE CASCADE,
    viewer_role_id     D_EntityId REFERENCES T_Role(id)    ON DELETE CASCADE,
    viewer_org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    can_view           D_Flag DEFAULT TRUE,
    can_edit           D_Flag DEFAULT FALSE,
    CONSTRAINT chk_single_viewer CHECK (
        (viewer_person_id   IS NOT NULL)::int +
        (viewer_role_id     IS NOT NULL)::int +
        (viewer_org_unit_id IS NOT NULL)::int = 1
    )
);

CREATE INDEX idx_visibility_owner      ON T_DataVisibility (owner_person_id);
CREATE INDEX idx_visibility_v_person   ON T_DataVisibility (viewer_person_id);
CREATE INDEX idx_visibility_v_role     ON T_DataVisibility (viewer_role_id);
CREATE INDEX idx_visibility_v_org      ON T_DataVisibility (viewer_org_unit_id);

SELECT create_audit_log('T_DataVisibility');


-- ─────────────────────────────────────────────────────────────────────────────
-- 9. CONVENIENCE SEARCH FUNCTIONS
-- ─────────────────────────────────────────────────────────────────────────────

-- 9.1 Search persons
CREATE OR REPLACE FUNCTION fn_search_persons(
    p_query     TEXT,
    p_limit     INT DEFAULT 50,
    p_ts_config REGCONFIG DEFAULT 'simple'
) RETURNS TABLE(
    id         BIGINT,
    first_name D_ShortName,
    last_name  D_MediumName,
    keywords   D_Keywords,
    rank       REAL
) AS $$
BEGIN
    RETURN QUERY
    SELECT p.id, p.first_name, p.last_name, p.keywords,
           ts_rank_cd(p.search_vector,
                      websearch_to_tsquery(p_ts_config, p_query)) AS rank
      FROM T_Person p
     WHERE p.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
     ORDER BY rank DESC
     LIMIT p_limit;
END;
$$ LANGUAGE plpgsql STABLE;

-- 9.2 Search org units
CREATE OR REPLACE FUNCTION fn_search_orgunits(
    p_query     TEXT,
    p_limit     INT DEFAULT 50,
    p_ts_config REGCONFIG DEFAULT 'simple'
) RETURNS TABLE(
    id       BIGINT,
    name     D_LongName,
    org_type D_OrgType,
    keywords D_Keywords,
    rank     REAL
) AS $$
BEGIN
    RETURN QUERY
    SELECT o.id, loc.name, o.org_type, o.keywords,
           ts_rank_cd(o.search_vector,
                      websearch_to_tsquery(p_ts_config, p_query)) AS rank
      FROM T_OrgUnit o
      JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
      JOIN T_Locale l             ON l.id = loc.locale_id AND l.is_default = TRUE
     WHERE o.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
     ORDER BY rank DESC
     LIMIT p_limit;
END;
$$ LANGUAGE plpgsql STABLE;

-- 9.3 Unified contact search (persons + orgs)
CREATE OR REPLACE FUNCTION fn_search_contacts(
    p_query     TEXT,
    p_limit     INT DEFAULT 50,
    p_ts_config REGCONFIG DEFAULT 'simple'
) RETURNS TABLE(
    entity_type  TEXT,
    entity_id    BIGINT,
    display_name TEXT,
    keywords     D_Keywords,
    rank         REAL
) AS $$
BEGIN
    RETURN QUERY
    SELECT sub.entity_type, sub.entity_id, sub.display_name, sub.keywords, sub.rank
    FROM (
        SELECT 'Person'::text AS entity_type, p.id AS entity_id,
               TRIM(COALESCE(p.first_name,'') || ' ' || COALESCE(p.last_name,'')) AS display_name,
               p.keywords,
               ts_rank_cd(p.search_vector, websearch_to_tsquery(p_ts_config, p_query)) AS rank
          FROM T_Person p
         WHERE p.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
        UNION ALL
        SELECT 'OrgUnit'::text, o.id,
               loc.name::text,
               o.keywords,
               ts_rank_cd(o.search_vector, websearch_to_tsquery(p_ts_config, p_query))
          FROM T_OrgUnit o
          JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
          JOIN T_Locale l             ON l.id = loc.locale_id AND l.is_default = TRUE
         WHERE o.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
    ) sub
    ORDER BY sub.rank DESC
    LIMIT p_limit;
END;
$$ LANGUAGE plpgsql STABLE;


-- ─────────────────────────────────────────────────────────────────────────────
-- 10. VIEWS & PER-LOCALE MATERIALIZED VIEWS
-- ─────────────────────────────────────────────────────────────────────────────

-- 10.1 Person with preferred contact info
CREATE VIEW V_PersonContactSummary AS
SELECT
    p.id,
    p.first_name,
    p.middle_name,
    p.last_name,
    p.photo,
    p.keywords,
    ph.country_code || ph.area_code || ph.number AS preferred_phone,
    e.email AS preferred_email
FROM T_Person p
LEFT JOIN T_Phone ph ON ph.person_id = p.id AND ph.is_preferred = TRUE AND ph.is_mobile = TRUE
LEFT JOIN T_Email e  ON e.person_id  = p.id AND e.is_preferred  = TRUE;

-- 10.2 Per-locale materialized views: OrgHierarchy
DO $$
DECLARE
    v_rec  RECORD;
    v_sfx  TEXT;
    v_mv   TEXT;
BEGIN
    FOR v_rec IN SELECT id, code FROM T_Locale ORDER BY id LOOP
        v_sfx := replace(v_rec.code, '-', '_');
        v_mv  := 'MV_OrgHierarchy_' || v_sfx;

        EXECUTE format(
            'CREATE MATERIALIZED VIEW %I AS
             WITH RECURSIVE org_tree AS (
                 SELECT o.id, o.parent_id, loc.name, o.org_type, o.keywords,
                        1 AS depth, ARRAY[o.id] AS path
                   FROM T_OrgUnit o
                   JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
                   JOIN T_Locale l             ON l.id = loc.locale_id AND l.code = %L
                  WHERE o.parent_id IS NULL
                 UNION ALL
                 SELECT o.id, o.parent_id, loc.name, o.org_type, o.keywords,
                        t.depth + 1, t.path || o.id
                   FROM T_OrgUnit o
                   JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
                   JOIN T_Locale l             ON l.id = loc.locale_id AND l.code = %L
                   JOIN org_tree t              ON o.parent_id = t.id
             )
             SELECT * FROM org_tree',
            v_mv, v_rec.code, v_rec.code);

        EXECUTE format('CREATE INDEX idx_%s_id     ON %I (id)',         lower(v_mv), v_mv);
        EXECUTE format('CREATE INDEX idx_%s_parent  ON %I (parent_id)', lower(v_mv), v_mv);
    END LOOP;
END $$;

-- 10.3 Per-locale materialized views: ContactQualifier
DO $$
DECLARE
    v_rec  RECORD;
    v_sfx  TEXT;
    v_mv   TEXT;
BEGIN
    FOR v_rec IN SELECT id, code FROM T_Locale ORDER BY id LOOP
        v_sfx := replace(v_rec.code, '-', '_');
        v_mv  := 'MV_ContactQualifier_' || v_sfx;

        EXECUTE format(
            'CREATE MATERIALIZED VIEW %I AS
             SELECT cq.id, loc.name
               FROM T_ContactQualifier cq
               JOIN T_LocalizedContactQualifier loc ON loc.qualifier_id = cq.id
               JOIN T_Locale l                      ON l.id = loc.locale_id AND l.code = %L',
            v_mv, v_rec.code);
    END LOOP;
END $$;

-- 10.4 Per-locale materialized views: Role
DO $$
DECLARE
    v_rec  RECORD;
    v_sfx  TEXT;
    v_mv   TEXT;
BEGIN
    FOR v_rec IN SELECT id, code FROM T_Locale ORDER BY id LOOP
        v_sfx := replace(v_rec.code, '-', '_');
        v_mv  := 'MV_Role_' || v_sfx;

        EXECUTE format(
            'CREATE MATERIALIZED VIEW %I AS
             SELECT r.id, r.org_unit_id, r.is_active, loc.name, loc.description
               FROM T_Role r
               JOIN T_LocalizedRole loc ON loc.role_id = r.id
               JOIN T_Locale l          ON l.id = loc.locale_id AND l.code = %L',
            v_mv, v_rec.code);
    END LOOP;
END $$;

-- 10.5 Session-aware accessor functions

CREATE OR REPLACE FUNCTION fn_mv_org_hierarchy()
RETURNS TABLE(id BIGINT, parent_id BIGINT, name D_LongName, org_type D_OrgType,
              keywords D_Keywords, depth INT, path BIGINT[]) AS $$
DECLARE
    v_locale TEXT := coalesce(nullif(current_setting('app.locale', TRUE), ''), 'pt-BR');
BEGIN
    RETURN QUERY EXECUTE format(
        'SELECT id, parent_id, name, org_type, keywords, depth, path FROM %I',
        'MV_OrgHierarchy_' || replace(v_locale, '-', '_'));
END;
$$ LANGUAGE plpgsql STABLE;

CREATE OR REPLACE FUNCTION fn_mv_contact_qualifier()
RETURNS TABLE(id SMALLINT, name D_ShortName) AS $$
DECLARE
    v_locale TEXT := coalesce(nullif(current_setting('app.locale', TRUE), ''), 'pt-BR');
BEGIN
    RETURN QUERY EXECUTE format(
        'SELECT id, name FROM %I',
        'MV_ContactQualifier_' || replace(v_locale, '-', '_'));
END;
$$ LANGUAGE plpgsql STABLE;

CREATE OR REPLACE FUNCTION fn_mv_role()
RETURNS TABLE(id BIGINT, org_unit_id BIGINT, is_active BOOLEAN,
              name D_MediumName, description D_Description) AS $$
DECLARE
    v_locale TEXT := coalesce(nullif(current_setting('app.locale', TRUE), ''), 'pt-BR');
BEGIN
    RETURN QUERY EXECUTE format(
        'SELECT id, org_unit_id, is_active, name, description FROM %I',
        'MV_Role_' || replace(v_locale, '-', '_'));
END;
$$ LANGUAGE plpgsql STABLE;

-- 10.6 Live locale-parameterised OrgUnit hierarchy
CREATE OR REPLACE FUNCTION fn_org_hierarchy(p_locale_code VARCHAR DEFAULT 'pt-BR')
RETURNS TABLE(id BIGINT, parent_id BIGINT, name D_LongName, org_type D_OrgType,
              keywords D_Keywords, depth INT, path BIGINT[]) AS $$
BEGIN
    RETURN QUERY
    WITH RECURSIVE org_tree AS (
        SELECT o.id::BIGINT, o.parent_id::BIGINT, loc.name, o.org_type, o.keywords,
               1 AS depth, ARRAY[o.id::BIGINT] AS path
          FROM T_OrgUnit o
          JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
          JOIN T_Locale l             ON l.id = loc.locale_id AND l.code = p_locale_code
         WHERE o.parent_id IS NULL
        UNION ALL
        SELECT o.id::BIGINT, o.parent_id::BIGINT, loc.name, o.org_type, o.keywords,
               t.depth + 1, t.path || o.id::BIGINT
          FROM T_OrgUnit o
          JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
          JOIN T_Locale l             ON l.id = loc.locale_id AND l.code = p_locale_code
          JOIN org_tree t              ON o.parent_id = t.id
    )
    SELECT * FROM org_tree;
END;
$$ LANGUAGE plpgsql STABLE;

-- 10.7 Refresh all per-locale MVs
CREATE OR REPLACE FUNCTION fn_refresh_locale_mvs()
RETURNS VOID AS $$
DECLARE
    v_rec RECORD;
    v_sfx TEXT;
BEGIN
    FOR v_rec IN SELECT code FROM T_Locale ORDER BY id LOOP
        v_sfx := replace(v_rec.code, '-', '_');
        EXECUTE format('REFRESH MATERIALIZED VIEW %I', 'MV_OrgHierarchy_'     || v_sfx);
        EXECUTE format('REFRESH MATERIALIZED VIEW %I', 'MV_ContactQualifier_'  || v_sfx);
        EXECUTE format('REFRESH MATERIALIZED VIEW %I', 'MV_Role_'              || v_sfx);
    END LOOP;
END;
$$ LANGUAGE plpgsql;

-- 10.8 Convenience: derive created_at / updated_at from audit log
CREATE OR REPLACE FUNCTION fn_entity_timestamps(p_log_table TEXT, p_entity_id BIGINT)
RETURNS TABLE(created_at TIMESTAMPTZ, created_by TEXT,
              updated_at TIMESTAMPTZ, updated_by TEXT) AS $$
BEGIN
    RETURN QUERY EXECUTE format(
        $q$SELECT
            (SELECT action_at  FROM %1$I WHERE action = 'C' AND (new_data->>'id')::bigint = $1 ORDER BY action_at       LIMIT 1)::timestamptz,
            (SELECT action_by::text  FROM %1$I WHERE action = 'C' AND (new_data->>'id')::bigint = $1 ORDER BY action_at       LIMIT 1),
            (SELECT action_at  FROM %1$I WHERE (new_data->>'id')::bigint = $1 OR (old_data->>'id')::bigint = $1 ORDER BY action_at DESC LIMIT 1)::timestamptz,
            (SELECT action_by::text  FROM %1$I WHERE (new_data->>'id')::bigint = $1 OR (old_data->>'id')::bigint = $1 ORDER BY action_at DESC LIMIT 1)
        $q$, p_log_table
    ) USING p_entity_id;
END;
$$ LANGUAGE plpgsql STABLE;


-- ─────────────────────────────────────────────────────────────────────────────
-- 11. GRANT basic privileges to application role
-- ─────────────────────────────────────────────────────────────────────────────

-- Tables: full CRUD for app role
DO $$
DECLARE
    t TEXT;
BEGIN
    FOR t IN
        SELECT table_name FROM information_schema.tables
         WHERE table_schema = 'public'
           AND table_type   = 'BASE TABLE'
           AND table_name   LIKE 'T_%'
    LOOP
        EXECUTE format('GRANT SELECT, INSERT, UPDATE, DELETE ON %I TO contact_app', t);
    END LOOP;

    -- Sequences: app needs USAGE for BIGSERIAL inserts
    FOR t IN
        SELECT sequence_name FROM information_schema.sequences
         WHERE sequence_schema = 'public'
    LOOP
        EXECUTE format('GRANT USAGE, SELECT ON SEQUENCE %I TO contact_app', t);
    END LOOP;
END $$;

-- Functions: app can search/read, admin can purge
GRANT EXECUTE ON FUNCTION fn_search_persons(TEXT, INT, REGCONFIG)       TO contact_app;
GRANT EXECUTE ON FUNCTION fn_search_orgunits(TEXT, INT, REGCONFIG)      TO contact_app;
GRANT EXECUTE ON FUNCTION fn_search_contacts(TEXT, INT, REGCONFIG)      TO contact_app;
GRANT EXECUTE ON FUNCTION fn_mv_org_hierarchy()                         TO contact_app;
GRANT EXECUTE ON FUNCTION fn_mv_contact_qualifier()                     TO contact_app;
GRANT EXECUTE ON FUNCTION fn_mv_role()                                  TO contact_app;
GRANT EXECUTE ON FUNCTION fn_org_hierarchy(VARCHAR)                     TO contact_app;
GRANT EXECUTE ON FUNCTION fn_entity_timestamps(TEXT, BIGINT)            TO contact_app;
GRANT EXECUTE ON FUNCTION fn_refresh_locale_mvs()                       TO contact_admin;

COMMIT;
