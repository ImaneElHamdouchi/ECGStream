#include <stdlib.h>
#include "filter.h"
#include "ring_buffer.h"

int ecg_moving_average(const EcgSignal *input, EcgSignal *output, size_t window_size) {
    if (input == NULL || output == NULL || input->count == 0 || window_size == 0) {
        return -1;
    }

    output->items = calloc(input->count, sizeof(EcgSample));
    if (output->items == NULL) {
        return -1;
    }

    output->count = input->count;

    RingBuffer buffer;
    if (ring_buffer_init(&buffer, window_size) != 0) {
        free(output->items);
        output->items = NULL;
        output->count = 0;
        return -1;
    }

    for (size_t i = 0; i < input->count; i++) {
        output->items[i].time_ms = input->items[i].time_ms;
        output->items[i].mv = ring_buffer_push_average(&buffer, input->items[i].mv);
    }

    ring_buffer_free(&buffer);

    return 0;
}
