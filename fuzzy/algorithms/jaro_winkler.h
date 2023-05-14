#ifndef JARO_WINKLER_H
#define JARO_WINKLER_H

#include "includes.h"

/**
 * @brief Calculate abbreviation dictionary from two columns in PostgreSQL
 *
 * @param 0: OID of full names table
 * @param 1: column of full names table
 * @param 2: OID of abbreviations table
 * @param 3: column of abbreviations table
 *
 * Returns table (see SQL definition)
 */
Datum jw_dist(PG_FUNCTION_ARGS);
//Datum get_bfactor(PG_FUNCTION_ARGS);
//Datum set_bfactor(PG_FUNCTION_ARGS);

#endif
