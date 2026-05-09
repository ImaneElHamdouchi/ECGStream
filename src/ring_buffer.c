#include <stdlib.h>
#include "ring_buffer.h"

int ring_buffer_init(RingBuffer *buffer, size_t capacity) {
    if (capacity == 0) {
        return -1;
    }

    buffer->values = calloc(capacity, sizeof(double));
    if (buffer->values == NULL) {
        return -1;
    }

    buffer->capacity = capacity;
    buffer->count = 0;
    buffer->index = 0;
    buffer->sum = 0.0;

    return 0;
}

void ring_buffer_free(RingBuffer *buffer) {
    if (buffer == NULL) {
        return;
    }

    free(buffer->values);
    buffer->values = NULL;
    buffer->capacity = 0;
    buffer->count = 0;
    buffer->index = 0;
    buffer->sum = 0.0;
}

double ring_buffer_push_average(RingBuffer *buffer, double value) {
    if (buffer->count < buffer->capacity) {
        buffer->values[buffer->index] = value;
        buffer->sum += value;
        buffer->count++;
    } else {
        buffer->sum -= buffer->values[buffer->index];
        buffer->values[buffer->index] = value;
        buffer->sum += value;
    }

    buffer->index = (buffer->index + 1) % buffer->capacity;

    return buffer->sum / (double)buffer->count;
}
