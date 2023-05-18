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

Datum simple_match_by_words(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("logfile.txt", "w");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);
    int percent = PG_GETARG_INT32(2);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);

    to_upper_case(str1);
    to_upper_case(str2);

    SplitStr sstr1 = tokenize(str1);

    int max_dist = 0;
    int distance = 0;

    for (int i = 0; i < sstr1.size; ++i) {
        distance = simple_match_algo(sstr1.words[i], str2);

        if (distance > max_dist) {
            max_dist = distance;
        }

        elog(INFO, "Distance between %s and %s is equals to %d", sstr1.words[i], str2, distance);
    }

    PG_RETURN_INT32(max_dist);
}