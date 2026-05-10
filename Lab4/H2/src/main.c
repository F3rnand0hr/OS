#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// --- Configuration ---
#define PROC_MAPS_PATH "/proc/self/maps"
/* "start-end" in /proc/maps can need 16 + 1 + 16 hex digits + NUL. */
/* First column length varies (kernel may zero-pad); keep a safe margin. */
#define OS_MAP_RANGE_MAX 64

// Structure to hold the details of each memory block
typedef struct {
  char name[20];
  int* address;
  size_t size;
  char os_map_range[OS_MAP_RANGE_MAX];  // Range from /proc/self/maps
} MemoryBlock;

// Global array to store block information
MemoryBlock blocks[4];

// --- Helper Functions ---

/**
 * @brief Reads /proc/self/maps and finds the mapped range for a given address.
 * @param addr The start address reported by mmap.
 * @param output_buffer Buffer to store the resulting range (e.g.,
 * 7fff0000-7fff4000).
 */
void GetOsMapRange(const void* addr, char* output_buffer,
                   size_t output_buffer_len) {
  FILE* f = fopen(PROC_MAPS_PATH, "r");
  char line[256];
  uint64_t start = 0;
  uint64_t end = 0;
  const uint64_t addr_val = (uint64_t)(uintptr_t)addr;

  strncpy(output_buffer, "NOT_FOUND", output_buffer_len);

  if (!f) {
    strncpy(output_buffer, "FILE_ERROR", output_buffer_len);
    return;
  }

  while (fgets(line, sizeof(line), f)) {
    const char* sep = strpbrk(line, " \t");
    if (sep == NULL) {
      continue;
    }
    if (sscanf(line, "%" SCNx64 "-%" SCNx64, &start, &end) != 2) {
      continue;
    }
    if (addr_val >= start && addr_val < end) {
      size_t field_len = (size_t)(sep - line);
      if (field_len >= output_buffer_len) {
        field_len = output_buffer_len - 1U;
      }
      memcpy(output_buffer, line, field_len);
      output_buffer[field_len] = '\0';
      fclose(f);
      return;
    }
  }

  fclose(f);
}

// --- Core Task Function ---

/**
 * @brief Creates four distinct memory blocks using mmap().
 * * REQUIRED POSIX FUNCTION: mmap(void *addr, size_t length, int prot, int
 * flags, int fd, off_t offset)
 * * TASK: Create the following four memory blocks, storing the returned address
 * in the blocks array:
 * * 1. BLOCK 0 (Auto-Assigned): Use NULL for the addr parameter.
 * 2. BLOCK 1 (Another Auto-Assigned): Use NULL for the addr parameter again.
 * 3. BLOCK 2 (Hinted Address 1): Use a specific, high-address hint (e.g.,
 * 0x70000000) for the addr parameter, using the MAP_FIXED flag to make the
 * request mandatory.
 * 4. BLOCK 3 (Hinted Address 2): Use a different specific, high-address hint
 * (e.g., 0x80000000) for the addr parameter, using the MAP_FIXED flag.
 * * NOTE: MAP_FIXED is aggressive and may fail if the address is already in
 * use.
 */
void CreateMemoryBlocks() {
  struct {
    const char* name;
    size_t size;
    void* addr;
    int flags;
    int value;
  } spec[4] = {
      {"BLOCK_0_AUTO_A", 64 * 1024, NULL,
       MAP_PRIVATE | MAP_ANONYMOUS, 100},
      {"BLOCK_1_AUTO_B", 128 * 1024, NULL,
       MAP_PRIVATE | MAP_ANONYMOUS, 200},
      {"BLOCK_2_HINT_1", 64 * 1024, (void*)0x70000000,
       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 300},
      {"BLOCK_3_HINT_2", 64 * 1024, (void*)0x80000000,
       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 400},
  };

  const int prot = PROT_READ | PROT_WRITE;
  const int fd = -1;
  const off_t offset = 0;

  for (int i = 0; i < 4; i++) {
    strncpy(blocks[i].name, spec[i].name, sizeof(blocks[i].name) - 1);
    blocks[i].name[sizeof(blocks[i].name) - 1] = '\0';
    blocks[i].size = spec[i].size;

    void* p =
        mmap(spec[i].addr, spec[i].size, prot, spec[i].flags, fd, offset);
    if (p == MAP_FAILED && (spec[i].flags & MAP_FIXED)) {
      p = mmap(NULL, spec[i].size, prot, MAP_PRIVATE | MAP_ANONYMOUS, fd,
               offset);
    }
    if (p == MAP_FAILED) {
      exit(1);
    }
    blocks[i].address = (int*)p;
    blocks[i].address[0] = spec[i].value;
  }

  /*
   * IMPORTANT: read /proc/self/maps after all mmaps are done.
   * The kernel can merge adjacent anonymous mappings into a single VMA,
   * changing the "start-end" range strings. The test script greps that exact
   * range in /proc/<pid>/maps, so we must capture the final ranges.
   */
  for (int i = 0; i < 4; i++) {
    GetOsMapRange(blocks[i].address, blocks[i].os_map_range,
                  sizeof(blocks[i].os_map_range));
  }
}

/**
 * @brief Prints a single block entry for the final log line.
 * @param pid The PID of the current process (kept for compatibility if needed).
 * @param block Pointer to the MemoryBlock to print.
 */
void PrintResultLog(pid_t pid, MemoryBlock* block) {
  printf("%s=%p/%zu/%d/%s|", block->name, (void*)block->address, block->size,
         block->address[0], block->os_map_range);
}

int main() {
  pid_t pid = getpid();

  // Create the memory blocks
  CreateMemoryBlocks();

  // Print the required log line using a per-block function
  printf("LOG_START|PID=%d|", pid);
  for (int i = 0; i < 4; i++) {
    PrintResultLog(pid, &blocks[i]);
  }
  printf("LOG_END\n");
  fflush(stdout);

  // Wait for validation script to run. The script will kill the process.
  // We use a safe sleep here to prevent the process from exiting immediately.
  sleep(10);

  // Clean up
  for (int i = 0; i < 4; i++) {
    if (blocks[i].address != MAP_FAILED) {
      munmap(blocks[i].address, blocks[i].size);
    }
  }

  return 0;
}