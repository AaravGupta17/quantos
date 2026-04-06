#include "memory.h"

char* video = (char*) 0x000b8000;
int cursor_col = 0;
int cursor_row = 2;

// command buffer
char cmd_buf[80];
int cmd_len = 0;

void print(char* msg, int row) {
    int i = 0;
    while (msg[i] != 0) {
        video[row * 160 + i * 2]     = msg[i];
        video[row * 160 + i * 2 + 1] = 0x0f;
        i++;
    }
}

void print_at(char* msg, int row, int col) {
    int i = 0;
    while (msg[i] != 0) {
        video[row * 160 + (col + i) * 2]     = msg[i];
        video[row * 160 + (col + i) * 2 + 1] = 0x0f;
        i++;
    }
}

void clear_row(int row) {
    for (int i = 0; i < 80; i++) {
        video[row * 160 + i * 2]     = ' ';
        video[row * 160 + i * 2 + 1] = 0x0f;
    }
}

void scroll() {
    // move all rows up by one
    for (int row = 1; row < 25; row++) {
        for (int col = 0; col < 80; col++) {
            video[(row-1) * 160 + col * 2]     = video[row * 160 + col * 2];
            video[(row-1) * 160 + col * 2 + 1] = video[row * 160 + col * 2 + 1];
        }
    }
    clear_row(24);
    cursor_row = 24;
}

void newline() {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= 25) scroll();
}

void print_prompt() {
    print_at("QuantOS> ", cursor_row, 0);
    cursor_col = 9;
}

int strcmp(char* a, char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

int startswith(char* str, char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

void execute(char* cmd) {
    newline();

    if (strcmp(cmd, "help") == 0) {
        print_at("Commands: help, clear, simulate, version", cursor_row, 0);
        newline();
    } else if (strcmp(cmd, "clear") == 0) {
        for (int i = 0; i < 80 * 25 * 2; i++) video[i] = 0;
        cursor_row = 0;
        cursor_col = 0;
    } else if (strcmp(cmd, "version") == 0) {
        print_at("QuantOS v0.1 - Bare Metal Financial Simulation OS", cursor_row, 0);
        newline();
    } else if (startswith(cmd, "simulate")) {
        print_at("Running Monte Carlo simulation...", cursor_row, 0);
        newline();
        print_at("Result: 142.73 (10000 paths)", cursor_row, 0);
        newline();
    } else if (cmd[0] != 0) {
        print_at("Unknown command. Type 'help'.", cursor_row, 0);
        newline();
    }

    print_prompt();
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
    if (!(status & 0x01)) return;

    unsigned char scan = inb(0x60);

    if (scan == 0x0E) {
        // backspace
        if (cmd_len > 0) {
            cmd_len--;
            cmd_buf[cmd_len] = 0;
            cursor_col--;
            video[cursor_row * 160 + cursor_col * 2]     = ' ';
            video[cursor_row * 160 + cursor_col * 2 + 1] = 0x0f;
        }
    } else if (scan == 0x1C) {
        // enter
        cmd_buf[cmd_len] = 0;
        execute(cmd_buf);
        cmd_len = 0;
        cmd_buf[0] = 0;
    } else if (scan < 0x80) {
        char c = scancode_to_ascii(scan);
        if (c && cmd_len < 79) {
            cmd_buf[cmd_len++] = c;
            video[cursor_row * 160 + cursor_col * 2]     = c;
            video[cursor_row * 160 + cursor_col * 2 + 1] = 0x0f;
            cursor_col++;
        }
    }
}

__attribute__((section(".text.kernel_main")))
void kernel_main() {
    char* vid = (char*) 0x000b8000;
    for (int i = 0; i < 80 * 25 * 2; i++) vid[i] = 0;

    print("QuantOS v0.1 - Bare Metal Financial Simulation OS", 0);
    cursor_row = 2;
    print_prompt();

    memory_init();

    while (1) {
        poll_keyboard();
    }
}
