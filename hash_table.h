#ifndef S_SEARCH_HASH_TABLE_H
#define S_SEARCH_HASH_TABLE_H

struct Node {
    unsigned int hash;
    char *value;
    Node *next;
    Node *prev;
};

struct HashTable {
    int elements;
    Node *first;
    Node *last;
};

unsigned int crc32b(const char *message);

Node *create_node(char *value);

HashTable *create_table(int len = 10000);

void show(HashTable *table, int max = 100);

char *random_string();

void push(HashTable *table, struct Node *node);

#endif //S_SEARCH_HASH_TABLE_H
