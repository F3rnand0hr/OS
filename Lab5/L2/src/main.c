#include <math.h>
#include <stdio.h>
#include <time.h>

#include "../include/functions.h"

static int ProcessMappedFile(const char* path,
                             double (*sum_fn)(const void*, size_t),
                             double* sum_out, double* time_out) {
  MappedFile mapped = MapFile(path);
  if (mapped.addr == NULL) {
    return -1;
  }

  struct timespec start;
  struct timespec end;

  clock_gettime(CLOCK_MONOTONIC, &start);
  *sum_out = sum_fn(mapped.addr, mapped.length);
  clock_gettime(CLOCK_MONOTONIC, &end);

  *time_out = ElapsedSeconds(&start, &end);
  UnmapFile(&mapped);

  return 0;
}

int main(void) {
  double sum_txt = 0.0;
  double sum_bin = 0.0;
  double time_txt = 0.0;
  double time_bin = 0.0;

  printf("Processing Text file...\n");
  if (ProcessMappedFile(TEXT_FILE, SumTextMapped, &sum_txt, &time_txt) != 0) {
    return 1;
  }

  printf("Processing Binary file...\n");
  if (ProcessMappedFile(BIN_FILE, SumBinaryMapped, &sum_bin, &time_bin) != 0) {
    return 1;
  }

  printf("\n--- RESULTS ---\n");
  printf("Sum .txt: %.10f (Time: %.5fs)\n", sum_txt, time_txt);
  printf("Sum .bin: %.10f (Time: %.5fs)\n", sum_bin, time_bin);

  if (fabs(sum_txt - sum_bin) < EPSILON) {
    printf("Verification: OK\n");
  } else {
    printf("Verification: FAIL\n");
  }

  if (time_bin > 0.0) {
    printf("Performance: The binary file is %.2fx faster.\n",
           time_txt / time_bin);
  }

  return 0;
}
