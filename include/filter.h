#ifndef FILTER_H
#define FILTER_H

#include <stddef.h>
#include "ecg_types.h"

int ecg_moving_average(const EcgSignal *input, EcgSignal *output, size_t window_size);

#endif
