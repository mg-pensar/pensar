# Contact Management Application - Implementation Plan

## Overview
A comprehensive contact management system supporting Person and Organization entities, with role-based access, organizational hierarchy (composite pattern), keyword & full-text search, and secure WebAssembly frontend.

### Architecture Principles

1. **Audit via Log Tables & Triggers** — Every table `T` has a companion `T_LOG_T` that records historical row snapshots. A trigger fires on INSERT/UPDATE/DELETE, storing the old values (NULL for INSERT), the operation (`C`reate / `U`pdate / `D`elete), `action_by` (session username), and `action_at` (timestamptz). This eliminates the need for `created_at` / `updated_at` columns in the main tables; creation and last-modification timestamps are derived from the log. An admin interface can purge log records per table/date range, and search logs by action type, user, or date range.

2. **Domains for Shared Column Types** — Every column type that appears in more than one table is defined once as a PostgreSQL `DOMAIN`. Tables reference the domain instead of raw types, ensuring a single point of change.

3. **Internationalization via Localized Tables & Per-Locale Materialized Views** — Any user-visible label/name/description that needs translation is removed from the main table and placed in a companion `T_Localized<Table>` keyed by `(entity_id, locale_id)`. One materialized view per locale is created with the naming convention `MV_<View>_<locale>` (hyphens → underscores, e.g. `MV_OrgHierarchy_pt_BR`, `MV_Role_en_US`). PostgreSQL does **not** support session-scoped aliases for views; instead, session-aware accessor functions (e.g. `fn_mv_org_hierarchy()`) read the `app.locale` session variable and transparently query the correct per-locale MV. Falls back to `pt-BR` (default) when `app.locale` is not set. Refresh via `fn_refresh_locale_mvs()`.

4. **Keywords & Full-Text Search** — Every contact entity (Person, OrgUnit) carries a user-editable `keywords` text field (space-separated words) plus an auto-maintained `search_vector` (`tsvector`) column. A trigger keeps the vector in sync with both the keywords and key data columns (name, email, phone, etc.). A GIN index on the vector enables sub-millisecond full-text queries via `plainto_tsquery` / `websearch_to_tsquery`.

---

## Phase 1: Database Foundation (PostgreSQL)
**Duration: 3-4 weeks**

### 1.0 Domains (Single Source of Truth for Shared Types)

```sql
-- ── Identity & References ────────────────────────────────────────────
CREATE DOMAIN D_EntityId        AS BIGINT;            -- FK columns: person_id, org_unit_id, role_id …
CREATE DOMAIN D_QualifierId     AS SMALLINT;           -- qualifier_id in Phone, Email, Address, SocialMedia, Website
CREATE DOMAIN D_FederalTaxId    AS VARCHAR(20);        -- federal_tax_id in Person, OrgUnit
CREATE DOMAIN D_CountryCode     AS CHAR(2);            -- country in Address, passport_country in Person

-- ── Text / Name Fields ───────────────────────────────────────────────
CREATE DOMAIN D_ShortName       AS VARCHAR(50);        -- first_name, middle_name, platform …
CREATE DOMAIN D_MediumName      AS VARCHAR(100);       -- last_name, role name, handle, city, state, title, username …
CREATE DOMAIN D_LongName        AS VARCHAR(200);       -- org name, street lines …
CREATE DOMAIN D_Description     AS TEXT;               -- description in Role …
CREATE DOMAIN D_EmailAddress    AS VARCHAR(254);       -- email in T_Email
CREATE DOMAIN D_Url             AS VARCHAR(500);       -- url in SocialMedia, Website
CREATE DOMAIN D_Keywords        AS TEXT;               -- space-separated search keywords

-- ── Phone Segments ───────────────────────────────────────────────────
CREATE DOMAIN D_PhoneCountryCode AS VARCHAR(5);
CREATE DOMAIN D_PhoneAreaCode    AS VARCHAR(5);
CREATE DOMAIN D_PhoneNumber      AS VARCHAR(20);
CREATE DOMAIN D_PhoneExtension   AS VARCHAR(10);

-- ── Address Segments ─────────────────────────────────────────────────
CREATE DOMAIN D_PostalCode      AS VARCHAR(20);
CREATE DOMAIN D_GeoCoordinate   AS DECIMAL(9,6);      -- latitude, longitude

-- ── Misc ─────────────────────────────────────────────────────────────
CREATE DOMAIN D_Flag            AS BOOLEAN NOT NULL DEFAULT FALSE;  -- is_preferred, is_mobile, is_verified …
CREATE DOMAIN D_ActiveFlag      AS BOOLEAN NOT NULL DEFAULT TRUE;   -- is_active
CREATE DOMAIN D_SessionToken    AS VARCHAR(64);
CREATE DOMAIN D_IdentNumber     AS VARCHAR(30);        -- identification_number, passport_number
CREATE DOMAIN D_DataCategory    AS VARCHAR(50);        -- data_category in DataVisibility
CREATE DOMAIN D_OrgType         AS VARCHAR(20);        -- org_type code

-- ── Audit Log ────────────────────────────────────────────────────────
CREATE DOMAIN D_AuditAction     AS CHAR(1) NOT NULL CHECK (VALUE IN ('C','U','D'));
CREATE DOMAIN D_AuditUser       AS VARCHAR(100) NOT NULL;
```

### 1.0.1 Locale Reference Table

```sql
-- BCP-47 / IETF locale tags
CREATE TABLE T_Locale (
    id SMALLSERIAL PRIMARY KEY,
    code VARCHAR(10) NOT NULL UNIQUE,     -- e.g. 'pt-BR', 'en-US', 'es-AR'
    is_default D_Flag DEFAULT FALSE       -- exactly one row should be TRUE
);
INSERT INTO T_Locale (code, is_default) VALUES
    ('pt-BR', TRUE),
    ('en-US', FALSE),
    ('es-AR', FALSE),
    ('es-MX', FALSE),
    ('fr-FR', FALSE);
```

