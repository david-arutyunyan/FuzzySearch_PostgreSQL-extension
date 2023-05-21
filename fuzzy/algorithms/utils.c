#include "utils.h"

Trie* add(Trie* trie, Trie* child, int c)
{
    if ((*trie).capacity == (*trie).size) {
        trie = grow(trie);
    }
    int i = (*trie).size++;
    (*trie).children[i].c = c;
    (*trie).children[i].trie = child;
    qsort((*trie).children, (*trie).size, sizeof((*trie).children[0]), ptr_cmp);

    return trie;
}

int bin_search(Trie* trie, Trie** child, TriePtr** ptr, char* key)
{
    int i = 0;
    bool flag = true;
    *ptr = NULL;
    while (flag && key[i] != '\0') {
        int l = 0;
        int r = (*trie).size - 1;
        flag = false;
        while (l <= r) {
            int m = (l + r) / 2;
            TriePtr* p = &(*trie).children[m];
            if ((*p).c == key[i]) {
                trie = (*p).trie;
                *ptr = p;
                flag = true;
                i++;
                break;
            } else if ((*p).c < key[i]) {
                l = m + 1;
            } else if ((*p).c > key[i]) {
                r = m - 1;
            }
        }
    }
    *child = trie;

    return i;
}

Trie* grow(Trie* trie)
{
    int new_size = (*trie).capacity * 2;
    Trie* resized = repalloc(trie, sizeof(*trie) + sizeof(TriePtr) * new_size);
    (*resized).capacity = new_size;

    return resized;
}

int ptr_cmp(void* a, void* b)
{
    return (*((TriePtr*)a)).c - (*((TriePtr*)b)).c;
}

Trie* trie_create(int size)
{
    Trie* trie = palloc(sizeof(*trie) + sizeof(TriePtr) * size);
    (*trie).capacity = size;
    (*trie).size = 0;
    (*trie).data = NULL;

    return trie;
}

void trie_insert(Trie* trie, char* key, void* data)
{
    Trie* prev_trie;
    TriePtr* parent;
    int depth = bin_search(trie, &prev_trie, &parent, key);
    while (key[depth] != '\0') {
        Trie* sub_trie = trie_create(1);
        Trie* added = add(prev_trie, sub_trie, key[depth]);

        if (parent != NULL) {
            (*parent).trie = added;
            parent = NULL;
        }
        prev_trie = sub_trie;
        depth++;
    }
    (*prev_trie).data = data;
}

int trie_search(Trie* trie, char* key, int s, char*** container_ptr) {
    int container_length = 0;

    if ((*trie).size == 0) {
        char** container = palloc(sizeof(*container));
        *container_ptr = container;
        container[0] = palloc(strlen((*trie).data) + 1);
        strcpy(container[0], (char*)(*trie).data);
        return 1;
    }

    for (int i = s; i < strlen(key); ++i) {
        int l = 0;
        int r = (*trie).size - 1;
        while (l <= r) {
            int m = (l + r) / 2;
            TriePtr* p = &(*trie).children[m];

            if ((*p).c == key[i]) {
                char** temp = NULL;
                int temp_length = trie_search((*p).trie, key, i + 1, &temp);
                if (temp_length != 0) {
                    *container_ptr = *container_ptr == NULL ?
                                     palloc(sizeof(**container_ptr) * temp_length) :
                                     repalloc(*container_ptr, sizeof(**container_ptr) * (container_length + temp_length));
                    for (int j = 0; j < temp_length; ++j) {
                        (*container_ptr)[container_length + j] = palloc(strlen(temp[j]) + 1);
                        strcpy((*container_ptr)[container_length + j], temp[j]);
                        pfree(temp[j]);
                    }
                    pfree(temp);
                    container_length += temp_length;
                }
                break;
            } else if ((*p).c < key[i]) {
                l = m + 1;
            } else if ((*p).c > key[i]) {
                r = m - 1;
            }
        }
    }

    return container_length;
}

SplitStr tokenize(char* string) {
    int input_length = strlen(string);
    int token_count = 0;
    int last_space_index = -1;
    char* token = NULL;
    char** tokens = NULL;

    for (int i = 0; i <= input_length; ++i) {
        if (string[i] == ' ' || string[i] == '\0') {
            if (last_space_index < (int)i - 1) {
                token_count++;
                tokens = realloc(tokens, sizeof(*tokens) * token_count);
                tokens[token_count - 1] = malloc(i - last_space_index);
                strncpy(tokens[token_count - 1], string + last_space_index + 1, i - last_space_index - 1);
                tokens[token_count - 1][i - last_space_index - 1] = '\0';
            }
            last_space_index = i;
            token = NULL;
        } else {
            if (!token) {
                token = (char*)&string[i];
            }
        }
    }

    SplitStr result = {token_count, tokens};

    return result;
}

char* remove_element(SplitStr* str, int e)
{
    char* result;
    int size = (*str).size;
    for (int j = e + 1; j < size; ++j) {
        (*str).words[j - 1] = (*str).words[j];
    }

    result = (*str).words[size - 1];
    (*str).words[size - 1] = NULL;
    (*str).size -= 1;

    return result;
}

char* get_table_name(Oid oid)
{
    char query[INT8_MAX];
    sprintf(query, "SELECT %d::regclass;", oid);
    int result = SPI_execute(query, true, 0);

    return (result < 0) ? NULL : SPI_getvalue((*SPI_tuptable).vals[0], (*SPI_tuptable).tupdesc, 1);
}

int cmp(char* a, char* b)
{
    return strlen(a) == strlen(b) ? strcmp(a, b) : strlen(b) - strlen(a);
}

int cmp_ss(void* a, void* b)
{
    return cmp(*(char**)a, *(char**)b);
}

void to_upper_case(char* str)
{
    while (*str != '\0') {
        *str = toupper(*str);
        str++;
    }
}