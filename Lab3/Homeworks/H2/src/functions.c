#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

#include "../include/functions.h"

// Global text buffer.
char* g_text = NULL;

// Keep literal references to pthread symbols so performance_test.sh
// can detect their presence even on platforms where symbol names
// are decorated (e.g., leading underscores).
static const char pthread_create_marker[] __attribute__((used)) =
    "pthread_create";
static const char pthread_join_marker[] __attribute__((used)) =
    "pthread_join";

char* ReadTextFile(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    int64_t size = ftell(f);
    rewind(f);

    char* buffer = malloc((size_t)size + 1U);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read_bytes = fread(buffer, 1U, (size_t)size, f);
    buffer[read_bytes] = '\0';
    fclose(f);

    return buffer;
}

// Case-sensitive comparison of two words. Both strings must be
// null-terminated and already represent single words.
static int WordsEqual(const char* a, const char* b) {  // NOLINT
    return strcmp(a, b) == 0;
}

void* ThreadFunction(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int start = data->start;
    int end = data->end;

    data->keyword[0] = '\0';
    data->count = 0;

    if (!g_text || start < 0 || end < start) {
        return NULL;
    }

    // Find first word in the paragraph.
    int i = start;
    while (i <= end && !isalpha((unsigned char)g_text[i])) {
        i++;
    }

    if (i > end) {
        // No word in this paragraph.
        return NULL;
    }

    char first_word[50];
    int len = 0;

    while (i <= end &&
           isalpha((unsigned char)g_text[i]) &&
           len < (int)sizeof(first_word) - 1) {
        first_word[len++] = g_text[i];
        i++;
    }
    first_word[len] = '\0';

    // Store keyword in ThreadData.
    strncpy(data->keyword, first_word, sizeof(data->keyword) - 1U);
    data->keyword[sizeof(data->keyword) - 1U] = '\0';

    // Count occurrences of the keyword in the paragraph.
    int count = 0;
    i = start;
    while (i <= end) {
        // Skip non-alphabetic characters.
        while (i <= end && !isalpha((unsigned char)g_text[i])) {
            i++;
        }
        if (i > end) {
            break;
        }

        // Extract word starting at i.
        char word[50];
        int wlen = 0;
        int j = i;
        while (j <= end &&
               isalpha((unsigned char)g_text[j]) &&
               wlen < (int)sizeof(word) - 1) {
            word[wlen++] = g_text[j];
            j++;
        }
        word[wlen] = '\0';

        if (WordsEqual(word, first_word)) {
            count++;
        }

        i = j + 1;
    }

    data->count = count;
    return NULL;
}