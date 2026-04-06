#include "memory.h"
#include "rng.h"

char* video = (char*) 0x000b8000;
int cursor_col = 0;
int cursor_row = 2;

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
        video[row * 160 + (col+i) * 2]     = msg[i];
        video[row * 160 + (col+i) * 2 + 1] = 0x0f;
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
    for (int row = 1; row < 25; row++)
        for (int col = 0; col < 80; col++) {
            video[(row-1)*160 + col*2]   = video[row*160 + col*2];
            video[(row-1)*160 + col*2+1] = video[row*160 + col*2+1];
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

// print integer
void print_int(int n, int row, int col) {
    if (n == 0) {
        video[row*160 + col*2] = '0';
        video[row*160 + col*2+1] = 0x0f;
        return;
    }
    char buf[12];
    int i = 0;
    int neg = 0;
    if (n < 0) { neg = 1; n = -n; }
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    if (neg) buf[i++] = '-';
    // reverse
    for (int j = 0; j < i/2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i-1-j];
        buf[i-1-j] = tmp;
    }
    buf[i] = 0;
    print_at(buf, row, col);
}

// Monte Carlo simulation
// Geometric Brownian Motion: S(t) = S0 * exp((mu - 0.5*sigma^2)*t + sigma*sqrt(t)*Z)
float monte_carlo(float S0, float drift, float vol, int paths, int steps) {
    float dt = 1.0f / steps;
    float sum = 0.0f;

    for (int p = 0; p < paths; p++) {
        float S = S0;
        for (int t = 0; t < steps; t++) {
            float Z = rng_normal();
            float exp_arg = (drift - 0.5f * vol * vol) * dt
                          + vol * Z * dt;
            // e^x approximation: 1 + x + x^2/2 + x^3/6
            float ex = 1.0f + exp_arg
                     + exp_arg*exp_arg/2.0f
                     + exp_arg*exp_arg*exp_arg/6.0f;
            S = S * ex;
        }
        sum += S;
    }
    return sum / paths;
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
        print_at("Commands: help, clear, version", cursor_row, 0);
        newline();
        print_at("simulate <price> <drift> <vol> <paths>", cursor_row, 0);
        newline();
        print_at("Example: simulate 180 8 25 1000", cursor_row, 0);
        newline();
    } else if (strcmp(cmd, "clear") == 0) {
        for (int i = 0; i < 80*25*2; i++) video[i] = 0;
        cursor_row = 0;
        cursor_col = 0;
    } else if (strcmp(cmd, "version") == 0) {
        print_at("QuantOS v0.1 - Bare Metal Financial Simulation OS", cursor_row, 0);
        newline();
    } else if (startswith(cmd, "simulate")) {
        print_at("Running Monte Carlo simulation on bare metal...", cursor_row, 0);
        newline();

        // seed RNG with a fixed value for now
        rng_seed(0xDEADBEEF);

        // default params: AAPL-like stock
        float result = monte_carlo(180.0f, 0.08f, 0.25f, 1000, 252);

        // display result as integer (no printf)
        int result_int = (int) result;
        print_at("Expected price: $", cursor_row, 0);
        print_int(result_int, cursor_row, 17);
        newline();
        print_at("Paths: 1000 | Steps: 252 | Bare metal", cursor_row, 0);
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
        if (cmd_len > 0) {
            cmd_len--;
            cmd_buf[cmd_len] = 0;
            cursor_col--;
            video[cursor_row*160 + cursor_col*2]   = ' ';
            video[cursor_row*160 + cursor_col*2+1] = 0x0f;
        }
    } else if (scan == 0x1C) {
        cmd_buf[cmd_len] = 0;
        execute(cmd_buf);
        cmd_len = 0;
        cmd_buf[0] = 0;
    } else if (scan < 0x80) {
        char c = scancode_to_ascii(scan);
        if (c && cmd_len < 79) {
            cmd_buf[cmd_len++] = c;
            video[cursor_row*160 + cursor_col*2]   = c;
            video[cursor_row*160 + cursor_col*2+1] = 0x0f;
            cursor_col++;
        }
    }
}

__attribute__((section(".text.kernel_main")))
void kernel_main() {
    char* vid = (char*) 0x000b8000;
    for (int i = 0; i < 80*25*2; i++) vid[i] = 0;

    print("QuantOS v0.1 - Bare Metal Financial Simulation OS", 0);
    cursor_row = 2;
    print_prompt();

    memory_init();
    rng_seed(12345);

    while (1) {
        poll_keyboard();
    }
}
