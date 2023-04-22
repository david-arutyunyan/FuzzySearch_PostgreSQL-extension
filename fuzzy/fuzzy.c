//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#define MAX_STRING_LENGTH 100
//
//// Calculate the minimum of three integers
//int min(int a, int b, int c) {
//    int m = a;
//    if (b < m) {
//        m = b;
//    }
//    if (c < m) {
//        m = c;
//    }
//    return m;
//}
//
//// Calculate the maximum of two integers
//int max(int a, int b) {
//    if (a > b) {
//        return a;
//    }
//    return b;
//}
//
//// Calculate the edit distance between two strings
//int edit_distance(char *s, int n, char *t, int m) {
//    int i, j;
//    int **dp = (int **)malloc(sizeof(int *) * (n + 1));
//    for (i = 0; i <= n; i++) {
//        dp[i] = (int *)malloc(sizeof(int) * (m + 1));
//        dp[i][0] = i;
//    }
//    for (j = 1; j <= m; j++) {
//        dp[0][j] = j;
//    }
//    for (i = 1; i <= n; i++) {
//        for (j = 1; j <= m; j++) {
//            int cost = (s[i - 1] == t[j - 1]) ? 0 : 1;
//            dp[i][j] = min(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost);
//        }
//    }
//    int distance = dp[n][m];
//    for (i = 0; i <= n; i++) {
//        free(dp[i]);
//    }
//    free(dp);
//    return distance;
//}
//
//// Calculate the Tao-Deng-Stonebraker similarity score between two strings
//float tds_similarity(char *s, int n, char *t, int m) {
//    int distance = edit_distance(s, n, t, m);
//    float score = 1.0 - ((float)distance / (float)max(n, m));
//    return score;
//}
//
//// Check if a string is a fuzzy match for another string
//int is_fuzzy_match(char *s, int n, char *t, int m, float threshold) {
//    float score = tds_similarity(s, n, t, m);
//    if (score >= threshold) {
//        return 1;
//    }
//    return 0;
//}
//
//int main() {
//    char s[MAX_STRING_LENGTH];
//    char t[MAX_STRING_LENGTH];
//    float threshold;
//    printf("Enter the string to search: ");
//    fgets(s, MAX_STRING_LENGTH, stdin);
//    s[strcspn(s, "\n")] = '\0';
//    printf("Enter the threshold: ");
//    scanf("%f", &threshold);
//    printf("Enter the string to compare: ");
//    getchar();
//    fgets(t, MAX_STRING_LENGTH, stdin);
//    t[strcspn(t, "\n")] = '\0';
//    int n = strlen(s);
//    int m = strlen(t);
//    if (is_fuzzy_match(s, n, t, m, threshold)) {
//        printf("%s is a fuzzy match for %s\n", s, t);
//    } else {
//        printf("%s is not a fuzzy match for %s\n", s, t);
//    }
//    return 0;
//}


//#include "postgres.h"
//#include "fmgr.h"
//#include "funcapi.h"
//#include "access/hash.h"
//#include "utils/builtins.h"
//#include "utils/array.h"
//#include "utils/typcache.h"
//#include "utils/varlena.h"
//#include "tsearch/ts_utils.h"
//#include "utils/memutils.h"
//
//PG_MODULE_MAGIC;
//
//PG_FUNCTION_INFO_V1(fuzzy);
//
//Datum fuzzy(PG_FUNCTION_ARGS)
//{
//    text *col_name = PG_GETARG_TEXT_P(0);
//    text *search_term = PG_GETARG_TEXT_P(1);
//
//    char *col_name_str = text_to_cstring(col_name);
//    char *search_term_str = text_to_cstring(search_term);
//
//    int col_name_len = strlen(col_name_str);
//    int search_term_len = strlen(search_term_str);
//
//    int i, j, m, n, k, match;
//    double score, max_score = -1;
//
//    /* Allocate memory for a dynamic 2D array */
//    double **matrix = (double **) palloc((search_term_len + 1) * sizeof(double *));
//    for (i = 0; i <= search_term_len; i++)
//    {
//        matrix[i] = (double *) palloc((col_name_len + 1) * sizeof(double));
//    }
//
//    /* Initialize the first row and column */
//    for (i = 0; i <= search_term_len; i++)
//    {
//        matrix[i][0] = i;
//    }
//    for (j = 0; j <= col_name_len; j++)
//    {
//        matrix[0][j] = j;
//    }
//
//    /* Compute the matrix */
//    for (i = 1; i <= search_term_len; i++)
//    {
//        for (j = 1; j <= col_name_len; j++)
//        {
//            /* Calculate the match score */
//            if (search_term_str[i - 1] == col_name_str[j - 1])
//            {
//                match = 1;
//            }
//            else
//            {
//                match = 0;
//            }
//
//            /* Calculate the cost of substitutions, insertions and deletions */
//            double substitution_cost = matrix[i - 1][j - 1] + (1 - match);
//            double insertion_cost = matrix[i][j - 1] + 1;
//            double deletion_cost = matrix[i - 1][j] + 1;
//
//            /* Find the minimum cost */
//            matrix[i][j] = Min(substitution_cost, Min(insertion_cost, deletion_cost));
//        }
//    }
//
//    /* Compute the maximum score */
//    for (j = 1; j <= col_name_len; j++)
//    {
//        if (matrix[search_term_len][j] <= search_term_len * 0.2)
//        {
//            score = (1.0 - matrix[search_term_len][j] / search_term_len);
//            if (score > max_score)
//            {
//                max_score = score;
//            }
//        }
//    }
//
//    /* Clean up memory */
//    for (i = 0; i <= search_term_len; i++)
//    {
//        pfree(matrix[i]);
//    }
//    pfree(matrix);
//    pfree(col_name_str);
//    pfree(search_term_str);
//
//    PG_RETURN_BOOL(max_score >= 0.7);
//}



//#include "postgres.h"
//#include "fmgr.h"
//#include "utils/geo_decls.h"
//#include "utils/builtins.h"
//
//PG_MODULE_MAGIC;
//
//PG_FUNCTION_INFO_V1(fuzzy);
//
//Datum
//fuzzy(PG_FUNCTION_ARGS)
//{
//    text *a = PG_GETARG_TEXT_P(0);
//    text *b = PG_GETARG_TEXT_P(1);
//
//    char *aa = text_to_cstring(a);
//    char *bb = text_to_cstring(b);
//
//
//    PG_RETURN_BOOL(strlen(aa) > strlen(bb));
//}

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(fs);

Datum
fs(PG_FUNCTION_ARGS)
{
    text     *a = PG_GETARG_TEXT_PP(0);
    text     *b = PG_GETARG_TEXT_PP(1);

    int32 arg1_size = VARSIZE_ANY_EXHDR(a);
    int32 arg2_size = VARSIZE_ANY_EXHDR(b);

    PG_RETURN_BOOL(arg1_size > arg2_size);
}