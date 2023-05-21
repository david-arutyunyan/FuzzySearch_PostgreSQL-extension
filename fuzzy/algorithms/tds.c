#include "tds.h"

int exact = 2;

void set_exactness(float value)
{
    exact = value;
}

float get_exactness(void)
{
    return exact;
}

typedef struct {
    int size;
    char*** rs;
} RuleSeq;

typedef struct {
    bool applies;
    int aside;
    int rside;
} SubRuleApplication;

typedef struct {
    SubRuleApplication a_f;
    SubRuleApplication f_a;
    const char** rule;
} RuleApplication;

int prefix_sig_length(int tokens_total, float exactness)
{
    return (int)(floor((1.0f - exactness) * tokens_total)) + 1;
}

SplitStr prefix_sig(const char* string, float exactness)
{
    SplitStr temp;
    SplitStr result = {0, NULL};

    temp = tokenize(string);
    pg_qsort((void*)temp.words, temp.size, sizeof(*temp.words), cmp_ss);
    result.size = prefix_sig_length(temp.size, exactness);
    result.size = Min(result.size, temp.size);
    result.words = palloc(sizeof(*result.words) * result.size);
    for (int i = 0; i < result.size; i++) {
        result.words[i] = temp.words[i];
    }
    for (int i = result.size; i < temp.size; i++) {
        pfree(temp.words[i]);
    }

    return result;
}

RuleApplication rule_apply(const char** rule, SplitStr ts, int ts_endpos)
{
    RuleApplication result = (RuleApplication){
            (SubRuleApplication){false, 0, 0},
            (SubRuleApplication){false, 0, 0},
            rule
    };
    const char* rule_A = rule[0];
    const char* rule_F = rule[1];
    SplitStr rule_F_tokenized;
    bool f_a_applies = true;

    ts_endpos = ts_endpos >= ts.size ? ts.size - 1 : ts_endpos;

    rule_F_tokenized = tokenize(rule_F);

    if (strcmp(rule_A, ts.words[ts_endpos]) == 0) {
        result.a_f = (SubRuleApplication){
                true,
                1,
                rule_F_tokenized.size
        };
    }

    for (int i = 0; i < rule_F_tokenized.size; i++) {
        int ts_currpos = ts_endpos - (rule_F_tokenized.size - 1) + i;
        if (ts_currpos < 0) {
            f_a_applies = false;
            break;
        }
        if (strcmp(rule_F_tokenized.words[i], ts.words[ts_currpos]) != 0) {
            f_a_applies = false;
            break;
        }
    }
    if (f_a_applies) {
        result.f_a = (SubRuleApplication) {
                true,
                rule_F_tokenized.size,
                1
        };
    }

    return result;
}

long calculate_g(SplitStr s, long i, long l, const char* t, bool* t_present, RuleSeq rules)
{
    long result = (long)INT_MAX;
    long result_current;

    if (l <= 0 || i < 0) {
        if (*t_present && l == 0) {
            result = 0;
        }
        return result;
    }

    {
        int comparation_result;
        comparation_result = cmp(s.words[i >= s.size ? s.size - 1 : i], t);
        if (comparation_result > 0) {
            result_current = calculate_g(s, i - 1, l - 1, t, t_present, rules);
        } else if (comparation_result < 0) {
            result_current = calculate_g(s, i - 1, l - 1, t, t_present, rules) + 1;
        } else {
            *t_present = true;
            result_current = calculate_g(s, i - 1, l - 1, t, t_present, rules);
        }
    }

    if (*t_present) {
        result = Min(result, result_current);
    }

    {
        result_current = (long)INT_MAX;

        for (int j = 0; j < rules.size; j++) {
            RuleApplication ra;
            long result_current_rule = (long)INT_MAX;

            ra = rule_apply((const char**)rules.rs[j], s, i);

            if (ra.a_f.applies) {
                SplitStr rule_ts;
                int ts_less = 0;
                int comparation_result;

                rule_ts = tokenize(ra.rule[1]);

                for (int t_i = 0; t_i < rule_ts.size; t_i++) {
                    comparation_result = cmp(rule_ts.words[t_i], t);
                    if (comparation_result == 0) {
                        *t_present = true;
                    }
                    if (comparation_result < 0) {
                        ts_less += 1;
                    }
                }

                result_current_rule = calculate_g(s, i - ra.a_f.aside, l - ra.a_f.rside, t, t_present, rules) + ts_less;
                result_current = Min(
                        result_current,
                        result_current_rule
                );
            }

            if (ra.f_a.applies) {
                int ts_less = 0;
                int comparation_result;

                comparation_result = cmp(ra.rule[0], t);
                if (comparation_result == 0) {
                    *t_present = true;
                }
                if (comparation_result < 0) {
                    ts_less += 1;
                }

                result_current_rule = calculate_g(s, i - ra.f_a.aside, l - ra.f_a.rside, t, t_present, rules) + ts_less;
                result_current = Min(
                        result_current,
                        result_current_rule
                );
            }
        }
    }

    if (*t_present) {
        result = Min(result, result_current);
    }

    return result;
}