### 1.0.2 Generic Audit-Log Trigger Function

A single PL/pgSQL function is reused by every table's trigger. It dynamically inserts a row into `T_LOG_<table>` using `to_jsonb` for the old/new row snapshot.

```sql
-- Generic audit trigger function.
-- Each table must have a companion T_LOG_<table> with columns:
--   log_id BIGSERIAL PRIMARY KEY,
--   action  D_AuditAction,
--   action_by D_AuditUser,
--   action_at TIMESTAMPTZ DEFAULT NOW(),
--   old_data  JSONB,
--   new_data  JSONB
--
-- The trigger function is installed per-table via:
--   CREATE TRIGGER trg_audit_<table>
--     AFTER INSERT OR UPDATE OR DELETE ON T_<table>
--     FOR EACH ROW EXECUTE FUNCTION fn_audit_log('T_LOG_<table>');

CREATE OR REPLACE FUNCTION fn_audit_log()
RETURNS TRIGGER AS $$
DECLARE
    v_action  CHAR(1);
    v_old     JSONB := NULL;
    v_new     JSONB := NULL;
    v_user    TEXT  := current_setting('app.current_user', TRUE);
BEGIN
    IF v_user IS NULL OR v_user = '' THEN
        v_user := session_user;
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
        'INSERT INTO %I (action, action_by, action_at, old_data, new_data) VALUES ($1, $2, NOW(), $3, $4)',
        TG_ARGV[0]
    ) USING v_action, v_user, v_old, v_new;

    RETURN COALESCE(NEW, OLD);
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;
```

Convenience macro for creating a log table + trigger pair:

```sql
-- Helper: call once per audited table.
-- Example:  SELECT create_audit_log('T_Person');
--   → creates T_LOG_Person and installs the trigger.
CREATE OR REPLACE FUNCTION create_audit_log(p_table TEXT)
RETURNS VOID AS $$
DECLARE
    v_log TEXT := 'T_LOG_' || substring(p_table FROM 3);  -- strip leading 'T_'
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
        'CREATE INDEX IF NOT EXISTS idx_%s_at ON %I (action_at)', lower(v_log), v_log);

    EXECUTE format(
        'CREATE INDEX IF NOT EXISTS idx_%s_action ON %I (action)', lower(v_log), v_log);

    EXECUTE format(
        'CREATE INDEX IF NOT EXISTS idx_%s_action_by ON %I (action_by)', lower(v_log), v_log);

    EXECUTE format(
        'CREATE TRIGGER trg_audit_%s
         AFTER INSERT OR UPDATE OR DELETE ON %I
         FOR EACH ROW EXECUTE FUNCTION fn_audit_log(%L)',
        lower(substring(p_table FROM 3)), p_table, v_log);
END;
$$ LANGUAGE plpgsql;
```

### 1.0.3 Admin: Audit Log Search & Purge

```sql
-- ── Search audit logs by action type, user, and/or date range ────────
-- Returns rows from any log table matching the given filters.
-- All filter parameters are optional (pass NULL to skip).
CREATE OR REPLACE FUNCTION fn_search_audit_log(
    p_log_table  TEXT,
    p_action     CHAR(1)      DEFAULT NULL,   -- 'C', 'U', or 'D'
    p_action_by  TEXT          DEFAULT NULL,   -- username (exact match)
    p_from       TIMESTAMPTZ   DEFAULT NULL,   -- inclusive start
    p_to         TIMESTAMPTZ   DEFAULT NULL    -- exclusive end
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

-- Restrict to admin role
REVOKE ALL ON FUNCTION fn_search_audit_log(TEXT, CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ) FROM PUBLIC;
GRANT EXECUTE ON FUNCTION fn_search_audit_log(TEXT, CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ) TO admin;

-- ── Purge log rows older than a given date ───────────────────────────
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

-- Restrict to admin role
REVOKE ALL ON FUNCTION fn_purge_audit_log(TEXT, TIMESTAMPTZ) FROM PUBLIC;
GRANT EXECUTE ON FUNCTION fn_purge_audit_log(TEXT, TIMESTAMPTZ) TO admin;

-- ── Search across ALL log tables at once ─────────────────────────────
-- Scans every T_LOG_* table and unions the results.
CREATE OR REPLACE FUNCTION fn_search_all_audit_logs(
    p_action     CHAR(1)      DEFAULT NULL,
    p_action_by  TEXT          DEFAULT NULL,
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
            'SELECT %L::text AS source_table, log_id, action, action_by, action_at, old_data, new_data
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
GRANT EXECUTE ON FUNCTION fn_search_all_audit_logs(CHAR, TEXT, TIMESTAMPTZ, TIMESTAMPTZ, INT) TO admin;
```

### 1.1 Core Entity Tables

