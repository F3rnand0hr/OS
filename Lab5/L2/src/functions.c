#include "../include/functions.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

MappedFile MapFile(const char* path) {
  MappedFile mapped = {NULL, 0, -1};

  mapped.fd = open(path, O_RDONLY);
  if (mapped.fd < 0) {
    perror("open");
    return mapped;
  }

  struct stat st;
  if (fstat(mapped.fd, &st) != 0) {
    perror("fstat");
    close(mapped.fd);
    mapped.fd = -1;
    return mapped;
  }

  mapped.length = (size_t)st.st_size;
  mapped.addr =
      mmap(NULL, mapped.length, PROT_READ, MAP_PRIVATE, mapped.fd, 0);
  if (mapped.addr == MAP_FAILED) {
    perror("mmap");
    close(mapped.fd);
    mapped.addr = NULL;
    mapped.fd = -1;
    mapped.length = 0;
  }

  return mapped;
}

void UnmapFile(MappedFile* mapped) {
  if (mapped == NULL) {
    return;
  }

  if (mapped->addr != NULL && mapped->addr != MAP_FAILED) {
    munmap(mapped->addr, mapped->length);
  }
  if (mapped->fd >= 0) {
    close(mapped->fd);
  }

  mapped->addr = NULL;
  mapped->length = 0;
  mapped->fd = -1;
}

double SumTextMapped(const void* data, size_t length) {
  const char* cursor = (const char*)data;
  const char* end = cursor + length;
  double sum = 0.0;

  for (int i = 0; i < TOTAL_NUMBERS; i++) {
    char* next = NULL;
    double value = strtod(cursor, &next);
    sum += value;
    cursor = next;
    if (cursor < end && *cursor == '\n') {
      cursor++;
    }
  }

  return sum;
}

double SumBinaryMapped(const void* data, size_t length) {
  const double* values = (const double*)data;
  size_t count = length / sizeof(double);
  double sum = 0.0;

  for (size_t i = 0; i < count; i++) {
    sum += values[i];
  }

  return sum;
}

double ElapsedSeconds(const struct timespec* start,
                      const struct timespec* end) {
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_nsec - start->tv_nsec) / 1e9;
}
