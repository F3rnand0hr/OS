#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <stddef.h>
#include <sys/mman.h>
#include <time.h>

#define TOTAL_NUMBERS 1000000
#define EPSILON 1.0

#define TEXT_FILE "integer.txt"
#define BIN_FILE "bindata.bin"

typedef struct {
  void* addr;
  size_t length;
  int fd;
} MappedFile;

MappedFile MapFile(const char* path);
void UnmapFile(MappedFile* mapped);

double SumTextMapped(const void* data, size_t length);
double SumBinaryMapped(const void* data, size_t length);

double ElapsedSeconds(const struct timespec* start,
                      const struct timespec* end);

#endif  // INCLUDE_FUNCTIONS_H_
