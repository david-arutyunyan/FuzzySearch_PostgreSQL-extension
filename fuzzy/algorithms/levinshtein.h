#ifndef LEVINSHTEIN_H
#define LEVINSHTEIN_H

#include "utils.h"

Datum lev_dist(PG_FUNCTION_ARGS);
Datum lev_dist_by_words(PG_FUNCTION_ARGS);

#endif
