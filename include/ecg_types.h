#ifndef ECG_TYPES_H
#define ECG_TYPES_H

#include <stddef.h>

typedef struct {
    double time_ms;
    double mv;
} EcgSample;

typedef struct {
    EcgSample *items;
    size_t count;
} EcgSignal;

typedef struct {
    double *time_ms;
    size_t count;
} PeakList;

typedef struct {
    char input_path[256];
    size_t sample_count;
    double duration_sec;
    double sample_rate_hz;

    PeakList peaks;

    double average_bpm;
    double minimum_bpm;
    double maximum_bpm;
    double rr_mean_ms;
    double rr_sd_ms;

    int bradycardia;
    int tachycardia;
    int irregular;

    const char *classification;
} EcgAnalysis;

#endif
