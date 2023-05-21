#include "levinshtein.h"

int levenshtein_distance_algo(const char *s1, const char *s2)
{
    size_t n = strlen(s1);
    size_t m = strlen(s2);
    int **d = (int **) malloc((n+1) * sizeof(int *));
    for (size_t i = 0; i <= n; ++i) {
        d[i] = (int *) malloc((m+1) * sizeof(int));
        d[i][0] = i;
    }
    for (size_t j = 0; j <= m; ++j) {
        d[0][j] = j;
    }
    for (size_t j = 1; j <= m; ++j) {
        for (size_t i = 1; i <= n; ++i) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            d[i][j] = fmin(fmin(d[i-1][j]+1, d[i][j-1]+1), d[i-1][j-1]+cost);
        }
    }
    int result = d[n][m];
    for (size_t i = 0; i <= n; ++i) {
        free(d[i]);
    }
    free(d);
    return result;
}

Datum lev_dist(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("lev_dist_logfile.txt", "a");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);

    to_upper_case(str1);
    to_upper_case(str2);

    clock_t start_time = clock();

    int distance = levenshtein_distance_algo(str1, str2);

    clock_t end_time = clock();

    float elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(log_file, "Lev distance between %s and %s is equals to %d\n", str1, str2, distance);
    fclose(log_file);

    PG_RETURN_INT32(distance);
}


Datum lev_dist_by_words(PG_FUNCTION_ARGS)
{
    if (strcmp(text_to_cstring(PG_GETARG_TEXT_P(2)), "BW") != 0) {
        elog(ERROR, "To search by words the last argument must be 'BW'");
    }

    FILE *log_file = fopen("lev_dist_by_words_logfile.txt", "a");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);

    to_upper_case(str1);
    to_upper_case(str2);

    SplitStr sstr1 = tokenize(str1);

    int min_dist = INT32_MAX;
    int distance = 0;

    for (int i = 0; i < sstr1.size; ++i) {
        distance = levenshtein_distance_algo(sstr1.words[i], str2);

        if (distance < min_dist) {
            min_dist = distance;
        }

        fprintf(log_file, "Lev distance between %s and %s is equals to %d\n", sstr1.words[i], str2, distance);
    }

    fclose(log_file);

    PG_RETURN_INT32(min_dist);
}