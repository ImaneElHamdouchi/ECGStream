#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ecg_parser.h"
#include "filter.h"
#include "r_peak_detector.h"
#include "report.h"

static void print_usage(const char *program) {
    printf("Usage:\n");
    printf("  %s analyze <csv_file> [options]\n", program);
    printf("\nOptions:\n");
    printf("  --json path          Write JSON report\n");
    printf("  --sample-rate hz     Sampling rate, default 250\n");
    printf("  --window n           Moving average window, default 5\n");
    printf("  --threshold mv       Manual R-peak threshold\n");
    printf("  -h, --help           Show help\n");
}

static int parse_double(const char *value, double *out) {
    char *end = NULL;
    double parsed = strtod(value, &end);

    if (end == value || *end != '\0') {
        return -1;
    }

    *out = parsed;
    return 0;
}

static int parse_size(const char *value, size_t *out) {
    char *end = NULL;
    unsigned long parsed = strtoul(value, &end, 10);

    if (end == value || *end != '\0' || parsed == 0) {
        return -1;
    }

    *out = (size_t)parsed;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "analyze") != 0) {
        print_usage(argv[0]);
        return 1;
    }

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *input_path = argv[2];
    const char *json_path = NULL;
    double sample_rate_hz = 250.0;
    double threshold_mv = -1.0;
    size_t window_size = 5;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0 && i + 1 < argc) {
            json_path = argv[++i];
        } else if (strcmp(argv[i], "--sample-rate") == 0 && i + 1 < argc) {
            if (parse_double(argv[++i], &sample_rate_hz) != 0 || sample_rate_hz <= 0.0) {
                fprintf(stderr, "invalid sample rate\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--window") == 0 && i + 1 < argc) {
            if (parse_size(argv[++i], &window_size) != 0) {
                fprintf(stderr, "invalid window size\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            if (parse_double(argv[++i], &threshold_mv) != 0) {
                fprintf(stderr, "invalid threshold\n");
                return 1;
            }
        } else {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    char error[256];
    EcgSignal raw = {0};
    EcgSignal filtered = {0};
    EcgAnalysis analysis;

    if (ecg_load_csv(input_path, &raw, error, sizeof(error)) != 0) {
        fprintf(stderr, "error: %s\n", error);
        return 1;
    }

    if (ecg_moving_average(&raw, &filtered, window_size) != 0) {
        fprintf(stderr, "error: filtering failed\n");
        ecg_signal_free(&raw);
        return 1;
    }

    if (ecg_analyze_signal(input_path, &filtered, sample_rate_hz, threshold_mv, &analysis, error, sizeof(error)) != 0) {
        fprintf(stderr, "error: %s\n", error);
        ecg_signal_free(&raw);
        ecg_signal_free(&filtered);
        return 1;
    }

    ecg_print_summary(&analysis);

    if (json_path != NULL) {
        if (ecg_write_json_report(json_path, &analysis, error, sizeof(error)) != 0) {
            fprintf(stderr, "error: %s\n", error);
            ecg_analysis_free(&analysis);
            ecg_signal_free(&raw);
            ecg_signal_free(&filtered);
            return 1;
        }
    }

    ecg_analysis_free(&analysis);
    ecg_signal_free(&raw);
    ecg_signal_free(&filtered);

    return 0;
}
