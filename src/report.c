#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "report.h"

static const char *json_bool(int value) {
    return value ? "true" : "false";
}

void ecg_print_summary(const EcgAnalysis *analysis) {
    printf("\n=== ECGStream Report ===\n");
    printf("Input: %s\n", analysis->input_path);
    printf("Samples: %zu\n", analysis->sample_count);
    printf("Duration: %.2f seconds\n", analysis->duration_sec);
    printf("R-peaks detected: %zu\n", analysis->peaks.count);
    printf("Average BPM: %.1f\n", analysis->average_bpm);
    printf("BPM range: %.1f - %.1f\n", analysis->minimum_bpm, analysis->maximum_bpm);
    printf("RR mean: %.1f ms\n", analysis->rr_mean_ms);
    printf("RR SD: %.1f ms\n", analysis->rr_sd_ms);
    printf("Classification: %s\n", analysis->classification);
    printf("Bradycardia flag: %s\n", analysis->bradycardia ? "true" : "false");
    printf("Tachycardia flag: %s\n", analysis->tachycardia ? "true" : "false");
    printf("Irregular rhythm flag: %s\n", analysis->irregular ? "true" : "false");
    printf("Disclaimer: synthetic signal only, not for patient care\n");
    printf("========================\n");
}

int ecg_write_json_report(const char *path, const EcgAnalysis *analysis, char *error, size_t error_size) {
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        snprintf(error, error_size, "cannot open %s: %s", path, strerror(errno));
        return -1;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"tool\": \"ECGStream\",\n");
    fprintf(file, "  \"input\": \"%s\",\n", analysis->input_path);
    fprintf(file, "  \"sample_count\": %zu,\n", analysis->sample_count);
    fprintf(file, "  \"duration_sec\": %.3f,\n", analysis->duration_sec);
    fprintf(file, "  \"sample_rate_hz\": %.1f,\n", analysis->sample_rate_hz);
    fprintf(file, "  \"r_peaks_detected\": %zu,\n", analysis->peaks.count);
    fprintf(file, "  \"heart_rate\": {\n");
    fprintf(file, "    \"average_bpm\": %.2f,\n", analysis->average_bpm);
    fprintf(file, "    \"minimum_bpm\": %.2f,\n", analysis->minimum_bpm);
    fprintf(file, "    \"maximum_bpm\": %.2f\n", analysis->maximum_bpm);
    fprintf(file, "  },\n");
    fprintf(file, "  \"rr_intervals\": {\n");
    fprintf(file, "    \"mean_ms\": %.2f,\n", analysis->rr_mean_ms);
    fprintf(file, "    \"sd_ms\": %.2f\n", analysis->rr_sd_ms);
    fprintf(file, "  },\n");
    fprintf(file, "  \"flags\": {\n");
    fprintf(file, "    \"bradycardia\": %s,\n", json_bool(analysis->bradycardia));
    fprintf(file, "    \"tachycardia\": %s,\n", json_bool(analysis->tachycardia));
    fprintf(file, "    \"irregular\": %s\n", json_bool(analysis->irregular));
    fprintf(file, "  },\n");
    fprintf(file, "  \"classification\": \"%s\",\n", analysis->classification);
    fprintf(file, "  \"disclaimer\": \"synthetic signal only, not for patient care\"\n");
    fprintf(file, "}\n");

    fclose(file);
    return 0;
}
