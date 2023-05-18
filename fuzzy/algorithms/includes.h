#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/elog.h"
#include "utils/builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "utils/timestamp.h"
#include <stdint.h>
#include <access/htup_details.h>

//#include "settings.h"

//#include "executor/spi.h"
//#include "funcapi.h"