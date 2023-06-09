CREATE OR REPLACE FUNCTION lev_dist(VARCHAR, VARCHAR)
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'lev_dist'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION lev_dist(VARCHAR, VARCHAR, VARCHAR)
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'lev_dist_by_words'
LANGUAGE C IMMUTABLE;


CREATE OR REPLACE FUNCTION bitap(VARCHAR, VARCHAR)
RETURNS boolean
AS 'MODULE_PATHNAME', 'bitap'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_max_errors()
RETURNS INT
AS 'MODULE_PATHNAME', 'get_max_errors'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_max_errors(INT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_max_errors'
LANGUAGE C IMMUTABLE;


CREATE OR REPLACE FUNCTION simple_match(VARCHAR, VARCHAR)
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'simple_match'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION simple_match(VARCHAR, VARCHAR, VARCHAR)
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'simple_match_by_words'
LANGUAGE C IMMUTABLE;


CREATE OR REPLACE FUNCTION jw_dist(VARCHAR, VARCHAR)
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'jw_dist'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION jw_dist(VARCHAR, VARCHAR, VARCHAR)
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'jw_dist_by_words'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_sfactor()
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'get_sfactor'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_sfactor(FLOAT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_sfactor'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_max_dist()
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'get_max_dist'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_max_dist(FLOAT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_max_dist'
LANGUAGE C IMMUTABLE;


CREATE OR REPLACE FUNCTION wf(VARCHAR, VARCHAR)
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'wf'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION wf(VARCHAR, VARCHAR, VARCHAR)
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'wf_by_words'
LANGUAGE C IMMUTABLE;


CREATE OR REPLACE FUNCTION trigram_match(VARCHAR, VARCHAR)
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'trigram_match'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION trigram_match(VARCHAR, VARCHAR, VARCHAR)
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'trigram_match_by_words'
LANGUAGE C IMMUTABLE;


CREATE OR REPLACE FUNCTION abbr(oid, VARCHAR, oid, VARCHAR)
RETURNS TABLE(f VARCHAR, a VARCHAR)
AS 'MODULE_PATHNAME', 'abbr'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION tds(oid, VARCHAR, oid, VARCHAR, oid, VARCHAR, VARCHAR, REAL)
RETURNS TABLE(s1 VARCHAR, s2 VARCHAR)
AS 'MODULE_PATHNAME', 'tds'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION pkduck(VARCHAR, VARCHAR, oid, VARCHAR, VARCHAR, REAL)
RETURNS boolean
AS 'MODULE_PATHNAME', 'pkduck'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_tds_exact()
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'get_tds_exact'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_tds_exact(val FLOAT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_tds_exact'
LANGUAGE C IMMUTABLE;