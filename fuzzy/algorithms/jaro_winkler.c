#include "jaro_winkler.h"


//void set_boost_factor(float value)
//{
//    boost_factor = value;
//}
//
//float get_boost_factor(void)
//{
//    return floor(boost_factor*100)/100;
//}

double jaro_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);

    if (len1 == 0 || len2 == 0) {
        return 0.0;
    }

    int match_distance = (int)floor(fmax(len1, len2) / 2.0) - 1;

    int *s1_matches = malloc(len1 * sizeof(int));
    int *s2_matches = malloc(len2 * sizeof(int));

    for (int i = 0; i < len1; i++) {
        s1_matches[i] = -1;
    }

    for (int i = 0; i < len2; i++) {
        s2_matches[i] = -1;
    }

    int matches = 0;

    for (int i = 0; i < len1; i++) {
        int start = fmax(0, i - match_distance);
        int end = fmin(i + match_distance + 1, len2);

        for (int j = start; j < end; j++) {
            if (s2_matches[j] != -1) {
                continue;
            }

            if (s1[i] != s2[j]) {
                continue;
            }

            s1_matches[i] = j;
            s2_matches[j] = i;
            matches++;
            break;
        }
    }

    if (matches == 0) {
        free(s1_matches);
        free(s2_matches);
        return 0.0;
    }

    double transpositions = 0.0;
    int index = 0;

    for (int i = 0; i < len1; i++) {
        if (s1_matches[i] == -1) {
            continue;
        }

        while (s2_matches[index] == -1) {
            index++;
        }

        if (s1[i] != s2[index]) {
            transpositions += 0.5;
        }

        index++;
    }

    double jaro_similarity = ((double)matches / len1 + (double)matches / len2 + ((double)matches - transpositions / 2) / matches) / 3;
    free(s1_matches);
    free(s2_matches);

    return jaro_similarity;
}

double jaro_winkler_distance_algo(const char *s1, const char *s2, double prefix_scale_factor, double max_distance) {
    double jaro_similarity = jaro_distance(s1, s2);
    int prefix_length = 0;

    while (s1[prefix_length] == s2[prefix_length] && prefix_length < 4) {
        prefix_length++;
    }

    double jaro_winkler_similarity = jaro_similarity + prefix_length * prefix_scale_factor * (1.0 - jaro_similarity);

    if (jaro_winkler_similarity > max_distance) {
        return max_distance;
    }

    return jaro_winkler_similarity;
}

Datum jw_dist(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("logfile.txt", "w"); // /home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);
    float scale_factor = PG_GETARG_FLOAT8(2);
    float max_distance = PG_GETARG_FLOAT8(3);

    float distance = jaro_winkler_distance_algo(str1, str2, scale_factor, max_distance);

    elog(INFO, "Jaro-Winkler distance between %s and %s is equals to %f", str1, str2, distance);

    PG_RETURN_FLOAT8(distance);
}

//Datum get_bfactor(PG_FUNCTION_ARGS) {
//    PG_RETURN_FLOAT8(floor( get_boost_factor()*100 )/100);
//}
//
//Datum set_bfactor(PG_FUNCTION_ARGS) {
//    set_boost_factor(PG_GETARG_FLOAT8(0));
//}