int cmp_ulong_pair_wrapped(const void* a, const void* b)
{
    int vA[2];
    int vB[2];

    for (unsigned char j = 0; j < 2; j++) {
        vA[j] = (*(const int**)a)[j];
        vB[j] = (*(const int**)b)[j];
    }

    return vA[0] == vB[0] ? vA[1] - vB[1] : vA[0] - vB[0];
}

int remove_duplicate_joins(int*** joins_ptr, int joins_size)
{
    int** joins = *joins_ptr;

    bool* joins_to_stay;
    int joins_to_stay_count;
    int joins_to_stay_insert_index;

    if (joins == NULL || joins_size == 0) {
        return joins_size;
    }

    pg_qsort(joins, joins_size, sizeof(*joins), cmp_ulong_pair_wrapped);

    joins_to_stay = palloc(sizeof(*joins_to_stay) * joins_size);
    joins_to_stay[0] = true;
    joins_to_stay_count = 1;
    for (int i = 1; i < joins_size; i++) {
        if (cmp_ulong_pair_wrapped(&joins[i - 1], &joins[i]) == 0) {
            joins_to_stay[i] = false;
        } else {
            joins_to_stay[i] = true;
            joins_to_stay_count += 1;
        }
    }

    if (joins_to_stay_count != joins_size) {
        joins_to_stay_insert_index = 0;
        for (int i = 0; i < joins_size; i++) {
            if (joins_to_stay[i]) {
                joins[joins_to_stay_insert_index] = joins[i];
                joins_to_stay_insert_index += 1;
            } else {
                pfree(joins[i]);
            }
        }
        joins = repalloc(joins, joins_to_stay_count);
        *joins_ptr = joins;
    }

    return joins_to_stay_count;
}