```sql
-- Person table (no created_at/updated_at — managed by audit log)
CREATE TABLE T_Person (
    id BIGSERIAL PRIMARY KEY,
    first_name D_ShortName NOT NULL,
    middle_name D_ShortName,
    last_name D_MediumName NOT NULL,
    federal_tax_id D_FederalTaxId,           -- CPF/SSN/etc
    identification_number D_IdentNumber,     -- RG/ID card
    passport_number D_IdentNumber,
    passport_country D_CountryCode,
    passport_expiry DATE,
    photo BYTEA,
    date_of_birth DATE,
    keywords D_Keywords,                     -- space-separated user search tags
    search_vector TSVECTOR                   -- auto-maintained full-text index
);
CREATE INDEX idx_person_name ON T_Person(last_name, first_name);
CREATE UNIQUE INDEX idx_person_tax_id ON T_Person(federal_tax_id) WHERE federal_tax_id IS NOT NULL;
CREATE INDEX idx_person_search ON T_Person USING GIN (search_vector);

SELECT create_audit_log('T_Person');

-- Organization Unit (composite pattern - self-referencing)
CREATE TABLE T_OrgUnit (
    id BIGSERIAL PRIMARY KEY,
    parent_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE SET NULL,
    org_type D_OrgType NOT NULL CHECK (org_type IN ('Group', 'Company', 'Department', 'Division', 'Team')),
    federal_tax_id D_FederalTaxId,
    is_active D_ActiveFlag,
    keywords D_Keywords,                     -- space-separated user search tags
    search_vector TSVECTOR                   -- auto-maintained full-text index
);
CREATE INDEX idx_orgunit_parent ON T_OrgUnit(parent_id);
CREATE INDEX idx_orgunit_type ON T_OrgUnit(org_type);
CREATE INDEX idx_orgunit_search ON T_OrgUnit USING GIN (search_vector);

SELECT create_audit_log('T_OrgUnit');

-- OrgUnit localized fields (name)
CREATE TABLE T_LocalizedOrgUnit (
    id BIGSERIAL PRIMARY KEY,
    org_unit_id D_EntityId NOT NULL REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    locale_id SMALLINT NOT NULL REFERENCES T_Locale(id),
    name D_LongName NOT NULL,
    UNIQUE(org_unit_id, locale_id)
);
CREATE INDEX idx_loc_orgunit ON T_LocalizedOrgUnit(org_unit_id);
```

### 1.1.1 Full-Text Search Infrastructure

The `search_vector` columns are maintained automatically by triggers that combine the entity's own fields with its keywords. The FTS configuration uses the `'simple'` dictionary so keywords work across languages without stemming surprises; switch to a language-specific config if stemming is desired.

```sql
-- ── Person search vector trigger ─────────────────────────────────────
-- Builds the tsvector from: first_name, middle_name, last_name,
-- federal_tax_id, identification_number, keywords.
CREATE OR REPLACE FUNCTION fn_person_search_vector()
RETURNS TRIGGER AS $$
BEGIN
    NEW.search_vector :=
        setweight(to_tsvector('simple', COALESCE(NEW.first_name, '')),  'A') ||
        setweight(to_tsvector('simple', COALESCE(NEW.middle_name, '')), 'B') ||
        setweight(to_tsvector('simple', COALESCE(NEW.last_name, '')),   'A') ||
        setweight(to_tsvector('simple', COALESCE(NEW.federal_tax_id, '')), 'C') ||
        setweight(to_tsvector('simple', COALESCE(NEW.identification_number, '')), 'C') ||
        setweight(to_tsvector('simple', COALESCE(NEW.keywords, '')),    'B');
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_person_search_vector
    BEFORE INSERT OR UPDATE OF first_name, middle_name, last_name,
                                federal_tax_id, identification_number, keywords
    ON T_Person
    FOR EACH ROW EXECUTE FUNCTION fn_person_search_vector();

-- ── OrgUnit search vector trigger ────────────────────────────────────
-- Rebuilds the tsvector from: keywords + localized name (default locale).
-- The localized name is fetched from T_LocalizedOrgUnit so the vector
-- stays accurate even when the name changes. A trigger on
-- T_LocalizedOrgUnit also refreshes the parent OrgUnit's vector.
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
        setweight(to_tsvector('simple', COALESCE(v_name, '')),        'A') ||
        setweight(to_tsvector('simple', COALESCE(NEW.federal_tax_id, '')), 'C') ||
        setweight(to_tsvector('simple', COALESCE(NEW.keywords, '')),  'B');
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_orgunit_search_vector
    BEFORE INSERT OR UPDATE OF federal_tax_id, keywords
    ON T_OrgUnit
    FOR EACH ROW EXECUTE FUNCTION fn_orgunit_search_vector();

-- When the localized name changes, refresh the parent OrgUnit's vector.
CREATE OR REPLACE FUNCTION fn_localized_orgunit_refresh_search()
RETURNS TRIGGER AS $$
BEGIN
    UPDATE T_OrgUnit SET keywords = keywords WHERE id = COALESCE(NEW.org_unit_id, OLD.org_unit_id);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_localized_orgunit_refresh
    AFTER INSERT OR UPDATE OR DELETE ON T_LocalizedOrgUnit
    FOR EACH ROW EXECUTE FUNCTION fn_localized_orgunit_refresh_search();

-- ── Convenience search functions ─────────────────────────────────────
-- Search persons by free-text query (uses websearch_to_tsquery for
-- natural-language boolean syntax: "john OR smith", "john -doe", etc.)
CREATE OR REPLACE FUNCTION fn_search_persons(
    p_query       TEXT,
    p_limit       INT DEFAULT 50,
    p_ts_config   REGCONFIG DEFAULT 'simple'
) RETURNS TABLE(
    id            BIGINT,
    first_name    D_ShortName,
    last_name     D_MediumName,
    keywords      D_Keywords,
    rank          REAL
) AS $$
BEGIN
    RETURN QUERY
    SELECT p.id, p.first_name, p.last_name, p.keywords,
           ts_rank_cd(p.search_vector, websearch_to_tsquery(p_ts_config, p_query)) AS rank
    FROM T_Person p
    WHERE p.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
    ORDER BY rank DESC
    LIMIT p_limit;
END;
$$ LANGUAGE plpgsql STABLE;

-- Search org units by free-text query.
CREATE OR REPLACE FUNCTION fn_search_orgunits(
    p_query       TEXT,
    p_limit       INT DEFAULT 50,
    p_ts_config   REGCONFIG DEFAULT 'simple'
) RETURNS TABLE(
    id            BIGINT,
    name          D_LongName,
    org_type      D_OrgType,
    keywords      D_Keywords,
    rank          REAL
) AS $$
BEGIN
    RETURN QUERY
    SELECT o.id, loc.name, o.org_type, o.keywords,
           ts_rank_cd(o.search_vector, websearch_to_tsquery(p_ts_config, p_query)) AS rank
    FROM T_OrgUnit o
    JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
    JOIN T_Locale l ON l.id = loc.locale_id AND l.is_default = TRUE
    WHERE o.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
    ORDER BY rank DESC
    LIMIT p_limit;
END;
$$ LANGUAGE plpgsql STABLE;

-- Unified contact search (persons + orgs) returning a common shape.
CREATE OR REPLACE FUNCTION fn_search_contacts(
    p_query       TEXT,
    p_limit       INT DEFAULT 50,
    p_ts_config   REGCONFIG DEFAULT 'simple'
) RETURNS TABLE(
    entity_type   TEXT,
    entity_id     BIGINT,
    display_name  TEXT,
    keywords      D_Keywords,
    rank          REAL
) AS $$
BEGIN
    RETURN QUERY
    (
        SELECT 'Person'::text, p.id,
               TRIM(COALESCE(p.first_name,'') || ' ' || COALESCE(p.last_name,'')),
               p.keywords,
               ts_rank_cd(p.search_vector, websearch_to_tsquery(p_ts_config, p_query))
        FROM T_Person p
        WHERE p.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
    )
    UNION ALL
    (
        SELECT 'OrgUnit'::text, o.id,
               loc.name::text,
               o.keywords,
               ts_rank_cd(o.search_vector, websearch_to_tsquery(p_ts_config, p_query))
        FROM T_OrgUnit o
        JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
        JOIN T_Locale l ON l.id = loc.locale_id AND l.is_default = TRUE
        WHERE o.search_vector @@ websearch_to_tsquery(p_ts_config, p_query)
    )
    ORDER BY rank DESC
    LIMIT p_limit;
END;
$$ LANGUAGE plpgsql STABLE;
```

