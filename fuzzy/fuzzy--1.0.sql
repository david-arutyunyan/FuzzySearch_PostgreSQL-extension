-- CREATE OR REPLACE FUNCTION fuzzy(s1 text, s2 text)
--   RETURNS boolean AS $$
--   DECLARE
--     m integer := length(s1);
--     n integer := length(s2);
--     d integer[][] := array_fill(0, ARRAY[m+1, n+1]);
--     i integer;
--     j integer;
--     cost integer;
--   BEGIN
--     FOR i IN 0..m LOOP
--           d[i][0] := i;
--     END LOOP;
--
--     FOR j IN 0..n LOOP
--           d[0][j] := j;
--     END LOOP;
--
--     FOR j IN 1..n LOOP
--           FOR i IN 1..m LOOP
--             IF s1[i] = s2[j] THEN
--               cost := 0;
--     ELSE
--               cost := 1;
--     END IF;
--
--             d[i][j] := LEAST(d[i-1][j] + 1, d[i][j-1] + 1, d[i-1][j-1] + cost);
--     END LOOP;
--     END LOOP;
--
--     RETURN d[m][n];
--   END;
--   $$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION fuzzy_search(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS boolean
AS 'MODULE_PATHNAME', 'fuzzy_search'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION lev_dist(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'lev_dist'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION simple_match(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'simple_match'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION trigram_match(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'trigram_match'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION jw_dist(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'jw_dist'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION bitap(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS boolean
AS 'MODULE_PATHNAME', 'bitap'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION wf(str1 VARCHAR(100), str2 VARCHAR(100))
RETURNS INTEGER
AS 'MODULE_PATHNAME', 'wf'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_tds_threshold()
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'get_threshold'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_tds_threshold(val FLOAT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_threshold'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_bfactor()
RETURNS FLOAT
AS 'MODULE_PATHNAME', 'get_bfactor'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_bfactor(val FLOAT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_bfactor'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION get_max_errors()
RETURNS INT
AS 'MODULE_PATHNAME', 'get_max_errors'
LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION set_max_errors(val INT)
RETURNS VOID
AS 'MODULE_PATHNAME', 'set_max_errors'
LANGUAGE C IMMUTABLE;