#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include "hash_table.h"
#include "sodium.h"


unsigned int crc32b(const char *message) {
    int i, j;
    unsigned int byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while (message[i] != 0) {
        byte = message[i];            // Get next byte.
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
}

HashTable *create_table(int len) {
    HashTable *ht;
    ht = (HashTable *) malloc(sizeof(HashTable));

    for (int i = 0; i < len; ++i) {
        push(ht, create_node(random_string()));
    }

    return ht;
}


char *random_string() {
    char *result;
    result = (char *) malloc(16);

    for (int i = 0; i < 16; ++i) {
        result[i] = 122 - randombytes_uniform(26);
    }

    return result;
}


Node *create_node(char *value) {
    Node *n;
    n = (Node *) malloc(sizeof(Node));

    n->value = value;
    n->hash = crc32b(value);

    return n;
}

void push(HashTable *table, Node *node) {
    table->elements++;

    if (table->last == nullptr) {
        table->last = node;
        table->first = node;
    } else {
        auto oldLast = table->last;

        table->last = node;
        table->last->prev = oldLast;
        table->last->prev->next = node;
        table->last->next = nullptr;
    }
}

void show(HashTable *table, int max) {
    auto cursor = table->first;
    int i = 0;

    while (cursor->next != nullptr && i < max) {
        printf("%s\n", cursor->value);
        cursor = cursor->next;

        i++;
    }
}

