#include "wagner_fisher.h"

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

int wagner_fischer_algo(char* s1, char* s2) {
    int m = strlen(s1);
    int n = strlen(s2);

    int* prev_row = (int*) calloc(n + 1, sizeof(int));
    int* curr_row = (int*) calloc(n + 1, sizeof(int));

    for (int j = 0; j <= n; ++j) {
        prev_row[j] = j;
    }

    for (int i = 1; i <= m; ++i) {
        curr_row[0] = i;

        for (int j = 1; j <= n; ++j) {
            int cost = s1[i-1] == s2[j-1] ? 0 : 1;
            curr_row[j] = min(
                    curr_row[j-1] + 1,
                    prev_row[j] + 1,
                    prev_row[j-1] + cost
            );
        }

        int* tmp = prev_row;
        prev_row = curr_row;
        curr_row = tmp;
    }

    int result = prev_row[n];
    free(prev_row);
    free(curr_row);

    return result;
}

Datum wf(PG_FUNCTION_ARGS)
{
    FILE *log_file = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/wf_logfile.txt", "a");

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

    int distance = wagner_fischer_algo(str1, str2);

    clock_t end_time = clock();

    float elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(log_file, "%lf\n", elapsed_time);
    fclose(log_file);

    //elog(INFO, "Wagner-Fisher distance between %s and %s is equals to %d", str1, str2, distance);

    PG_RETURN_INT32(distance);
}

Datum wf_by_words(PG_FUNCTION_ARGS)
{
    if (strcmp(text_to_cstring(PG_GETARG_TEXT_P(2)), "BW") != 0) {
        elog(ERROR, "To search by words the last argument must be 'BW'");
    }

    FILE *log_file = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/wf_by_words_logfile.txt", "a");

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

    int max_dist = 0;
    int distance = 0;

    clock_t start_time = clock();

    for (int i = 0; i < sstr1.size; ++i) {
        distance = wagner_fischer_algo(sstr1.words[i], str2);

        if (distance > max_dist) {
            max_dist = distance;
        }

        //elog(INFO, "Wagner-Fisher distance between %s and %s is equals to %d", sstr1.words[i], str2, distance);
    }

    clock_t end_time = clock();

    float elapsed_time = (float)(end_time - start_time) / CLOCKS_PER_SEC;

    fprintf(log_file, "%lf\n", elapsed_time);
    fclose(log_file);

    FILE *log = fopen("/home/daarutyunyan/hse/diploma/PostgresFuzzySearchExtension/fuzzy/logfile.txt", "a");

    fprintf(log, "%lf\n", elapsed_time);
    fclose(log);

    PG_RETURN_INT32(max_dist);
}