#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include "ecg_types.h"

void ecg_print_summary(const EcgAnalysis *analysis);
int ecg_write_json_report(const char *path, const EcgAnalysis *analysis, char *error, size_t error_size);

#endif
