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



//
//                                                       Settings
//
float tds_threshold = 0.7f;
float boost_factor = 0.1f;

void set_tds_threshold(float value)
{
    tds_threshold = value;
}

float get_tds_threshold(void)
{
    return floor(tds_threshold*100)/100;
}

void set_boost_factor(float value)
{
    boost_factor = value;
}

float get_boost_factor(void)
{
    return floor(boost_factor*100)/100;
}


//
//                                                    Get abbreviation
//
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

//
//                                                Tao-Deng-Stonebraker
//
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
int tds_match_algo(char *s, int n, char *t, int m, float threshold) {
    float score = tds_similarity(s, n, t, m);
    if (score >= threshold) {
        return 1;
    }
    return 0;
}
//
//                                                   Levinshtein Distance
//
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

//                                              Simple Match
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



//
//                                                     Trigrams
//
#define MAX_TRIGRAMS 512

// Helper function to calculate the Jaccard similarity coefficient of two sets of trigrams
double jaccard_similarity(char **trigrams1, int count1, char **trigrams2, int count2) {
    int i, j;
    int intersection = 0;
    int union_size = count1 + count2;

    for (i = 0; i < count1; i++) {
        for (j = 0; j < count2; j++) {
            if (strcmp(trigrams1[i], trigrams2[j]) == 0) {
                intersection++;
                break;
            }
        }
    }

    union_size -= intersection;

    if (union_size == 0) {
        return 0;
    }

    return ((double) intersection) / union_size;
}

// Helper function to break a string into trigrams
void get_trigrams(const char *str, char **trigrams, int *count) {
    int i, j;
    int len = strlen(str);
    char buf[4] = "";

    for (i = 0; i < len - 2 && *count < MAX_TRIGRAMS; i++) {
        buf[0] = str[i];
        buf[1] = str[i+1];
        buf[2] = str[i+2];
        buf[3] = '\0';

        // check for duplicates
        for (j = 0; j < *count; j++) {
            if (strcmp(buf, trigrams[j]) == 0) {
                break;
            }
        }

        // add unique trigrams
        if (j == *count) {
            trigrams[*count] = strdup(buf);
            (*count)++;
        }
    }
}

// Main function for fuzzy matching using trigrams
double trigram_match_algo(const char *s1, const char *s2) {
    char *trigrams1[MAX_TRIGRAMS], *trigrams2[MAX_TRIGRAMS];
    int count1 = 0, count2 = 0;
    int i;
    double similarity;

    get_trigrams(s1, trigrams1, &count1);
    get_trigrams(s2, trigrams2, &count2);

    similarity = jaccard_similarity(trigrams1, count1, trigrams2, count2);

    // free memory
    for (i = 0; i < count1; i++) {
        free(trigrams1[i]);
    }

    for (i = 0; i < count2; i++) {
        free(trigrams2[i]);
    }

    return similarity;
}


//
//                                                  Jaro-Winkler distance
//
double jaro_winkler_distance_algo(const char *s1, const char *s2, double jw_boost_factor)
{
    size_t n1 = strlen(s1);
    size_t n2 = strlen(s2);
    if (n1 == 0 || n2 == 0) {
        return 0.0;
    }
    if (n1 > n2) {
        const char *tmp = s1;
        s1 = s2;
        s2 = tmp;
        size_t tmpn = n1;
        n1 = n2;
        n2 = tmpn;
    }
    size_t match_distance = (n2 / 2) - 1;
    bool *s2_matches = (bool *) calloc(n2, sizeof(bool));
    size_t matches = 0;
    for (size_t i = 0; i < n1; i++) {
        size_t start = (i >= match_distance) ? (i - match_distance) : 0;
        size_t end = (i + match_distance + 1 < n2) ? (i + match_distance + 1) : n2;
        for (size_t j = start; j < end; j++) {
            if (!s2_matches[j] && s1[i] == s2[j]) {
                s2_matches[j] = true;
                matches++;
                break;
            }
        }
    }
    free(s2_matches);
    if (matches == 0) {
        return 0.0;
    }
    double jaro_similarity = ((double) matches) / ((double) n1 + n2 - matches);
    if (jaro_similarity < 0.7) {
        return jaro_similarity;
    }
    size_t prefix_len = 0;
    for (size_t i = 0; i < n1 && i < n2 && s1[i] == s2[i]; i++) {
        prefix_len++;
    }
    double jaro_winkler_distance = jaro_similarity + (prefix_len * jw_boost_factor * (1.0 - jaro_similarity));
    return jaro_winkler_distance;
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


PG_FUNCTION_INFO_V1(get_bfactor);

Datum
get_bfactor(PG_FUNCTION_ARGS) {
    PG_RETURN_FLOAT8(floor( get_boost_factor()*100 )/100);
}


PG_FUNCTION_INFO_V1(set_bfactor);

Datum
set_bfactor(PG_FUNCTION_ARGS) {
    float boost_factor = PG_GETARG_FLOAT8(0);

    set_boost_factor(boost_factor);
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
    float threshold = get_tds_threshold(); //PG_GETARG_FLOAT8(2);

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


    PG_RETURN_BOOL(tds_match_algo(str1, len1, str2, len2, threshold));
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

    int distance = levenshtein_distance_algo(str1, str2);

    elog(INFO, "Lev distance between %s and %s equals to %d", str1, str2, distance);

    PG_RETURN_INT32(distance);
}


PG_FUNCTION_INFO_V1(simple_match);

Datum
simple_match(PG_FUNCTION_ARGS)
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

    int distance = simple_match_algo(str1, str2);

    elog(INFO, "Distance between %s and %s equals to %d", str1, str2, distance);

    PG_RETURN_INT32(distance);
}


PG_FUNCTION_INFO_V1(trigram_match);

Datum
trigram_match(PG_FUNCTION_ARGS)
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

    float distance = trigram_match_algo(str1, str2);

    elog(INFO, "Trigram distance between %s and %s equals to %f", str1, str2, floor( distance*100 )/100);

    PG_RETURN_FLOAT8(floor( distance*100 )/100);
}


PG_FUNCTION_INFO_V1(jw_dist);

Datum
jw_dist(PG_FUNCTION_ARGS)
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
    float jw_boost_factor = get_boost_factor();

    float distance = jaro_winkler_distance_algo(str1, str2, jw_boost_factor);

    elog(INFO, "Jaro-Winkler distance between %s and %s equals to %f", str1, str2, floor( distance*100 )/100);

    PG_RETURN_FLOAT8(floor( distance*100 )/100);
}