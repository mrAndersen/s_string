#include <cassert>
#include "cstring"
#include "random"
#include "pthread.h"
#include "unistd.h"
#include "sodium.h"
#include "hash_table.h"

double getTimeMs() {
    return (double) clock() * 1000.f / CLOCKS_PER_SEC;
}


std::vector<char *> *create_vector(int len = 10000) {
    auto ret = new std::vector<char *>();

    for (int i = 0; i < len; ++i) {
        ret->push_back(random_string());
    }

    return ret;
}

void bench_vector(int len = 10000) {
    auto start = getTimeMs();

    auto vec = create_vector(len);
    std::vector<char *>::iterator it = {};

    for (it = vec->begin(); it != vec->end(); ++it) {
        //do things
    }

    assert(it == vec->end());
    auto end = getTimeMs();

    printf("vec in %.2fms\n", end - start);
}

void bench_ht(int len = 10000) {
    auto start = getTimeMs();

    auto ht = create_table(len);
    auto cursor = ht->first;

    while (cursor->next != nullptr) {
        //do things
        cursor = cursor->next;
    }

    assert(cursor == ht->last);
    auto end = getTimeMs();

    printf("ht in %.2fms\n", end - start);
}

int main() {
    if (sodium_init() < 0) {
        printf("Sodium error");
        return 1;
    }

    int l = 1000000;

    bench_ht(l);
    bench_vector(l);


    return 0;
}