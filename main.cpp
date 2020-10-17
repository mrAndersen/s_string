#include "cstring"
#include "random"
#include "pthread.h"
#include "unistd.h"

#define DIRECTION_TOP 0
#define DIRECTION_BOTTOM 1

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

struct Node {
    unsigned int hash = 0;
    char *value = {};
    Node *next = nullptr;
    Node *prev = nullptr;
};

struct Table {
    int elements = 0;
    Node *first = nullptr;
    Node *last = nullptr;
};

struct ControlParameters {
    bool working = false;
};

struct WorkerParameters {
    ControlParameters *controlParameters = {};
    char *searchValue = nullptr;
    int direction = DIRECTION_TOP;
    int threadIndex = 1;
    double spent = 0;
    Table *table = {};
};

struct ScanResult {
    unsigned int scanned = 0;
    int position = 0;
};

Node *create(char *value) {
    Node *n;
    n = (Node *) malloc(sizeof(Node));

    n->value = value;
    n->hash = crc32b(value);

    return n;
}

void push(Table *table, Node *node) {
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

ScanResult search(WorkerParameters *wp) {
    int position = 0;
    unsigned int scanned = 0;

    auto hash = crc32b(wp->searchValue);
    auto cursor = wp->direction == DIRECTION_TOP ? wp->table->first : wp->table->last;
    bool searching = true;

    while (searching) {
        searching = wp->direction == DIRECTION_TOP ? cursor->next != nullptr : cursor->prev != nullptr;

        if (cursor->hash == hash) {
            return {scanned, position};
        }

        position++;
        scanned++;
        cursor = wp->direction == DIRECTION_TOP ? cursor->next : cursor->prev;

        if (!wp->controlParameters->working) {
            printf("thread#%d aborted, another one won\n", wp->threadIndex);
            return {scanned, -1};
        }
    }

    return {scanned, -1};
}

double get_time_ms() {
    return (double) clock() * 1000.f / CLOCKS_PER_SEC;
}

Table *create_hash_table(const char *path) {
    Table *table;
    table = (Table *) malloc(sizeof(Table));

    double start = get_time_ms();
    FILE *fp = fopen(path, "r");

    if (fp == nullptr) {
        printf("Unable to open file %s, check paths\n", path);
        exit(-1);
    }

    int i = 1;
    int buf_len = 255;

    char buf[buf_len];
    char *clean;

    while (fgets(buf, buf_len, fp)) {
        clean = strtok(buf, "\n");

        if (clean == nullptr) {
            continue;
        }

        push(table, create(clean));
        i++;

        if (i % 1000 == 0) {
            printf("\rloading: %d - %s", i, clean);
        }

        memset(&buf, 0, buf_len);
    }

    printf("\nloaded: %d", i);
    fclose(fp);

    printf("\ntable created, size = %d \t\t %.3fms\n", table->elements, (get_time_ms() - start));
    return table;
}

pthread_mutex_t control_m;
pthread_mutex_t working_m;

void *search_worker(void *wp) {
    auto parameters = (WorkerParameters *) (wp);

    while (true) {
        if (!parameters->controlParameters->working) {
            usleep(100);
        } else {
            auto start = get_time_ms();
            auto r = search(parameters);

            pthread_mutex_lock(&working_m);
            parameters->controlParameters->working = false;
            pthread_mutex_unlock(&working_m);

            parameters->spent = (get_time_ms() - start);

            if (r.position == -1) {
                printf(
                        "thread#%d (direction %s) found \033[1;31mnothing\033[0m, scanned %d, time %.2fms\n",
                        parameters->threadIndex,
                        (parameters->direction == DIRECTION_TOP ? "top" : "bottom"),
                        r.scanned,
                        parameters->spent
                );
            } else {
                printf(
                        "thread#%d (direction %s) \033[1;32mfound \033[0m at position %d, scanned %d, time %.2fms\n",
                        parameters->threadIndex,
                        (parameters->direction == DIRECTION_TOP ? "top" : "bottom"),
                        r.position,
                        r.scanned,
                        parameters->spent
                );
            }
        }
    }
}

void show(Table *table, int max = 100) {
    auto cursor = table->first;
    int i = 0;

    while (cursor->next != nullptr && i < max) {
        printf("%s\n", cursor->value);
        cursor = cursor->next;

        i++;
    }
}

int main() {
    pthread_attr_t attr;
    pthread_mutex_init(&control_m, nullptr);
    pthread_mutex_init(&working_m, nullptr);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    auto table = create_hash_table("../passwords.txt");
    int threadsCount = 2;

    ControlParameters control;

    WorkerParameters wp1 = {};
    wp1.table = table;
    wp1.threadIndex = 1;
    wp1.direction = DIRECTION_TOP;
    wp1.controlParameters = &control;

    WorkerParameters wp2 = {};
    wp2.table = table;
    wp2.threadIndex = 2;
    wp2.direction = DIRECTION_BOTTOM;
    wp2.controlParameters = &control;

    pthread_t threads[threadsCount];

    pthread_create(&threads[0], &attr, search_worker, (void *) (&wp1));
    pthread_create(&threads[1], &attr, search_worker, (void *) (&wp2));

    printf("threads created\n");

    int len = 255;
    char buf[len];
    memset(&buf, 0, len);

    while (true) {
        while (scanf("%s", buf)) {
            wp1.searchValue = buf;
            wp2.searchValue = buf;
            control.working = true;
        }

        usleep(2000);
    }

    pthread_exit(nullptr);
}