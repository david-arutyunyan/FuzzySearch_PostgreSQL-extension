#ifndef BITAP_H
#define BITAP_H

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
Datum bitap(PG_FUNCTION_ARGS);
//Datum get_max_errors(PG_FUNCTION_ARGS);
//Datum set_max_errors(PG_FUNCTION_ARGS);

#endif