StringPairRows do_calc_pairs(Oid t1oid, const char* t1col, Oid t2oid, const char* t2col, Oid tRoid, const char* tRcol_abbr, const char* tRcol_full, float exactness, MemoryContext rescontext)
{
    char* t1;
    char* t2;
    char* tR;

    char query[4096];

    char** rows[2];
    SplitStr* rows_signatures[2];
    int rows_used[2] = {0, 0};

    RuleSeq rules = {0, NULL};
    int longest_rule_length = 0;

    int** joins;
    int joins_used = 0;

    StringPairRows results;

    MemoryContext oldcontext;

    t1 = get_table_name(t1oid);
    t2 = get_table_name(t2oid);
    tR = get_table_name(tRoid);

    sprintf(query, "SELECT %s FROM %s;", t1col, t1);
    if (SPI_execute(query, true, 0) < 0 || SPI_tuptable == NULL) {
        elog(ERROR, "Could not SELECT rows from table '%s' (OID %d).", t1, t1oid);
    }
    elog(INFO, "Processing %d rows in first source...", SPI_processed);
    rows[0] = palloc(sizeof(*rows[0]) * SPI_processed);
    for (uint32 i = 0; i < SPI_processed; i++) {
        char* row = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 1);
        if (row == NULL) {
            continue;
        }
        rows[0][rows_used[0]++] = row;
    }
    SPI_freetuptable(SPI_tuptable);

    sprintf(query, "SELECT %s FROM %s;", t2col, t2);
    if (SPI_execute(query, true, 0) < 0 || SPI_tuptable == NULL) {
        elog(ERROR, "Could not SELECT rows from table '%s' (OID %d).", t2, t2oid);
    }
    elog(INFO, "Processing %d rows in second source...", SPI_processed);
    rows[1] = palloc(sizeof(*rows[1]) * SPI_processed);
    for (uint32 i = 0; i < SPI_processed; i++) {
        char* row = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 1);
        if (row == NULL) {
            continue;
        }
        rows[1][rows_used[1]++] = row;
    }
    SPI_freetuptable(SPI_tuptable);

    sprintf(query, "SELECT %s, %s FROM %s;", tRcol_abbr, tRcol_full, tR);
    if (SPI_execute(query, true, 0) < 0 || SPI_tuptable == NULL) {
        elog(ERROR, "Could not SELECT rows from table '%s' (OID %d).", tR, tRoid);
    }
    elog(INFO, "%d rules found, processing...", SPI_processed);
    rules.rs = palloc(sizeof(*rules.rs) * SPI_processed);
    for (uint32 i = 0; i < SPI_processed; i++) {
        char* temp[2];
        SplitStr rule_full_seq;
        temp[0] = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 1);
        temp[1] = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 2);
        if (temp[0] == NULL || temp[1] == NULL) {
            continue;
        }
        rules.rs[rules.size] = palloc(sizeof(rules.rs[rules.size]) * 2);
        for (unsigned char j = 0; j < 2; j++) {
            rules.rs[rules.size][j] = temp[j];
        }
        rules.size += 1;
        rule_full_seq = tokenize(temp[1]);
        longest_rule_length = Max(longest_rule_length, rule_full_seq.size);
    }
    SPI_freetuptable(SPI_tuptable);

    elog(INFO, "Calculating prefix signatures...");
    for (unsigned char j = 0; j < 2; j++) {
        rows_signatures[j] = palloc(sizeof(*rows_signatures[j]) * rows_used[j]);
        for (int i = 0; i < rows_used[j]; i++) {
            rows_signatures[j][i] = prefix_sig(rows[j][i], exactness);
        }
    }

    elog(INFO, "Calculating joins...");
    joins = palloc(sizeof(*joins) * rows_used[0] * rows_used[1] * 2);
    for (int i = 0; i < rows_used[0] * rows_used[1] * 2; i++) {
        joins[i] = palloc(sizeof(*joins[i]) * 2);
    }

    for (unsigned char j = 0; j < 2; j++) {
        const unsigned char ROW_PF_INDEX = j;
        const unsigned char ROW_U_INDEX = 1 - j;

        for (int pf_i = 0; pf_i < rows_used[ROW_PF_INDEX]; pf_i++) {
            for (int u_i = 0; u_i < rows_used[ROW_U_INDEX]; u_i++) {
                SplitStr seq = rows_signatures[ROW_PF_INDEX][pf_i];
                SplitStr seq_u = rows_signatures[ROW_U_INDEX][u_i];
                for (int token_i = 0; token_i < seq.size; token_i++) {
                    const char* token = seq.words[token_i];
                    for (long l = seq_u.size + longest_rule_length; l > 0; l--) {
                        bool t_present = false;
                        long g = calculate_g(seq_u, seq_u.size - 1, l, token, &t_present, rules);
                        if (t_present && (g + 1 <= prefix_sig_length(l, exactness))) {
                            joins[joins_used][ROW_PF_INDEX] = pf_i;
                            joins[joins_used][ROW_U_INDEX] = u_i;
                            joins_used += 1;
                            token_i = seq.size;
                            break;
                        }
                    }
                }
            }
        }
    }

    elog(INFO, "Removing duplicates...");
    joins_used = remove_duplicate_joins(&joins, joins_used);

    oldcontext = MemoryContextSwitchTo(rescontext);
    results.size = joins_used;
    results.read = 0;
    results.pairs = palloc(sizeof(*results.pairs) * results.size);
    for (int i = 0; i < joins_used; i++) {
        results.pairs[i] = palloc(sizeof(*results.pairs[results.read]) * 2);
        for (unsigned char j = 0; j < 2; j++) {
            results.pairs[i][j] = palloc(strlen(rows[j][joins[i][j]]) + 1);
            strcpy(results.pairs[i][j], rows[j][joins[i][j]]);
        }
    }
    MemoryContextSwitchTo(oldcontext);

    return results;
}


