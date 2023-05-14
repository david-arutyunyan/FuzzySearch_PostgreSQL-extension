#include "levinshtein.h"

int levenshtein_distance_algo(const char *s1, const char *s2)
{
    size_t n = strlen(s1);
    size_t m = strlen(s2);
    int **d = (int **) malloc((n+1) * sizeof(int *));
    for (size_t i = 0; i <= n; i++) {
        d[i] = (int *) malloc((m+1) * sizeof(int));
        d[i][0] = i;
    }
    for (size_t j = 0; j <= m; j++) {
        d[0][j] = j;
    }
    for (size_t j = 1; j <= m; j++) {
        for (size_t i = 1; i <= n; i++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            d[i][j] = fmin(fmin(d[i-1][j]+1, d[i][j-1]+1), d[i-1][j-1]+cost);
        }
    }
    int result = d[n][m];
    for (size_t i = 0; i <= n; i++) {
        free(d[i]);
    }
    free(d);
    return result;
}

Datum lev_dist(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("logfile.txt", "w");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);

    int distance = levenshtein_distance_algo(str1, str2);

    elog(INFO, "Lev distance between %s and %s is equals to %d", str1, str2, distance);

    PG_RETURN_INT32(distance);
}