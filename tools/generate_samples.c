#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

#define PI 3.14159265358979323846

static double triangle(double x, double center, double width, double height) {
    double distance = fabs(x - center);

    if (distance > width) {
        return 0.0;
    }

    return height * (1.0 - distance / width);
}

static double ecg_value(double time_ms, const double *beats, size_t beat_count) {
    double seconds = time_ms / 1000.0;
    double value = 0.02 * sin(2.0 * PI * 0.33 * seconds);
    value += 0.006 * sin(2.0 * PI * 17.0 * seconds);

    for (size_t i = 0; i < beat_count; i++) {
        double dt = time_ms - beats[i];

        value += triangle(dt, -35.0, 18.0, -0.12);
        value += triangle(dt, 0.0, 42.0, 1.20);
        value += triangle(dt, 55.0, 22.0, -0.18);
        value += triangle(dt, 220.0, 95.0, 0.18);
    }

    return value;
}

static size_t build_regular_beats(double *beats, size_t max_beats, double duration_ms, double rr_ms) {
    size_t count = 0;

    for (double t = 500.0; t < duration_ms && count < max_beats; t += rr_ms) {
        beats[count++] = t;
    }

    return count;
}

static size_t build_irregular_beats(double *beats, size_t max_beats, double duration_ms) {
    const double pattern[] = {700.0, 980.0, 600.0, 1220.0, 760.0, 1510.0, 640.0, 900.0};
    const size_t pattern_count = sizeof(pattern) / sizeof(pattern[0]);

    size_t count = 0;
    double t = 500.0;
    size_t index = 0;

    while (t < duration_ms && count < max_beats) {
        beats[count++] = t;
        t += pattern[index % pattern_count];
        index++;
    }

    return count;
}

static int write_sample(const char *path, const char *kind) {
    const double sample_rate = 250.0;
    const double duration_sec = 12.0;
    const double duration_ms = duration_sec * 1000.0;
    const size_t total_samples = (size_t)(duration_sec * sample_rate);

    double beats[128];
    size_t beat_count = 0;

    if (kind[0] == 'n') {
        beat_count = build_regular_beats(beats, 128, duration_ms, 1000.0);
    } else if (kind[0] == 't') {
        beat_count = build_regular_beats(beats, 128, duration_ms, 500.0);
    } else if (kind[0] == 'b') {
        beat_count = build_regular_beats(beats, 128, duration_ms, 1500.0);
    } else {
        beat_count = build_irregular_beats(beats, 128, duration_ms);
    }

    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror(path);
        return -1;
    }

    fprintf(file, "time_ms,mv\n");

    for (size_t i = 0; i < total_samples; i++) {
        double time_ms = (double)i * 1000.0 / sample_rate;
        double value = ecg_value(time_ms, beats, beat_count);
        fprintf(file, "%.3f,%.6f\n", time_ms, value);
    }

    fclose(file);
    return 0;
}

int main(void) {
    mkdir("samples", 0755);

    if (write_sample("samples/normal.csv", "normal") != 0) return 1;
    if (write_sample("samples/tachycardia.csv", "tachycardia") != 0) return 1;
    if (write_sample("samples/bradycardia.csv", "bradycardia") != 0) return 1;
    if (write_sample("samples/irregular.csv", "irregular") != 0) return 1;

    printf("Generated ECG samples in samples/\n");
    return 0;
}
