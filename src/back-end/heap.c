#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_ALLOCATIONS   1024

static void* allocations[MAX_ALLOCATIONS];
static int num_allocations;

void heap_init(void) {
    num_allocations = 0;
    memset(allocations, 0, sizeof(allocations));
}

void* heap_alloc(size_t size) {
    void *m = malloc(size);
    memset(m, 0, size);
    allocations[num_allocations++] = m;
    return m;
}

void heap_free_all(void) {
    for (int i = 0; i < num_allocations; i++) {
        free(allocations[i]);
    }
    num_allocations = 0;
}