Datum calc_pairs(PG_FUNCTION_ARGS)
{
    MemoryContext oldcontext;

    FuncCallContext *funcctx;
    TupleDesc tupdesc;
    Datum result;
    StringPairRows* pairs;

    if (SRF_IS_FIRSTCALL())
    {
        Oid t1oid;
        Oid t2oid;
        Oid tRoid;
        char* t1col;
        char* t2col;
        char* tRcol_full;
        char* tRcol_abbr;
        float exactness;

        StringPairRows calculated;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo((*funcctx).multi_call_memory_ctx);
        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE) {
            ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("Function returning record called in context that cannot accept type record")));
        }
        (*funcctx).attinmeta = TupleDescGetAttInMetadata(tupdesc);
        (*funcctx).user_fctx = palloc(sizeof(StringPairRows));

        t1oid = PG_GETARG_OID(0);
        t1col = text_to_cstring(PG_GETARG_TEXT_P(1));
        t2oid = PG_GETARG_OID(2);
        t2col = text_to_cstring(PG_GETARG_TEXT_P(3));
        tRoid = PG_GETARG_OID(4);
        tRcol_full = text_to_cstring(PG_GETARG_TEXT_P(5));
        tRcol_abbr = text_to_cstring(PG_GETARG_TEXT_P(6));
        exactness = PG_GETARG_FLOAT8(7);

        SPI_connect();
        calculated = do_calc_pairs(t1oid, t1col, t2oid, t2col, tRoid, tRcol_abbr, tRcol_full, exactness, (*funcctx).multi_call_memory_ctx);
        SPI_finish();
        *(StringPairRows*)(*funcctx).user_fctx = calculated;

        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();

    pairs = (StringPairRows*)(*funcctx).user_fctx;

    if ((*pairs).read >= (*pairs).size) {
        SRF_RETURN_DONE(funcctx);
    }

    result = HeapTupleGetDatum(BuildTupleFromCStrings((*funcctx).attinmeta, (*pairs).pairs[(*pairs).read++]));

    SRF_RETURN_NEXT(funcctx, result);
}

int compare_pair(const void* l, const void* r)
{
    const char** pair_l = *(const char***)l;
    const char** pair_r = *(const char***)r;
    int result = strcmp(pair_l[1], pair_r[1]);
    if (result == 0) {
        result = strcmp(pair_l[0], pair_r[0]);
    }
    return result;
}

