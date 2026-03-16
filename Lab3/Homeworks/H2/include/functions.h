#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <pthread.h>

// Global text buffer (read-only for worker threads)
extern char* g_text;

// Thread configuration structure
typedef struct {
    int id;               // Paragraph ID
    int start;            // Inclusive start index in g_text
    int end;              // Inclusive end index in g_text
    char keyword[50];     // First word in the paragraph
    int count;            // Number of occurrences of keyword
} ThreadData;

// Provided helper to read the entire file into memory
char* ReadTextFile(const char* filename);

// Worker thread function
void* ThreadFunction(void* arg);

#endif  // INCLUDE_FUNCTIONS_H_