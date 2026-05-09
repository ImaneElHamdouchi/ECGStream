#ifndef R_PEAK_DETECTOR_H
#define R_PEAK_DETECTOR_H

#include <stddef.h>
#include "ecg_types.h"

int ecg_analyze_signal(
    const char *input_path,
    const EcgSignal *signal,
    double sample_rate_hz,
    double threshold_mv,
    EcgAnalysis *analysis,
    char *error,
    size_t error_size
);

void ecg_analysis_free(EcgAnalysis *analysis);

#endif
