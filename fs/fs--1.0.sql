CREATE FUNCTION substract_integers(int, int)
RETURNS int
AS 'MODULE_PATHNAME', 'substract_integers'
LANGUAGE C IMMUTABLE;