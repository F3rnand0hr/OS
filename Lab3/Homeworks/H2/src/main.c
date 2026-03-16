#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../include/functions.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <textfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load entire file into memory.
    g_text = ReadTextFile(argv[1]);
    if (!g_text) {
        fprintf(stderr, "Failed to read file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    size_t len = strlen(g_text);
    if (len == 0U) {
        free(g_text);
        return EXIT_SUCCESS;
    }

    // First pass: count paragraphs (separated by '\n').
    int paragraphs = 0;
    if (len > 0U) {
        // At least one paragraph if file not empty.
        paragraphs = 1;
        for (size_t i = 0U; i < len; i++) {
            if (g_text[i] == '\n') {
                paragraphs++;
            }
        }
        // If file ends with '\n', last paragraph after it is empty; remove it.
        if (g_text[len - 1U] == '\n') {
            paragraphs--;
        }
    }

    if (paragraphs <= 0) {
        free(g_text);
        return EXIT_SUCCESS;
    }

    pthread_t* threads = (pthread_t*)malloc(
        sizeof(pthread_t) * (size_t)paragraphs);
    ThreadData* data = (ThreadData*)malloc(
        sizeof(ThreadData) * (size_t)paragraphs);
    if (!threads || !data) {
        free(threads);
        free(data);
        free(g_text);
        fprintf(stderr, "Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    // Second pass: compute start/end indices for each paragraph.
    int idx = 0;
    int start = 0;
    for (size_t i = 0U; i < len && idx < paragraphs; i++) {
        if (g_text[i] == '\n') {
            int end = (int)i - 1;
            if (end >= start) {
                data[idx].id = idx;
                data[idx].start = start;
                data[idx].end = end;
                data[idx].keyword[0] = '\0';
                data[idx].count = 0;
                pthread_create(&threads[idx], NULL, ThreadFunction, &data[idx]);
                idx++;
            }
            start = (int)i + 1;
        }
    }
    // Last paragraph (if file does not end with '\n').
    if (start <= (int)len - 1 && idx < paragraphs) {
        data[idx].id = idx;
        data[idx].start = start;
        data[idx].end = (int)len - 1;
        data[idx].keyword[0] = '\0';
        data[idx].count = 0;
        pthread_create(&threads[idx], NULL, ThreadFunction, &data[idx]);
        idx++;
    }

    // Wait for all threads.
    for (int i = 0; i < idx; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print results.
    for (int i = 0; i < idx; i++) {
        printf("PARAGRAPH %d | KEYWORD: %s | COUNT: %d\n",
               data[i].id, data[i].keyword, data[i].count);
    }

    free(threads);
    free(data);
    free(g_text);

    return EXIT_SUCCESS;
}