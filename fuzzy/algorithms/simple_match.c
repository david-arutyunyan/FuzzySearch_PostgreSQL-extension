#include "simple_match.h"

int simple_match_algo(const char *s1, const char *s2)
{
    size_t n = strlen(s1);
    size_t m = strlen(s2);
    size_t i = 0, j = 0, matches = 0;

    while (i < n && j < m) {
        if (s1[i] == s2[j]) {
            matches++;
            i++;
            j++;
        } else if (i < n-1 && s1[i+1] == s2[j]) {
            i++;
        } else if (j < m-1 && s1[i] == s2[j+1]) {
            j++;
        } else {
            i++;
            j++;
        }
    }

    return (matches * 100) / (n + m - matches);
}

Datum simple_match(PG_FUNCTION_ARGS)
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

    int distance = simple_match_algo(str1, str2);

    elog(INFO, "Distance between %s and %s is equals to %d", str1, str2, distance);

    PG_RETURN_INT32(distance);
}