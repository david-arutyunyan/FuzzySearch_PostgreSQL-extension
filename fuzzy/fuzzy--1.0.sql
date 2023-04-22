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


CREATE OR REPLACE FUNCTION fs(text, text)
RETURNS boolean
AS 'MODULE_PATHNAME', 'fs'
LANGUAGE C IMMUTABLE;