StringPairRows do_calc_dict(const Oid fullOid, const char* fullCol, const Oid abbrOid, const char* abbrCol)
{
    char* fullTable;
    char* abbrTable;

    char query[4096];

    StringPairRows result = {.size = 0, .read = 0, .pairs = NULL};

    char** abbrs = NULL;
    int abbrs_used = 0;

    char*** pairs = NULL;
    int pairs_used = 0;

    fullTable = get_table_name(fullOid);
    abbrTable = get_table_name(abbrOid);

    struct trie* trie = trie_create(255);

    sprintf(query, "SELECT %s FROM %s;", abbrCol, fullTable);
    if (SPI_execute(query, true, 0) < 0 || SPI_tuptable == NULL) {
        elog(ERROR, "Could not SELECT from abbreviations table '%s' (OID %d).", fullTable, fullOid);
    }
    elog(INFO, "Processing %d rows of abbreviations...", SPI_processed);

    abbrs = palloc(sizeof(*abbrs) * SPI_processed);
    for (int i = 0; i < SPI_processed; i++) {
        char* row = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 1);
        if (row == NULL) {
            continue;
        }
        trie_insert(trie, row, row);
        abbrs[abbrs_used] = row;
        abbrs_used += 1;
    }
    if (abbrs_used == 0) {
        elog(ERROR, "No abbreviations found in given table and column.");
    }

    SPI_freetuptable(SPI_tuptable);

    sprintf(query, "SELECT %s FROM %s;", fullCol, abbrTable);
    if (SPI_execute(query, true, 0) < 0 || SPI_tuptable == NULL) {
        elog(ERROR, "Could not SELECT from full forms table '%s' (OID %d).", abbrTable, abbrOid);
    }
    elog(INFO, "Processing %d rows of full forms...", SPI_processed);

    for (int i = 0; i < SPI_processed; i++) {
        char* row = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 1);

        char** subsequences = NULL;
        int subsequences_found;

        if (row == NULL) {
            continue;
        }

        subsequences_found = trie_search(trie, row, 0,&subsequences);
        if (subsequences_found != 0) {
            pairs = pairs == NULL ?
                    palloc(subsequences_found * sizeof(*pairs)) :
                    repalloc(pairs, (pairs_used + subsequences_found) * sizeof(*pairs));
            for (int s = 0; s < subsequences_found; s++) {
                char** current_pair = palloc(sizeof(*current_pair) * 2);
                current_pair[0] = palloc(strlen(row) + 1);
                strcpy(current_pair[0], row);
                current_pair[1] = palloc(strlen(subsequences[s]) + 1);
                strcpy(current_pair[1], subsequences[s]);
                pfree(subsequences[s]);

                pairs[pairs_used] = current_pair;
                pairs_used += 1;
            }
            pfree(subsequences);
        }

        pfree(row);
    }

    SPI_freetuptable(SPI_tuptable);

    if (pairs_used == 0) {
        elog(INFO, "No abbreviation rules found");
        return result;
    }

    result.pairs = pairs;
    result.size = pairs_used;

    return result;
}

void remove_duplicate_pairs(StringPairRows* dict)
{
    bool* pairs_to_stay;
    int pairs_to_stay_count = 1;

    if ((*dict).pairs == NULL || (*dict).size == 0) {
        return;
    }

    pg_qsort((void*)(*dict).pairs, (*dict).size, sizeof(*(*dict).pairs), compare_pair);

    pairs_to_stay = palloc(sizeof(*pairs_to_stay) * (*dict).size);
    pairs_to_stay[0] = true;
    for (int i = 1; i < (*dict).size; i++) {

        if (compare_pair(&(*dict).pairs[i - 1], &(*dict).pairs[i]) == 0) {
            pairs_to_stay[i] = false;
        } else {
            pairs_to_stay[i] = true;
            pairs_to_stay_count += 1;
        }
    }

    if (pairs_to_stay_count != (*dict).size) {
        char*** pairs_duplicate;
        int pairs_duplicate_used = 0;

        pairs_duplicate = palloc(sizeof(*pairs_duplicate) * pairs_to_stay_count);
        for (int i = 0; i < (*dict).size; i++) {
            if (pairs_to_stay[i]) {
                pairs_duplicate[pairs_duplicate_used] = (*dict).pairs[i];
                pairs_duplicate_used += 1;
            } else {
                pfree((*dict).pairs[i][0]);
                pfree((*dict).pairs[i][1]);
                pfree((*dict).pairs[i]);
            }
        }

        pfree((*dict).pairs);
        (*dict).pairs = pairs_duplicate;
        (*dict).size = pairs_to_stay_count;
    }
}