### 1.2 Contact Information Tables

```sql
-- Contact Qualifier Enum Table (id only; label is localized)
CREATE TABLE T_ContactQualifier (
    id SMALLINT PRIMARY KEY
);
INSERT INTO T_ContactQualifier VALUES (1), (2), (3);

-- Localized qualifier labels
CREATE TABLE T_LocalizedContactQualifier (
    id BIGSERIAL PRIMARY KEY,
    qualifier_id D_QualifierId NOT NULL REFERENCES T_ContactQualifier(id) ON DELETE CASCADE,
    locale_id SMALLINT NOT NULL REFERENCES T_Locale(id),
    name D_ShortName NOT NULL,            -- 'Home' / 'Casa' / 'Maison' …
    UNIQUE(qualifier_id, locale_id)
);
-- locale_id: 1=pt-BR, 2=en-US, 3=es-AR, 4=es-MX, 5=fr-FR
INSERT INTO T_LocalizedContactQualifier (qualifier_id, locale_id, name) VALUES
    (1, 1, 'Casa'),       (1, 2, 'Home'),     (1, 3, 'Hogar'),    (1, 4, 'Hogar'),    (1, 5, 'Maison'),
    (2, 1, 'Comercial'),  (2, 2, 'Business'), (2, 3, 'Comercial'), (2, 4, 'Comercial'), (2, 5, 'Professionnel'),
    (3, 1, 'Outro'),      (3, 2, 'Other'),    (3, 3, 'Otro'),     (3, 4, 'Otro'),     (3, 5, 'Autre');

-- Phone Numbers
CREATE TABLE T_Phone (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId REFERENCES T_Person(id) ON DELETE CASCADE,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    country_code D_PhoneCountryCode NOT NULL,
    area_code D_PhoneAreaCode,
    number D_PhoneNumber NOT NULL,
    extension D_PhoneExtension,
    is_mobile D_Flag DEFAULT FALSE,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    CONSTRAINT chk_phone_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL AND org_unit_id IS NOT NULL)
    )
);
CREATE INDEX idx_phone_person ON T_Phone(person_id);
CREATE INDEX idx_phone_org ON T_Phone(org_unit_id);

SELECT create_audit_log('T_Phone');

-- Email Addresses
CREATE TABLE T_Email (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId REFERENCES T_Person(id) ON DELETE CASCADE,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    email D_EmailAddress NOT NULL,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    is_verified D_Flag DEFAULT FALSE,
    CONSTRAINT chk_email_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL AND org_unit_id IS NOT NULL)
    )
);
CREATE UNIQUE INDEX idx_email_unique ON T_Email(email);
CREATE INDEX idx_email_person ON T_Email(person_id);

SELECT create_audit_log('T_Email');

-- Physical Addresses
CREATE TABLE T_Address (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId REFERENCES T_Person(id) ON DELETE CASCADE,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    street_line1 D_LongName NOT NULL,
    street_line2 D_LongName,
    city D_MediumName NOT NULL,
    state_province D_MediumName,
    postal_code D_PostalCode,
    country D_CountryCode NOT NULL,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    latitude D_GeoCoordinate,
    longitude D_GeoCoordinate,
    CONSTRAINT chk_address_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL AND org_unit_id IS NOT NULL)
    )
);
CREATE INDEX idx_address_person ON T_Address(person_id);
CREATE INDEX idx_address_org ON T_Address(org_unit_id);

SELECT create_audit_log('T_Address');

-- Social Media
CREATE TABLE T_SocialMedia (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId REFERENCES T_Person(id) ON DELETE CASCADE,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    platform D_ShortName NOT NULL,         -- LinkedIn, Twitter, Instagram …
    handle D_MediumName NOT NULL,
    url D_Url,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    CONSTRAINT chk_social_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL AND org_unit_id IS NOT NULL)
    )
);

SELECT create_audit_log('T_SocialMedia');

-- Websites
CREATE TABLE T_Website (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId REFERENCES T_Person(id) ON DELETE CASCADE,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    url D_Url NOT NULL,
    title D_MediumName,
    qualifier_id D_QualifierId REFERENCES T_ContactQualifier(id) DEFAULT 3,
    is_preferred D_Flag DEFAULT FALSE,
    CONSTRAINT chk_website_owner CHECK (
        (person_id IS NOT NULL AND org_unit_id IS NULL) OR
        (person_id IS NULL AND org_unit_id IS NOT NULL)
    )
);

SELECT create_audit_log('T_Website');
```

