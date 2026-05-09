#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "r_peak_detector.h"

static int add_peak(PeakList *peaks, size_t *capacity, double time_ms) {
    if (peaks->count == *capacity) {
        size_t new_capacity = (*capacity == 0) ? 32 : (*capacity * 2);
        double *new_items = realloc(peaks->time_ms, new_capacity * sizeof(double));

        if (new_items == NULL) {
            return -1;
        }

        peaks->time_ms = new_items;
        *capacity = new_capacity;
    }

    peaks->time_ms[peaks->count++] = time_ms;
    return 0;
}

static double automatic_threshold(const EcgSignal *signal) {
    double min_value = signal->items[0].mv;
    double max_value = signal->items[0].mv;
    double sum = 0.0;

    for (size_t i = 0; i < signal->count; i++) {
        double value = signal->items[i].mv;

        if (value < min_value) {
            min_value = value;
        }

        if (value > max_value) {
            max_value = value;
        }

        sum += value;
    }

    double mean = sum / (double)signal->count;
    return mean + 0.45 * (max_value - min_value);
}

static double compute_sd(const double *values, size_t count, double mean) {
    if (count < 2) {
        return 0.0;
    }

    double sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        double delta = values[i] - mean;
        sum += delta * delta;
    }

    return sqrt(sum / (double)(count - 1));
}

int ecg_analyze_signal(
    const char *input_path,
    const EcgSignal *signal,
    double sample_rate_hz,
    double threshold_mv,
    EcgAnalysis *analysis,
    char *error,
    size_t error_size
) {
    if (signal == NULL || analysis == NULL || signal->count < 3) {
        snprintf(error, error_size, "invalid signal");
        return -1;
    }

    memset(analysis, 0, sizeof(*analysis));
    snprintf(analysis->input_path, sizeof(analysis->input_path), "%s", input_path);
    analysis->sample_count = signal->count;
    analysis->sample_rate_hz = sample_rate_hz;
    analysis->duration_sec = (signal->items[signal->count - 1].time_ms - signal->items[0].time_ms) / 1000.0;

    double threshold = threshold_mv;
    if (threshold <= 0.0) {
        threshold = automatic_threshold(signal);
    }

    size_t capacity = 0;
    double last_peak_ms = -1000000.0;
    const double refractory_ms = 250.0;

    for (size_t i = 1; i + 1 < signal->count; i++) {
        double prev = signal->items[i - 1].mv;
        double curr = signal->items[i].mv;
        double next = signal->items[i + 1].mv;
        double time_ms = signal->items[i].time_ms;

        int is_local_maximum = curr > prev && curr >= next;
        int above_threshold = curr >= threshold;
        int outside_refractory = (time_ms - last_peak_ms) >= refractory_ms;

        if (is_local_maximum && above_threshold && outside_refractory) {
            if (add_peak(&analysis->peaks, &capacity, time_ms) != 0) {
                snprintf(error, error_size, "memory allocation failed");
                ecg_analysis_free(analysis);
                return -1;
            }

            last_peak_ms = time_ms;
        }
    }

    if (analysis->peaks.count < 2) {
        snprintf(error, error_size, "not enough R-peaks detected");
        ecg_analysis_free(analysis);
        return -1;
    }

    size_t rr_count = analysis->peaks.count - 1;
    double *rr = calloc(rr_count, sizeof(double));

    if (rr == NULL) {
        snprintf(error, error_size, "memory allocation failed");
        ecg_analysis_free(analysis);
        return -1;
    }

    double rr_sum = 0.0;
    double min_rr = 1000000.0;
    double max_rr = 0.0;

    for (size_t i = 0; i < rr_count; i++) {
        rr[i] = analysis->peaks.time_ms[i + 1] - analysis->peaks.time_ms[i];
        rr_sum += rr[i];

        if (rr[i] < min_rr) {
            min_rr = rr[i];
        }

        if (rr[i] > max_rr) {
            max_rr = rr[i];
        }
    }

    analysis->rr_mean_ms = rr_sum / (double)rr_count;
    analysis->rr_sd_ms = compute_sd(rr, rr_count, analysis->rr_mean_ms);

    analysis->average_bpm = 60000.0 / analysis->rr_mean_ms;
    analysis->minimum_bpm = 60000.0 / max_rr;
    analysis->maximum_bpm = 60000.0 / min_rr;

    analysis->bradycardia = analysis->average_bpm < 60.0;
    analysis->tachycardia = analysis->average_bpm > 100.0;
    analysis->irregular = analysis->rr_sd_ms > 120.0 || (analysis->rr_sd_ms / analysis->rr_mean_ms) > 0.12;

    if (analysis->irregular) {
        analysis->classification = "irregular";
    } else if (analysis->tachycardia) {
        analysis->classification = "tachycardia";
    } else if (analysis->bradycardia) {
        analysis->classification = "bradycardia";
    } else {
        analysis->classification = "normal";
    }

    free(rr);
    return 0;
}

void ecg_analysis_free(EcgAnalysis *analysis) {
    if (analysis == NULL) {
        return;
    }

    free(analysis->peaks.time_ms);
    analysis->peaks.time_ms = NULL;
    analysis->peaks.count = 0;
}
