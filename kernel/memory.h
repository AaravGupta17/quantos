#ifndef MEMORY_H
#define MEMORY_H

#define HEAP_START 0x10000
#define HEAP_SIZE  0x100000  // 1MB heap

void memory_init();
void* malloc(unsigned int size);
void free(void* ptr);

#endif