### 1.3 Roles and Relationships

```sql
-- Role Definition (name & description are localized)
CREATE TABLE T_Role (
    id BIGSERIAL PRIMARY KEY,
    org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    is_active D_ActiveFlag
);
CREATE INDEX idx_role_org ON T_Role(org_unit_id);

SELECT create_audit_log('T_Role');

-- Role localized fields
CREATE TABLE T_LocalizedRole (
    id BIGSERIAL PRIMARY KEY,
    role_id D_EntityId NOT NULL REFERENCES T_Role(id) ON DELETE CASCADE,
    locale_id SMALLINT NOT NULL REFERENCES T_Locale(id),
    name D_MediumName NOT NULL,
    description D_Description,
    UNIQUE(role_id, locale_id)
);
CREATE INDEX idx_loc_role ON T_LocalizedRole(role_id);

-- Person-Organization Relationship
CREATE TYPE relationship_type AS ENUM ('Employee', 'Trainee', 'MinorApprentice', 'ThirdParty');

CREATE TABLE T_PersonOrgRelation (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId NOT NULL REFERENCES T_Person(id) ON DELETE CASCADE,
    org_unit_id D_EntityId NOT NULL REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    relationship relationship_type NOT NULL,
    start_date DATE NOT NULL,
    end_date DATE,
    is_active D_ActiveFlag,
    UNIQUE(person_id, org_unit_id, relationship)
);
CREATE INDEX idx_personorg_person ON T_PersonOrgRelation(person_id);
CREATE INDEX idx_personorg_org ON T_PersonOrgRelation(org_unit_id);

SELECT create_audit_log('T_PersonOrgRelation');

-- Person-Role Assignment (many-to-many)
CREATE TABLE T_PersonRole (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId NOT NULL REFERENCES T_Person(id) ON DELETE CASCADE,
    role_id D_EntityId NOT NULL REFERENCES T_Role(id) ON DELETE CASCADE,
    assigned_date DATE DEFAULT CURRENT_DATE,
    end_date DATE,
    is_primary D_Flag DEFAULT FALSE,
    UNIQUE(person_id, role_id)
);
CREATE INDEX idx_personrole_person ON T_PersonRole(person_id);
CREATE INDEX idx_personrole_role ON T_PersonRole(role_id);

SELECT create_audit_log('T_PersonRole');
```

### 1.4 Access Control Tables

```sql
-- Data Visibility Control (who can see what)
CREATE TABLE T_DataVisibility (
    id BIGSERIAL PRIMARY KEY,
    owner_person_id D_EntityId NOT NULL REFERENCES T_Person(id) ON DELETE CASCADE,
    data_category D_DataCategory NOT NULL,  -- 'Phone', 'Email', 'Address', 'Personal', 'All'
    
    -- Visibility grants (exactly one must be set)
    viewer_person_id D_EntityId REFERENCES T_Person(id) ON DELETE CASCADE,
    viewer_role_id D_EntityId REFERENCES T_Role(id) ON DELETE CASCADE,
    viewer_org_unit_id D_EntityId REFERENCES T_OrgUnit(id) ON DELETE CASCADE,
    
    can_view D_Flag DEFAULT TRUE,
    can_edit D_Flag DEFAULT FALSE,
    
    CONSTRAINT chk_single_viewer CHECK (
        (viewer_person_id IS NOT NULL)::int +
        (viewer_role_id IS NOT NULL)::int +
        (viewer_org_unit_id IS NOT NULL)::int = 1
    )
);
CREATE INDEX idx_visibility_owner ON T_DataVisibility(owner_person_id);
CREATE INDEX idx_visibility_viewer_person ON T_DataVisibility(viewer_person_id);
CREATE INDEX idx_visibility_viewer_role ON T_DataVisibility(viewer_role_id);
CREATE INDEX idx_visibility_viewer_org ON T_DataVisibility(viewer_org_unit_id);

SELECT create_audit_log('T_DataVisibility');
```

### 1.5 Useful Views & Per-Locale Materialized Views

PostgreSQL does **not** support session-scoped aliases that make one view name resolve to different materialized views per connection. Instead, one MV per locale is created with the naming convention `MV_<View>_<locale>` (hyphens replaced by underscores, e.g. `MV_OrgHierarchy_pt_BR`). Session-aware **accessor functions** read the session variable `app.locale` and dynamically query the matching MV:

```sql
-- At session / transaction start:
SET LOCAL "app.locale" = 'en-US';

-- Then simply call the accessor functions — they resolve to the
-- correct per-locale MV transparently:
SELECT * FROM fn_mv_org_hierarchy();       -- → MV_OrgHierarchy_en_US
SELECT * FROM fn_mv_contact_qualifier();   -- → MV_ContactQualifier_en_US
SELECT * FROM fn_mv_role();                -- → MV_Role_en_US
```

