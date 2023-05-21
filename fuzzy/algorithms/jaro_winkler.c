#include "jaro_winkler.h"

float scale_factor = 0.2f;
float max_dist = 1.0f;

void set_scale_factor(float value)
{
    scale_factor = value;
}

float get_scale_factor(void)
{
    return floor(scale_factor*100)/100;
}

void set_dist(float value)
{
    max_dist = value;
}

float get_dist(void)
{
    return floor(max_dist*100)/100;
}

float jaro_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);

    if (len1 == 0 || len2 == 0) {
        return 0.0;
    }

    int match_distance = (int)floor(fmax(len1, len2) / 2.0) - 1;

    int *s1_matches = malloc(len1 * sizeof(int));
    int *s2_matches = malloc(len2 * sizeof(int));

    for (int i = 0; i < len1; ++i) {
        s1_matches[i] = -1;
    }

    for (int i = 0; i < len2; ++i) {
        s2_matches[i] = -1;
    }

    int matches = 0;

    for (int i = 0; i < len1; ++i) {
        int start = fmax(0, i - match_distance);
        int end = fmin(i + match_distance + 1, len2);

        for (int j = start; j < end; ++j) {
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

    float transpositions = 0.0;
    int index = 0;

    for (int i = 0; i < len1; ++i) {
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

    float jaro_similarity = ((float)matches / len1 + (float)matches / len2 + ((float)matches - transpositions / 2) / matches) / 3;
    free(s1_matches);
    free(s2_matches);

    return jaro_similarity;
}

float jaro_winkler_distance_algo(const char *s1, const char *s2, float prefix_scale_factor, float max_distance) {
    float jaro_similarity = jaro_distance(s1, s2);
    int prefix_length = 0;

    while (s1[prefix_length] == s2[prefix_length] && prefix_length < 4) {
        prefix_length++;
    }

    float jaro_winkler_similarity = jaro_similarity + prefix_length * prefix_scale_factor * (1.0 - jaro_similarity);

    if (jaro_winkler_similarity > max_distance) {
        return max_distance;
    }

    return jaro_winkler_similarity;
}

Datum jw_dist(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("jw_dist_logfile.txt", "a");

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


    float distance = jaro_winkler_distance_algo(str1, str2, get_scale_factor(), get_dist());

    fprintf(log_file, "Jaro-Winkler distance between %s and %s is equals to %f\n", str1, str2, distance);
    fclose(log_file);

    PG_RETURN_FLOAT8(distance);
}

Datum jw_dist_by_words(PG_FUNCTION_ARGS)
{
    if (strcmp(text_to_cstring(PG_GETARG_TEXT_P(2)), "BW") != 0) {
        elog(ERROR, "To search by words the last argument must be 'BW'");
    }

    FILE *log_file = fopen("jw_dist_by_words_logfile.txt", "a");

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

    float max_dist = 0;
    float distance = 0;

    clock_t start_time = clock();

    for (int i = 0; i < sstr1.size; ++i) {
        distance = jaro_winkler_distance_algo(sstr1.words[i],str2,get_scale_factor(), get_dist());

        if (distance > max_dist) {
            max_dist = distance;
        }

        fprintf(log_file, "Jaro-Winkler distance between %s and %s is equals to %f\n", sstr1.words[i], str2, distance);
    }

    fclose(log_file);

    PG_RETURN_FLOAT8(max_dist);
}

Datum get_sfactor(PG_FUNCTION_ARGS) {
    PG_RETURN_FLOAT8(floor( get_scale_factor()*100 )/100);
}

Datum set_sfactor(PG_FUNCTION_ARGS) {
    set_scale_factor(PG_GETARG_FLOAT8(0));
}

Datum get_max_dist(PG_FUNCTION_ARGS) {
    PG_RETURN_FLOAT8(floor( get_dist()*100 )/100);
}

Datum set_max_dist(PG_FUNCTION_ARGS) {
    set_dist(PG_GETARG_FLOAT8(0));
}