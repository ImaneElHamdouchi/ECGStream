.RECIPEPREFIX := >

CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c11 -Iinclude
LDFLAGS=-lm

SRC=src/main.c src/ecg_parser.c src/ring_buffer.c src/filter.c src/r_peak_detector.c src/report.c

all: dirs ecgstream samples

dirs:
>mkdir -p samples out

ecgstream: $(SRC)
>$(CC) $(CFLAGS) -o ecgstream $(SRC) $(LDFLAGS)

tools/generate_samples: tools/generate_samples.c
>$(CC) $(CFLAGS) -o tools/generate_samples tools/generate_samples.c $(LDFLAGS)

samples: tools/generate_samples
>./tools/generate_samples

test: all
>./tests/test_ecgstream.sh

demo: all
>./ecgstream analyze samples/normal.csv --json out/normal_report.json
>./ecgstream analyze samples/tachycardia.csv --json out/tachycardia_report.json
>./ecgstream analyze samples/bradycardia.csv --json out/bradycardia_report.json
>./ecgstream analyze samples/irregular.csv --json out/irregular_report.json

clean:
>rm -f ecgstream tools/generate_samples
>rm -rf out
>rm -f samples/*.csv

.PHONY: all dirs samples test demo clean
