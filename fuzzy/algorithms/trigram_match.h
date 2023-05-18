#ifndef TRIGRAM_MATCH_H
#define TRIGRAM_MATCH_H

#include "utils.h"

Datum trigram_match(PG_FUNCTION_ARGS);
Datum trigram_match_by_words(PG_FUNCTION_ARGS);

#endif
