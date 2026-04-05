#include "memory.h"

void kernel_main() {
    char* video = (char*) 0xb8000;
    for (int i = 0; i < 80 * 25 * 2; i++) video[i] = 0;

    // Write directly without calling print first
    char* msg = "QuantOS Kernel";
    int i = 0;
    while (msg[i] != 0) {
        video[i * 2]     = msg[i];
        video[i * 2 + 1] = 0x0f;
        i++;
    }

    memory_init();

    int* a = (int*) malloc(sizeof(int));
    *a = 42;

    if (*a == 42) {
        char* msg2 = "Memory allocator OK";
        int j = 0;
        while (msg2[j] != 0) {
            video[160 + j * 2]     = msg2[j];
            video[160 + j * 2 + 1] = 0x0f;
            j++;
        }
    }

    while (1) {}
}
