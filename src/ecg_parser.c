#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ecg_parser.h"

int ecg_load_csv(const char *path, EcgSignal *signal, char *error, size_t error_size) {
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        snprintf(error, error_size, "cannot open %s: %s", path, strerror(errno));
        return -1;
    }

    size_t capacity = 1024;
    size_t count = 0;
    EcgSample *items = malloc(capacity * sizeof(EcgSample));

    if (items == NULL) {
        fclose(file);
        snprintf(error, error_size, "memory allocation failed");
        return -1;
    }

    char line[512];

    while (fgets(line, sizeof(line), file) != NULL) {
        char *cursor = line;
        char *end = NULL;

        double time_ms = strtod(cursor, &end);
        if (end == cursor) {
            continue;
        }

        cursor = end;
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }

        if (*cursor != ',') {
            continue;
        }

        cursor++;

        double mv = strtod(cursor, &end);
        if (end == cursor) {
            continue;
        }

        if (count == capacity) {
            capacity *= 2;
            EcgSample *new_items = realloc(items, capacity * sizeof(EcgSample));

            if (new_items == NULL) {
                free(items);
                fclose(file);
                snprintf(error, error_size, "memory allocation failed");
                return -1;
            }

            items = new_items;
        }

        items[count].time_ms = time_ms;
        items[count].mv = mv;
        count++;
    }

    fclose(file);

    if (count < 3) {
        free(items);
        snprintf(error, error_size, "not enough ECG samples");
        return -1;
    }

    signal->items = items;
    signal->count = count;

    return 0;
}

void ecg_signal_free(EcgSignal *signal) {
    if (signal == NULL) {
        return;
    }

    free(signal->items);
    signal->items = NULL;
    signal->count = 0;
}