If `app.locale` is not set, the functions fall back to `pt-BR` (the default locale).

```sql
-- Person with preferred contact info + keywords
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
LEFT JOIN T_Email e ON e.person_id = p.id AND e.is_preferred = TRUE;

-- ── Per-Locale Materialized Views ────────────────────────────────────
-- One MV per locale: MV_<View>_<locale> (hyphens → underscores).
-- Created by iterating T_Locale rows.

-- OrgHierarchy: one MV per locale
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
                 JOIN T_Locale l ON l.id = loc.locale_id AND l.code = %L
                 WHERE o.parent_id IS NULL
                 UNION ALL
                 SELECT o.id, o.parent_id, loc.name, o.org_type, o.keywords,
                        t.depth + 1, t.path || o.id
                 FROM T_OrgUnit o
                 JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
                 JOIN T_Locale l ON l.id = loc.locale_id AND l.code = %L
                 JOIN org_tree t ON o.parent_id = t.id
             )
             SELECT * FROM org_tree',
            v_mv, v_rec.code, v_rec.code);

        EXECUTE format('CREATE INDEX idx_%s_id     ON %I (id)',        lower(v_mv), v_mv);
        EXECUTE format('CREATE INDEX idx_%s_parent  ON %I (parent_id)', lower(v_mv), v_mv);
    END LOOP;
END $$;

-- ContactQualifier: one MV per locale
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
             JOIN T_Locale l ON l.id = loc.locale_id AND l.code = %L',
            v_mv, v_rec.code);
    END LOOP;
END $$;

-- Role: one MV per locale
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
             JOIN T_Locale l ON l.id = loc.locale_id AND l.code = %L',
            v_mv, v_rec.code);
    END LOOP;
END $$;

-- ── Session-Aware Accessor Functions ─────────────────────────────────
-- Read current_setting('app.locale') and query the matching per-locale MV.
-- Fall back to 'pt-BR' when app.locale is not set.
--
-- Usage:
--   SET LOCAL "app.locale" = 'en-US';
--   SELECT * FROM fn_mv_org_hierarchy();       -- → MV_OrgHierarchy_en_US
--   SELECT * FROM fn_mv_contact_qualifier();   -- → MV_ContactQualifier_en_US
--   SELECT * FROM fn_mv_role();                -- → MV_Role_en_US

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

-- ── Locale-parameterised OrgUnit hierarchy (live query, any locale) ──
CREATE OR REPLACE FUNCTION fn_org_hierarchy(p_locale_code VARCHAR DEFAULT 'pt-BR')
RETURNS TABLE(id BIGINT, parent_id BIGINT, name D_LongName, org_type D_OrgType,
              keywords D_Keywords, depth INT, path BIGINT[]) AS $$
BEGIN
    RETURN QUERY
    WITH RECURSIVE org_tree AS (
        SELECT o.id, o.parent_id, loc.name, o.org_type, o.keywords,
               1 AS depth, ARRAY[o.id] AS path
        FROM T_OrgUnit o
        JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
        JOIN T_Locale l ON l.id = loc.locale_id AND l.code = p_locale_code
        WHERE o.parent_id IS NULL
        UNION ALL
        SELECT o.id, o.parent_id, loc.name, o.org_type, o.keywords,
               t.depth + 1, t.path || o.id
        FROM T_OrgUnit o
        JOIN T_LocalizedOrgUnit loc ON loc.org_unit_id = o.id
        JOIN T_Locale l ON l.id = loc.locale_id AND l.code = p_locale_code
        JOIN org_tree t ON o.parent_id = t.id
    )
    SELECT * FROM org_tree;
END;
$$ LANGUAGE plpgsql STABLE;

-- ── Refresh all per-locale MVs (call after bulk data changes) ────────
CREATE OR REPLACE FUNCTION fn_refresh_locale_mvs()
RETURNS VOID AS $$
DECLARE
    v_rec  RECORD;
    v_sfx  TEXT;
BEGIN
    FOR v_rec IN SELECT code FROM T_Locale ORDER BY id LOOP
        v_sfx := replace(v_rec.code, '-', '_');
        EXECUTE format('REFRESH MATERIALIZED VIEW %I', 'MV_OrgHierarchy_'    || v_sfx);
        EXECUTE format('REFRESH MATERIALIZED VIEW %I', 'MV_ContactQualifier_' || v_sfx);
        EXECUTE format('REFRESH MATERIALIZED VIEW %I', 'MV_Role_'             || v_sfx);
    END LOOP;
END;
$$ LANGUAGE plpgsql;

-- Convenience: created_at / updated_at derived from audit log
CREATE OR REPLACE FUNCTION fn_entity_timestamps(p_log_table TEXT, p_entity_id BIGINT)
RETURNS TABLE(created_at TIMESTAMPTZ, created_by D_AuditUser,
              updated_at TIMESTAMPTZ, updated_by D_AuditUser) AS $$
BEGIN
    RETURN QUERY EXECUTE format(
        'SELECT
            (SELECT action_at FROM %I WHERE action = ''C'' AND (new_data->>''id'')::bigint = $1 ORDER BY action_at LIMIT 1),
            (SELECT action_by FROM %I WHERE action = ''C'' AND (new_data->>''id'')::bigint = $1 ORDER BY action_at LIMIT 1),
            (SELECT action_at FROM %I WHERE (new_data->>''id'')::bigint = $1 OR (old_data->>''id'')::bigint = $1 ORDER BY action_at DESC LIMIT 1),
            (SELECT action_by FROM %I WHERE (new_data->>''id'')::bigint = $1 OR (old_data->>''id'')::bigint = $1 ORDER BY action_at DESC LIMIT 1)',
        p_log_table, p_log_table, p_log_table, p_log_table
    ) USING p_entity_id;
END;
$$ LANGUAGE plpgsql STABLE;
```

