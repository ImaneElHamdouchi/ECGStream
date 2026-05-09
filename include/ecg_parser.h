#ifndef ECG_PARSER_H
#define ECG_PARSER_H

#include "ecg_types.h"

int ecg_load_csv(const char *path, EcgSignal *signal, char *error, size_t error_size);
void ecg_signal_free(EcgSignal *signal);

#endif
