#ifndef BITAP_H
#define BITAP_H

#include "utils.h"

Datum bitap(PG_FUNCTION_ARGS);
Datum get_max_errors(PG_FUNCTION_ARGS);
Datum set_max_errors(PG_FUNCTION_ARGS);

#endif