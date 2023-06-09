#include "bitap.h"

int max_errors = 2;

void set_errors(int value)
{
    max_errors = value;
}

int get_errors(void)
{
    return max_errors;
}

#define ALPHABET_SIZE 75

uint64_t* generateAlphabetMasks(const char* alphabet, const char* needle) {
    uint64_t* masks = malloc(INT16_MAX);
    if (masks == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    size_t needleLen = strlen(needle);

    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        uint64_t mask = ~0;

        for (int pos = 0; pos < needleLen; ++pos) {
            if (alphabet[i] == needle[needleLen - 1 - pos]) {
                mask &= ~(1UL << pos);
            }
        }

        masks[(int)alphabet[i]] = (mask << 1);
    }

    return masks;
}

uint64_t* generateBitArray(int lev) {
    uint64_t* bitArray = malloc((lev + 1) * sizeof(uint64_t));
    if (bitArray == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    for (int k = 0; k <= lev; ++k) {
        bitArray[k] = ~1;
    }

    return bitArray;
}

bool bitap_algo(const char* haystack, const char* needle, int errors, const char* alphabet) {
    uint64_t* alphabetMasks = generateAlphabetMasks(alphabet, needle);
    size_t haystackLen = strlen(haystack);

    uint64_t* bitArray = generateBitArray(errors);

    if (bitArray == NULL) {
        return false;
    }

    for (int i = haystackLen - 1; i >= 0; --i) {
        uint64_t* old = malloc((errors + 1) * sizeof(uint64_t));
        if (old == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(bitArray);
            return false;
        }

        memcpy(old, bitArray, (errors + 1) * sizeof(uint64_t));

        bitArray[0] = (old[0] << 1) | alphabetMasks[(int)haystack[i]];

        if (errors > 0) {
            for (int k = 1; k <= errors; ++k) {
                uint64_t ins = old[k - 1];
                uint64_t sub = ins << 1;
                uint64_t del = bitArray[k - 1] << 1;
                uint64_t match = (old[k] << 1) | alphabetMasks[(int)haystack[i]];
                bitArray[k] = ins & del & sub & match;
            }
        }

        if ((bitArray[errors] & (1UL << strlen(needle))) == 0) {
            free(old);
            free(bitArray);
            return true;
        }

        free(old);
    }

    free(bitArray);
    return false;
}

Datum bitap(PG_FUNCTION_ARGS)
{
    char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.,!? &";
    FILE *log_file = fopen("bitap_logfile.txt", "a");

    if (log_file == NULL) {
        elog(ERROR, "Failed to open log file.");
        PG_RETURN_NULL();
    }

    text* text_a = PG_GETARG_TEXT_P(0);
    text* text_b = PG_GETARG_TEXT_P(1);

    char* str1 = text_to_cstring(text_a);
    char* str2 = text_to_cstring(text_b);
    int errors = get_errors();

    bool res = false;

    if (errors == 0) {
        SplitStr new_haystak = tokenize(str1);

        for (int i = 0; i < new_haystak.size; ++i) {
            if (strcmp(new_haystak.words[i], str2) == 0) {
                PG_RETURN_BOOL(true);
            }
        }
    } else {
        char* haystack = malloc((strlen(str1) + 2) * sizeof(char));
        char* needle = str2;
        strcpy(haystack, str1);
        haystack[strlen(str1)] = '&';
        haystack[strlen(str1) + 1] = '\0';

        to_upper_case(haystack);
        to_upper_case(needle);

        res = bitap_algo(haystack, needle, errors, alphabet);

        fprintf(log_file, "Bitap: haystack = %s; needle = %s; errors = %d. Match: %d\n", haystack, needle, errors, res);
    }

    fclose(log_file);

    PG_RETURN_BOOL(res);
}

Datum get_max_errors(PG_FUNCTION_ARGS) {
    PG_RETURN_INT32(get_errors());
}

Datum set_max_errors(PG_FUNCTION_ARGS) {
    set_errors(PG_GETARG_INT32(0));
}