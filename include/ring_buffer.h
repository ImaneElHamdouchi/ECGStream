#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>

typedef struct {
    double *values;
    size_t capacity;
    size_t count;
    size_t index;
    double sum;
} RingBuffer;

int ring_buffer_init(RingBuffer *buffer, size_t capacity);
void ring_buffer_free(RingBuffer *buffer);
double ring_buffer_push_average(RingBuffer *buffer, double value);

#endif