---

## Phase 2: Security & Authentication
**Duration: 2-3 weeks**

### 2.1 Key Management Schema

```sql
-- User Credentials (password-protected keypairs)
CREATE TABLE T_UserAuth (
    id BIGSERIAL PRIMARY KEY,
    person_id D_EntityId UNIQUE NOT NULL REFERENCES T_Person(id) ON DELETE CASCADE,
    username D_MediumName UNIQUE NOT NULL,
    public_key BYTEA NOT NULL,                -- OpenSSL public key (PEM/DER)
    encrypted_private_key BYTEA NOT NULL,     -- AES-256 encrypted with password-derived key
    key_salt BYTEA NOT NULL,                  -- For PBKDF2 key derivation
    key_iterations INT DEFAULT 100000,        -- PBKDF2 iterations
    last_login TIMESTAMPTZ,
    is_active D_ActiveFlag
);

SELECT create_audit_log('T_UserAuth');

-- Session Tokens
CREATE TABLE T_Session (
    id BIGSERIAL PRIMARY KEY,
    user_auth_id D_EntityId NOT NULL REFERENCES T_UserAuth(id) ON DELETE CASCADE,
    session_token D_SessionToken NOT NULL UNIQUE,
    expires_at TIMESTAMPTZ NOT NULL,
    ip_address INET,
    user_agent TEXT
);
CREATE INDEX idx_session_token ON T_Session(session_token);
CREATE INDEX idx_session_expires ON T_Session(expires_at);

SELECT create_audit_log('T_Session');
```

### 2.2 C++ Security Module (contact/security/)

```
contact/security/
├── keypair.hpp          - OpenSSL RSA/EC key pair management
├── key_storage.hpp      - Password-encrypted key storage
├── auth_manager.hpp     - Authentication flow
├── session.hpp          - Session token management
├── tls_context.hpp      - TLS/SSL context for encrypted comms
└── crypto_util.hpp      - AES encryption, PBKDF2, signing
```

Key operations:
1. **Key generation**: RSA-4096 or EC P-384 on first registration
2. **Key encryption**: PBKDF2 + AES-256-GCM with user password
3. **Authentication**: Challenge-response using private key signature
4. **Session**: JWT or opaque tokens signed by server

---

## Phase 3: Backend API Layer
**Duration: 3-4 weeks**

### 3.1 SOCI Data Access Layer

```
contact/data/
├── db_connection.hpp    - SOCI session management & app.current_user propagation
├── person_dao.hpp       - Person CRUD operations
├── org_dao.hpp          - OrgUnit CRUD with hierarchy
├── contact_dao.hpp      - Phone/Email/Address operations
├── role_dao.hpp         - Role management
├── search_dao.hpp       - Full-text & keyword search operations
├── visibility_dao.hpp   - Access control queries
├── locale_dao.hpp       - Locale & localized-table operations
├── audit_dao.hpp        - Audit log queries, search & purge interface
└── dao_factory.hpp      - DAO instantiation
```

> **Note:** Every DAO that writes to the database must call `SET LOCAL "app.current_user" = '<username>'` at the start of the transaction so the audit trigger can capture `action_by`. For locale-aware reads, also set `SET LOCAL "app.locale" = '<locale>'` (e.g. `'pt-BR'`) so session-aware accessor functions (`fn_mv_org_hierarchy()`, `fn_mv_contact_qualifier()`, `fn_mv_role()`) return data in the correct language.

### 3.2 Service Layer

```
contact/service/
├── person_service.hpp   - Business logic for persons
├── org_service.hpp      - Organization management
├── role_service.hpp     - Role assignment logic
├── search_service.hpp   - Contact search (keyword & full-text)
├── auth_service.hpp     - Authentication service
├── visibility_service.hpp - Access control checks
├── locale_service.hpp   - Locale management & MV refresh
└── audit_service.hpp    - Audit log browsing, search & admin purge
```

### 3.3 Communication Layer (Encrypted)

```
contact/net/
├── message.hpp          - Protocol buffer / binary message format
├── server.hpp           - ASIO-based TCP/TLS server
├── client.hpp           - Client-side connection
├── command_handler.hpp  - Request routing
└── serialization.hpp    - Binary serialization helpers
```

---

## Phase 4: WebAssembly Frontend
**Duration: 4-5 weeks**

### 4.1 Dear ImGui Bundle Setup

```
contact/ui/
├── main_wasm.cpp        - Emscripten entry point
├── app.hpp              - Application state
├── imgui_config.hpp     - ImGui configuration
├── fonts/               - Embedded fonts
└── textures/            - UI textures
```

### 4.2 UI Components

```
contact/ui/components/
├── organogram.hpp       - Organizational chart visualization
├── person_card.hpp      - Rounded rectangle person view
├── person_detail.hpp    - Expanded person info
├── role_panel.hpp       - Role assignment UI
├── search_bar.hpp       - Contact search (keywords & full-text)
├── visibility_editor.hpp - Privacy settings UI
├── auth_dialog.hpp      - Login/registration
├── audit_log_viewer.hpp - Browse, search & purge audit logs (admin)
└── locale_selector.hpp  - Language selector & locale management
```

### 4.3 Organogram Component Design

```cpp
// Collapsed card: rounded rect with photo, name, preferred mobile
struct PersonCard {
    ImTextureID photo;
    std::string name;
    std::string mobile;
    bool is_expanded;
    
    void render(ImDrawList* draw_list, ImVec2 pos);
};

// Expanded view: full person info + roles
struct PersonDetailPanel {
    Person person;
    std::vector<Role> roles;
    
    void render();
};
```

