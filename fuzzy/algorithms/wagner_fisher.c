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

    for (int j = 0; j <= n; j++) {
        prev_row[j] = j;
    }

    for (int i = 1; i <= m; i++) {
        curr_row[0] = i;

        for (int j = 1; j <= n; j++) {
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
    FILE *log_file = fopen("logfile.txt", "w");

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

    int distance = wagner_fischer_algo(str1, str2);

    elog(INFO, "Wagner-Fisher distance between %s and %s is equals to %d", str1, str2, distance);

    PG_RETURN_INT32(distance);
}