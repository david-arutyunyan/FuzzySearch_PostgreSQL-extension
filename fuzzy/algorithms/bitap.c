#include "bitap.h"

//void set_errors(int value)
//{
//    max_errors = value;
//}
//
//int get_errors(void)
//{
//    return max_errors;
//}

//typedef unsigned long long int bit_vector;

bool bitap_algo(const char *str1, const char *str2, int k) {
    int n = strlen(str1), m = strlen(str2);
    uint32_t mask, last = 0, d0 = ~0, d1, d2, *d;

    int i, j;

    d = (uint32_t *) calloc((k + 1), sizeof(uint32_t));
//    if (d == NULL) {
//        fprintf(stderr, "Error: Unable to allocate memory\n");
//        exit(EXIT_FAILURE);
//    }

    for (i = 0; i < m; i++) {
        mask = 1 << i;
        last |= mask;
        d0 &= ~mask;
        for (j = 0; j <= k; j++) {
            d[j] = ((d[j] << 1) | 1) & (str2[i] == str1[j] ? d0 : d0 | mask);
            if (j > 0) {
                d[j] |= ((d[j - 1] << 1) | 1) | mask;
                d[j] &= d1;
            }
        }
        d0 = (d0 << 1) | 1;
        d1 = (d1 << 1) | 1;
        d1 |= d[k];
    }

    bool match = false;
    for (i = 0; i <= k; i++) {
        if ((d[k - i] & (1 << (n - 1))) != 0) {
            match = true;
            break;
        }
    }

    free(d);

    return match;
}

//bool bitap_algo(char* pattern, char* text, int errors) {
//    int m = strlen(pattern);
//    bit_vector mask[128] = {0}; // initialize mask to 0
//    bit_vector R = ~(1 << (m - 1));
//    int i, j;
//
//    // Preprocessing
//    for (i = 0; i < m; i++) {
//        mask[pattern[i]] |= 1 << i;
//    }
//
//    // Searching
//    for (i = 0; text[i] != '\0'; i++) {
//        bit_vector old_R = R;
//        R |= mask[text[i]];
//        R <<= 1;
//        if ((R & (1 << m)) == 0) {
//            return true;
//        } else if ((old_R & (1 << m)) && errors > 0) {
//            if (bitap_algo(pattern, text + i + 1, errors - 1)) {
//                return true;
//            }
//        }
//    }
//    return false;
//}

Datum bitap(PG_FUNCTION_ARGS)
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
    int errors = PG_GETARG_INT32(2);

    fflush(log_file);
    fclose(log_file);

    //elog(INFO, "ERS: %d", errors);

    PG_RETURN_BOOL(bitap_algo(str1, str2, errors));
}

//Datum get_max_errors(PG_FUNCTION_ARGS) {
//    PG_RETURN_INT32(get_errors());
//}
//
//Datum set_max_errors(PG_FUNCTION_ARGS) {
//    set_errors(PG_GETARG_INT32(0));
//}