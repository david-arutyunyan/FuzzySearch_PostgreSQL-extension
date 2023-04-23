// --------------------------------------------------------------------------------------------
#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/elog.h"
#include "utils/builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "settings.h"
#include <ctype.h>

float tds_threshold = 0.7f;

void set_tds_threshold(float value)
{
    tds_threshold = value;
}

float get_tds_threshold(void)
{
    return tds_threshold;
}

#define MAX_STRING_LENGTH 100

// Calculate the minimum of three integers
int min(int a, int b, int c) {
    int m = a;
    if (b < m) {
        m = b;
    }
    if (c < m) {
        m = c;
    }
    return m;
}

// Calculate the maximum of two integers
int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

// Calculate the edit distance between two strings
int edit_distance(char *s, int n, char *t, int m) {
    int i, j;
    int **dp = (int **)malloc(sizeof(int *) * (n + 1));
    for (i = 0; i <= n; i++) {
        dp[i] = (int *)malloc(sizeof(int) * (m + 1));
        dp[i][0] = i;
    }
    for (j = 1; j <= m; j++) {
        dp[0][j] = j;
    }
    for (i = 1; i <= n; i++) {
        for (j = 1; j <= m; j++) {
            int cost = (s[i - 1] == t[j - 1]) ? 0 : 1;
            dp[i][j] = min(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost);
        }
    }
    int distance = dp[n][m];
    for (i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);
    return distance;
}

// Calculate the Tao-Deng-Stonebraker similarity score between two strings
float tds_similarity(char *s, int n, char *t, int m) {
    int distance = edit_distance(s, n, t, m);
    float score = 1.0 - ((float)distance / (float)max(n, m));
    return score;
}

// Check if a string is a fuzzy match for another string
int is_fuzzy_match(char *s, int n, char *t, int m, float threshold) {
    float score = tds_similarity(s, n, t, m);
    if (score >= threshold) {
        return 1;
    }
    return 0;
}

int levenshtein_distance(const char *s1, const char *s2)
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

char* get_initials(char* s) {
    char* initials = malloc(100 * sizeof(char));
    int len = strlen(s);
    int index = 0;
    for (int i = 0; i < len; i++) {
        if (isalpha(s[i])) {
            initials[index++] = s[i];
            while (isalpha(s[i+1])) {
                i++;
            }
        }
    }
    initials[index] = '\0';
    return initials;
}



PG_MODULE_MAGIC;


PG_FUNCTION_INFO_V1(get_threshold);

Datum
get_threshold(PG_FUNCTION_ARGS) {
    PG_RETURN_FLOAT8(floor( get_tds_threshold()*100 )/100);

}


PG_FUNCTION_INFO_V1(set_threshold);

Datum
set_threshold(PG_FUNCTION_ARGS) {
    float threshold = PG_GETARG_FLOAT8(0);

    set_tds_threshold(threshold);
}


PG_FUNCTION_INFO_V1(fuzzy_search);

Datum
fuzzy_search(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/logfile.txt", "w");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }


    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);
    float threshold = tds_threshold; //PG_GETARG_FLOAT8(2);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);

    char* str1_init = get_initials(str1);
    char* str2_init = get_initials(str2);

    elog(INFO, "TRSLD: %f", threshold);
    elog(INFO, "My a string initials: %s", str1_init);
    elog(INFO, "My b string initials: %s", str2_init);

    int len1 = strlen(str1);
    int len2 = strlen(str2);

    fprintf(log_file, "Threshold: %f\n", threshold);
    fprintf(log_file, "My a string: %s\n", str1);
    fprintf(log_file, "My b string: %s\n", str2);

    //elog(INFO, "My a string: %s", str1);
    //elog(INFO, "My b string: %s", str2);
    //elog(INFO, "TRSLD: %f", threshold);

    pfree(str1);
    pfree(str2);

    fflush(log_file);
    fclose(log_file);


    PG_RETURN_BOOL(is_fuzzy_match(str1, len1, str2, len2, threshold));
}


PG_FUNCTION_INFO_V1(lev_dist);

Datum
lev_dist(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/logfile.txt", "w");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);

    int distance = levenshtein_distance(str1, str2);
    PG_RETURN_INT32(distance);
}