---

## Phase 5: Integration & Testing
**Duration: 2-3 weeks**

### 5.1 Test Structure

```
contact/test/
├── db_test.cpp          - Database integration tests
├── audit_log_test.cpp   - Audit trigger, search & purge tests
├── search_test.cpp      - Keyword & full-text search tests
├── locale_test.cpp      - Localized table & MV refresh tests
├── domain_test.cpp      - Domain constraint tests
├── security_test.cpp    - Crypto and auth tests
├── api_test.cpp         - Service layer tests
├── visibility_test.cpp  - Access control tests
└── e2e_test.cpp         - End-to-end scenarios
```

### 5.2 Build Configuration

```cmake
# WebAssembly build
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(contact_ui PRIVATE
        -sUSE_GLFW=3
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
    )
endif()
```

---

## Phase 6: Deployment
**Duration: 1-2 weeks**

### 6.1 Components
- **PostgreSQL**: Docker container or managed service
- **Backend Server**: Native binary with TLS
- **Frontend**: Static WASM/HTML/JS served via CDN

### 6.2 Configuration
- Environment-based configuration (dev/staging/prod)
- TLS certificate management
- Database connection pooling

---

## Implementation Checklist

### Phase 1: Database
- [ ] Create domain definitions (D_EntityId, D_FederalTaxId, D_Flag, D_Keywords, etc.)
- [ ] Create T_Locale reference table & seed data
- [ ] Create generic audit-log trigger function (fn_audit_log)
- [ ] Create helper function create_audit_log() with action, action_by, action_at indexes
- [ ] Create admin search function fn_search_audit_log() with action/user/date filters
- [ ] Create admin cross-table search fn_search_all_audit_logs()
- [ ] Create admin purge function fn_purge_audit_log()
- [ ] Create PostgreSQL schema scripts (all tables)
- [ ] Add keywords (D_Keywords) and search_vector (TSVECTOR) to T_Person and T_OrgUnit
- [ ] Create GIN indexes on search_vector columns
- [ ] Create search vector triggers (fn_person_search_vector, fn_orgunit_search_vector)
- [ ] Create search helper functions (fn_search_persons, fn_search_orgunits, fn_search_contacts)
- [ ] Create T_Localized* tables for OrgUnit, ContactQualifier, Role
- [ ] Install audit triggers for every table via create_audit_log()
- [ ] Implement migration system
- [ ] Set up SOCI connection with app.current_user propagation
- [ ] Create base DAO classes
- [ ] Write T_Person CRUD (with keywords)
- [ ] Write T_OrgUnit with hierarchy & localized names (with keywords)
- [ ] Write contact info tables
- [ ] Write role/relationship tables with localized fields
- [ ] Write visibility tables
- [ ] Create per-locale materialized views (MV_OrgHierarchy_*, MV_ContactQualifier_*, MV_Role_*)
- [ ] Create session-aware accessor functions (fn_mv_org_hierarchy, fn_mv_contact_qualifier, fn_mv_role)
- [ ] Create fn_refresh_locale_mvs() helper
- [ ] Create fn_entity_timestamps utility function
- [ ] Create fn_org_hierarchy locale-parameterised function (default pt-BR)
- [ ] Write search_dao for keyword & full-text search
- [ ] Write audit_dao for log browsing, search & purge

### Phase 2: Security
- [ ] OpenSSL key generation
- [ ] Password-based key encryption
- [ ] Challenge-response auth
- [ ] Session management
- [ ] TLS context setup
- [ ] Integration tests

### Phase 3: Backend
- [ ] SOCI DAOs complete (including search_dao, audit_dao)
- [ ] Service layer implementation (including search_service, audit_service)
- [ ] ASIO server setup
- [ ] TLS integration
- [ ] API message protocol
- [ ] Request handlers
- [ ] Error handling

### Phase 4: Frontend
- [ ] Emscripten build setup
- [ ] ImGui integration
- [ ] TLS client in WASM
- [ ] Person card component
- [ ] Organogram layout
- [ ] Expanded detail view
- [ ] Auth dialogs
- [ ] Search bar (keywords & full-text)
- [ ] Visibility editor
- [ ] Audit log viewer / search / admin purge UI
- [ ] Locale selector component

### Phase 5: Testing
- [ ] Unit tests all modules
- [ ] Audit log trigger, search & purge tests
- [ ] Keyword & full-text search tests (ranking, edge cases)
- [ ] Localized table & materialized view refresh tests
- [ ] Domain constraint tests
- [ ] Integration tests
- [ ] End-to-end tests
- [ ] Performance testing (FTS with large data sets)

### Phase 6: Deployment
- [ ] Docker configuration
- [ ] CI/CD pipeline
- [ ] Production deployment

---

## Technology Stack Summary

| Layer | Technology |
|-------|------------|
| Database | PostgreSQL 15+ |
| DB Access | SOCI 4.x |
| Backend | C++23, Boost.Asio |
| Security | OpenSSL 3.x |
| Frontend | Dear ImGui + Emscripten |
| Build | CMake, Ninja |
| Testing | Custom test framework |

---

## Estimated Timeline

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| 1. Database | 3-4 weeks | None |
| 2. Security | 2-3 weeks | Phase 1 |
| 3. Backend | 3-4 weeks | Phases 1-2 |
| 4. Frontend | 4-5 weeks | Phase 3 |
| 5. Testing | 2-3 weeks | Phases 1-4 |
| 6. Deployment | 1-2 weeks | Phase 5 |

**Total: 15-21 weeks (4-5 months)**
