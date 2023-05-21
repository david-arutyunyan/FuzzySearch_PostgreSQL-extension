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
#include <unistd.h>
#include <stddef.h>

typedef struct {
    int size;
    char*** pairs;
    int read;
} StringPairRows;

typedef struct {
    int size;
    char** words;
} SplitStr;

typedef struct {
    struct Trie* trie;
    int c;
} TriePtr;

typedef struct {
    void* data;
    int size, capacity;
    TriePtr children[];
} Trie;

Trie* grow(Trie* trie);

int ptr_cmp(void* a, void* b);

Trie* trie_create(int size);

void trie_insert(Trie* trie, char* key, void* data);

//int trie_search(Trie* trie, char* key, int s, char*** container);
int trie_search(Trie* trie, char* key, int key_start_pos, char*** container);

SplitStr tokenize(char* string);

char* remove_element(SplitStr* str, int e);

char* get_table_name(Oid oid);

int cmp(char* a, char* b);

int cmp_ss(void* a, void* b);

void to_upper_case(char* str);

#endif