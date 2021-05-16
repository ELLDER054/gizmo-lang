#ifndef HEAP_H
#define HEAP_H

void heap_init(void);
void* heap_alloc(size_t size);
void heap_free_all(void);

#endif /* HEAP_H */
