#ifndef UTILS_H
#define UTILS_H

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "utils/elog.h"
#include "utils/builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "utils/timestamp.h"
#include <stdint.h>
#include <access/htup_details.h>
#include "executor/spi.h"
#include "funcapi.h"
#include "ctype.h"

typedef struct {
    int size;
    int read;
    char*** pairs;
} StringPairRows;

typedef struct {
    int size;
    char** words;
} SplitStr;

typedef struct {
    int c;
    struct Trie* trie;
} TriePtr;

typedef struct {
    int size, capacity;
    void* data;
    TriePtr children[];
} Trie;

Trie* trie_create(int size);

void trie_insert(Trie* trie, char* key, void* data);

int trie_search(Trie* trie, char* key, int s, char*** container);

SplitStr tokenize(char* string);

char* remove_element(SplitStr* str, int e);

char* get_table_name(Oid oid);

int cmp(char* a, char* b);

int cmp_ss(void* a, void* b);

#endif