Datum calc_dict(PG_FUNCTION_ARGS)
{
    MemoryContext oldcontext;
    FuncCallContext *funcctx;
    TupleDesc tupdesc;
    Datum result;
    StringPairRows* dict;

    if (SRF_IS_FIRSTCALL())
    {
        Oid fullOid;
        Oid abbrOid;
        char* fullCol;
        char* abbrCol;

        StringPairRows calculated;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo((*funcctx).multi_call_memory_ctx);
        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE) {
            ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED), errmsg("Function returning record called in context that cannot accept type record")));
        }
        (*funcctx).attinmeta = TupleDescGetAttInMetadata(tupdesc);
        (*funcctx).user_fctx = palloc(sizeof(StringPairRows));

        fullOid = PG_GETARG_OID(0);
        abbrOid = PG_GETARG_OID(2);
        fullCol = text_to_cstring(PG_GETARG_TEXT_P(1));
        abbrCol = text_to_cstring(PG_GETARG_TEXT_P(3));

        SPI_connect();
        calculated = do_calc_dict(fullOid, fullCol, abbrOid, abbrCol);
        SPI_finish();
        Assert(calculated.pairs != NULL);
        remove_duplicate_pairs(&calculated);

        dict = (*funcctx).user_fctx;
        (*dict).size = calculated.size;
        (*dict).read = 0;
        (*dict).pairs = palloc(sizeof(*(*dict).pairs) * calculated.size);
        for (int i = 0; i < calculated.size; i++) {
            (*dict).pairs[i] = palloc(sizeof(*(*dict).pairs[i]) * 2);
            for (int j = 0; j < 2; j++) {
                (*dict).pairs[i][j] = palloc(strlen(calculated.pairs[i][j]) + 1);
                strcpy((*dict).pairs[i][j], calculated.pairs[i][j]);
            }
        }

        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();

    dict = (*funcctx).user_fctx;

    if ((*dict).read >= (*dict).size) {
        SRF_RETURN_DONE(funcctx);
    }

    result = HeapTupleGetDatum(BuildTupleFromCStrings((*funcctx).attinmeta, (*dict).pairs[(*dict).read]));
    (*dict).read += 1;

    SRF_RETURN_NEXT(funcctx, result);
}


typedef struct {
    SplitStr a;
    SplitStr r;
} Rule;

typedef struct {
    int size;
    Rule *rules;
} RuleSequence;

float apply_rule(SplitStr *s1, SplitStr *s2, Rule rule, bool modify_sequences, int *tokens_shared, int *tokens_thrown) {
    float usefullness;

    int rule_i = 0;
    for (int i = 0; i < (*s1).size; i++) {
        if (strcmp(rule.a.words[rule_i], (*s1).words[i]) == 0) {
            rule_i += 1;
            if (rule_i == rule.a.size) {
                break;
            }
        }
    }
    if (rule_i != rule.a.size) {
        return -1.0f;
    }

    int common_tokens = 0;
    float rsize = rule.r.size;

    for (int i_rule = 0; i_rule < rule.r.size; i_rule++) {
        for (long i_s2 = 0; i_s2 < (*s2).size; i_s2++) {
            if (strcmp(rule.r.words[i_rule], (*s2).words[i_s2]) == 0) {
                common_tokens += 1;
                if (modify_sequences) {
                    remove_element(s2, i_s2);
                    i_s2 -= 1;
                }
                break;
            }
        }
    }

    usefullness = common_tokens / rsize;

    if (modify_sequences) {
        int rule_i = 0;
        for (long i = 0; i < (*s1).size; i++) {
            if (strcmp(rule.a.words[rule_i], (*s1).words[i]) == 0) {
                rule_i += 1;
                remove_element(s1, i);
                i -= 1;
                if (rule_i == rule.a.size) {
                    break;
                }
            }
        }
        *tokens_shared += common_tokens;
        *tokens_thrown += rule.r.size - common_tokens;
    }

    return usefullness;
}

float calc_pkduck(SplitStr *s1, SplitStr *s2, const RuleSequence *rules_ptr) {
    int tokens_similar = 0;
    int tokens_thrown = 0;
    int tokens_shared = 0;

    while (true) {
        float max_usefullness = -0.5f;
        int max_index = 0;
        for (int rule_i = 0; rule_i < (*rules_ptr).size; rule_i++) {
            float curr_usefullness = apply_rule(s1, s2, (*rules_ptr).rules[rule_i], false, NULL, NULL);
            if (curr_usefullness > max_usefullness) {
                max_usefullness = curr_usefullness;
                max_index = rule_i;
            }
        }
        if (max_usefullness < 0.0f) {
            break;
        }
        apply_rule(s1, s2, (*rules_ptr).rules[max_index], true, &tokens_similar, &tokens_thrown);
    }

    for (long s1_i = 0; s1_i < (*s1).size; s1_i++) {
        for (long s2_i = 0; s2_i < (*s2).size; s2_i++) {
            if (strcmp((*s1).words[s1_i], (*s2).words[s2_i]) == 0) {
                tokens_shared += 1;
                remove_element(s1, s1_i);
                s1_i -= 1;
                remove_element(s2, s2_i);
                s2_i -= 1;
                break;
            }
        }
    }

    float jaccard_common = tokens_similar + tokens_shared;
    float jaccard_total = jaccard_common + (*s1).size + (*s2).size + tokens_thrown;

    return jaccard_common / jaccard_total;
}

