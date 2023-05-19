#include "trigram_match.h"

#define MAX_TRIGRAMS 512

// Helper function to calculate the Jaccard similarity coefficient of two sets of trigrams
double jaccard_similarity(char **trigrams1, int count1, char **trigrams2, int count2) {
    int i, j;
    int intersection = 0;
    int union_size = count1 + count2;

    for (i = 0; i < count1; ++i) {
        for (j = 0; j < count2; ++j) {
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

    for (i = 0; i < len - 2 && *count < MAX_TRIGRAMS; ++i) {
        buf[0] = str[i];
        buf[1] = str[i+1];
        buf[2] = str[i+2];
        buf[3] = '\0';

        // check for duplicates
        for (j = 0; j < *count; ++j) {
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
    for (i = 0; i < count1; ++i) {
        free(trigrams1[i]);
    }

    for (i = 0; i < count2; ++i) {
        free(trigrams2[i]);
    }

    return similarity;
}

Datum trigram_match(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/trigram_match_logfile.txt", "a");

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

    float distance = floor(trigram_match_algo(str1, str2) * 100) / 100;

    clock_t end_time = clock();

    float elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(log_file, "%lf\n", elapsed_time);
    fclose(log_file);

    //elog(INFO, "Trigram distance between %s and %s is equals to %f", str1, str2, distance);

    PG_RETURN_FLOAT8(distance);
}


Datum trigram_match_by_words(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/trigram_match_by_words_logfile.txt", "a");

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
        distance = floor(trigram_match_algo(sstr1.words[i], str2) * 100) / 100;

        if (distance > max_dist) {
            max_dist = distance;
        }

        //elog(INFO, "Trigram distance between %s and %s is equals to %f", sstr1.words[i], str2, distance);
    }

    clock_t end_time = clock();

    float elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(log_file, "%lf\n", elapsed_time);
    fclose(log_file);

    PG_RETURN_FLOAT8(max_dist);
}