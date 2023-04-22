#include "postgres.h"
#include "fmgr.h"
#include "utils/geo_decls.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(substract_integers);

Datum
substract_integers(PG_FUNCTION_ARGS)
{
    int a = PG_GETARG_INT32(0);
    int b = PG_GETARG_INT32(1);
    PG_RETURN_INT32(a - b);
}