float do_cmp(const char *string1, const char *string2, float exactness, Oid tRoid, const char *tRcol_abbr,
             const char *tRcol_full) {
    char *tR;

    char query[4096];

    RuleSequence rs;

    SplitStr s1;
    SplitStr s2;
    float pkduck;

    tR = get_table_name(tRoid);

    sprintf(query, "SELECT %s, %s FROM %s;", tRcol_abbr, tRcol_full, tR);
    if (SPI_execute(query, true, 0) < 0 || SPI_tuptable == NULL) {
        elog(ERROR, "Could not SELECT rows from table '%s' (OID %d).", tR, tRoid);
    }
    rs = (RuleSequence) {
            0,
            palloc(sizeof(*rs.rules) * SPI_processed * 2)
    };
    for (uint32 i = 0; i < SPI_processed; i++) {
        char *temp[2];
        SplitStr rule_abbr_seq;
        SplitStr rule_full_seq;

        temp[0] = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 1);
        temp[1] = SPI_getvalue((*SPI_tuptable).vals[i], (*SPI_tuptable).tupdesc, 2);
        if (temp[0] == NULL || temp[1] == NULL) {
            continue;
        }

        rule_abbr_seq = tokenize(temp[0]);
        pg_qsort(rule_abbr_seq.words, rule_abbr_seq.size, sizeof(*rule_abbr_seq.words), cmp_ss);
        rule_full_seq = tokenize(temp[1]);
        pg_qsort(rule_full_seq.words, rule_full_seq.size, sizeof(*rule_full_seq.words), cmp_ss);

        rs.rules[rs.size].a = rule_abbr_seq;
        rs.rules[rs.size].r = rule_full_seq;
        rs.size += 1;

        rs.rules[rs.size].a = rule_full_seq;
        rs.rules[rs.size].r = rule_abbr_seq;
        rs.size += 1;
    }
    if (rs.size != SPI_processed * 2) {
        rs.rules = repalloc(rs.rules, sizeof(*rs.rules) * rs.size);
    }
    SPI_freetuptable(SPI_tuptable);

    s1 = tokenize(string1);
    pg_qsort(s1.words, s1.size, sizeof(*s1.words), cmp_ss);
    s2 = tokenize(string2);
    pg_qsort(s2.words, s2.size, sizeof(*s2.words), cmp_ss);

    pkduck = calc_pkduck(&s1, &s2, &rs);

    elog(INFO, "PKDUCK-57 %s, %s: %lf", string1, string2, pkduck);

    return pkduck;
//    if (pkduck - exactness > 0.0f) {
//        return true;
//    }
//    return false;
}


Datum pkduck(PG_FUNCTION_ARGS) {
    char *string1 = text_to_cstring(PG_GETARG_TEXT_P(0));
    char *string2 = text_to_cstring(PG_GETARG_TEXT_P(1));
    Oid tRoid = PG_GETARG_OID(2);
    char *tRcol_full = text_to_cstring(PG_GETARG_TEXT_P(3));
    char *tRcol_abbr = text_to_cstring(PG_GETARG_TEXT_P(4));
    float exactness = PG_GETARG_FLOAT4(5);

    SPI_connect();
    float result = do_cmp(string1, string2, exactness, tRoid, tRcol_abbr, tRcol_full);
    SPI_finish();

    PG_RETURN_FLOAT8(result);
}

Datum get_tds_exact(PG_FUNCTION_ARGS) {
    PG_RETURN_FLOAT8(get_exactness());
}

Datum set_tds_exact(PG_FUNCTION_ARGS) {
    set_exactness(PG_GETARG_FLOAT8(0));
}