#include "idt.h"

#define IDT_BASE 0x5000
#define IDT_SIZE 256

typedef struct {
    unsigned short base_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  flags;
    unsigned short base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed)) idt_ptr_t;

extern void keyboard_handler_asm();

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

void idt_init() {
    idt_entry_t* idt = (idt_entry_t*) IDT_BASE;
    unsigned char* p = (unsigned char*) IDT_BASE;
    for (int i = 0; i < IDT_SIZE * 8; i++) p[i] = 0;

    // Set keyboard handler at interrupt 33
    unsigned int base = (unsigned int) keyboard_handler_asm;
    idt[33].base_low  = base & 0xFFFF;
    idt[33].base_high = (base >> 16) & 0xFFFF;
    idt[33].selector  = 0x08;
    idt[33].zero      = 0;
    idt[33].flags     = 0x8E;

    idt_ptr_t idt_p;
    idt_p.limit = (IDT_SIZE * 8) - 1;
    idt_p.base  = IDT_BASE;

    // Remap PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask ALL IRQs except IRQ1 (keyboard) = 0xFD
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);

    __asm__ volatile ("lidt (%0)" :: "r"(&idt_p));
    __asm__ volatile ("sti");
}
