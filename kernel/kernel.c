#include "memory.h"

char* video = (char*) 0x000b8000;
int cursor_col = 0;
int cursor_row = 5;

void print(char* msg, int row) {
    int i = 0;
    while (msg[i] != 0) {
        video[row * 160 + i * 2]     = msg[i];
        video[row * 160 + i * 2 + 1] = 0x0f;
        i++;
    }
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

char scancode_to_ascii(unsigned char scan) {
    char keys[] = {
        0, 0, '1','2','3','4','5','6','7','8','9','0','-','=',
        0, 0, 'q','w','e','r','t','y','u','i','o','p','[',']',
        0, 0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
        0,'\\','z','x','c','v','b','n','m',',','.','/', 0,
        '*', 0, ' '
    };
    if (scan < sizeof(keys)) return keys[scan];
    return 0;
}

void poll_keyboard() {
    unsigned char status = inb(0x64);
    if (status & 0x01) {
        unsigned char scan = inb(0x60);
        if (scan == 0x0E) {
            if (cursor_col > 0) {
                cursor_col--;
                video[cursor_row * 160 + cursor_col * 2]     = ' ';
                video[cursor_row * 160 + cursor_col * 2 + 1] = 0x0f;
            }
        } else if (scan < 0x80) {
            char c = scancode_to_ascii(scan);
            if (c) {
                video[cursor_row * 160 + cursor_col * 2]     = c;
                video[cursor_row * 160 + cursor_col * 2 + 1] = 0x0f;
                cursor_col++;
                if (cursor_col >= 80) {
                    cursor_col = 0;
                    cursor_row++;
                }
            }
        }
    }
}

__attribute__((section(".text.kernel_main")))
void kernel_main() {
    char* vid = (char*) 0x000b8000;
    for (int i = 0; i < 80 * 25 * 2; i++) vid[i] = 0;

    print("QuantOS Kernel v0.1", 0);
    print("Type below:", 4);

    memory_init();

    while (1) {
        poll_keyboard();
    }
}
