void kernel_main() {
    // Video memory starts at 0xb8000
    // Each character = 2 bytes: ASCII + color
    char* video = (char*) 0xb8000;

    // Clear screen
    for (int i = 0; i < 80 * 25 * 2; i++) {
        video[i] = 0;
    }

    // Write "QuantOS Kernel" to screen
    char* msg = "QuantOS Kernel";
    int i = 0;
    while (msg[i] != 0) {
        video[i * 2]     = msg[i];   // character
        video[i * 2 + 1] = 0x0f;    // white on black
        i++;
    }

    // Hang
    while (1) {}
}
