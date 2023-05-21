#ifndef TDS_H
#define TDS_H

#include "utils.h"

Datum calc_dict(PG_FUNCTION_ARGS);
Datum calc_pairs(PG_FUNCTION_ARGS);
Datum pkduck(PG_FUNCTION_ARGS);
Datum get_tds_exact(PG_FUNCTION_ARGS);
Datum set_tds_exact(PG_FUNCTION_ARGS);

#endif
