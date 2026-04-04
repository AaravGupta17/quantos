[BITS 16]
[ORG 0x7C00]

start:
    mov ah, 0x0E
    mov al, 'Q'
    int 0x10
    mov al, 'u'
    int 0x10
    mov al, 'a'
    int 0x10
    mov al, 'n'
    int 0x10
    mov al, 't'
    int 0x10
    mov al, 'O'
    int 0x10
    mov al, 'S'
    int 0x10

hang:
    jmp hang

times 510-($-$$) db 0
dw 0xAA55
