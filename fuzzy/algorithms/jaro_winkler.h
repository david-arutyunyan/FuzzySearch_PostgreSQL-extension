#ifndef JARO_WINKLER_H
#define JARO_WINKLER_H

#include "utils.h"


Datum jw_dist(PG_FUNCTION_ARGS);
Datum jw_dist_by_words(PG_FUNCTION_ARGS);
Datum get_sfactor(PG_FUNCTION_ARGS);
Datum set_sfactor(PG_FUNCTION_ARGS);
Datum get_max_dist(PG_FUNCTION_ARGS);
Datum set_max_dist(PG_FUNCTION_ARGS);

#endif
