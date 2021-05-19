#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "linked-l.h"

#define MAX_ALLOCATIONS   1024

static char** allocations;
static int num_allocations;

void heap_init(void) {
    num_allocations = 0;
    allocations = list_new();
}

void* heap_alloc(size_t size) {
    void *m = malloc(size);
    memset(m, 0, size);
    list_add(allocations, m);
    return m;
}

void heap_free_all(void) {
    list_end_use(allocations);
    num_allocations = 0;
}
