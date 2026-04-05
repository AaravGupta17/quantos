#include "memory.h"

// Each block has a header describing it
typedef struct block {
    unsigned int size;
    int free;
    struct block* next;
} block_t;

static block_t* heap = (block_t*) HEAP_START;

void memory_init() {
    heap->size = HEAP_SIZE - sizeof(block_t);
    heap->free = 1;
    heap->next = 0;
}

void* malloc(unsigned int size) {
    block_t* current = heap;

    // Find a free block big enough
    while (current) {
        if (current->free && current->size >= size) {
            // Split block if large enough
            if (current->size > size + sizeof(block_t)) {
                block_t* new = (block_t*)((char*)current 
                               + sizeof(block_t) + size);
                new->size = current->size - size - sizeof(block_t);
                new->free = 1;
                new->next = current->next;
                current->next = new;
                current->size = size;
            }
            current->free = 0;
            return (char*)current + sizeof(block_t);
        }
        current = current->next;
    }
    return 0;  // out of memory
}

void free(void* ptr) {
    if (!ptr) return;
    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    block->free = 1;

    // Merge adjacent free blocks
    block_t* current = heap;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(block_t) + current->next->size;
            current->next  = current->next->next;
        }
        current = current->next;
    }
}
