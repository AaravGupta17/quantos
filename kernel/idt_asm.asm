[BITS 32]

global keyboard_handler_asm

keyboard_handler_asm:
    push eax
    push ebx
    push ecx

    ; read scancode
    in al, 0x60

    ; ignore key release
    cmp al, 0x80
    jge .eoi

    ; save scancode
    movzx ecx, al

    ; look up ASCII
    mov al, byte [0x6000 + ecx]
    test al, al
    jz .eoi

    ; save ASCII char
    mov cl, al

    ; get cursor
    mov ebx, dword [0x6100]

    ; compute video address: 0xb8320 + cursor*2
    mov eax, ebx
    shl eax, 1
    add eax, 0xb8320

    ; write character and color
    mov byte [eax], cl
    mov byte [eax+1], 0x0f

    ; increment and save cursor
    inc ebx
    mov dword [0x6100], ebx

.eoi:
    mov al, 0x20
    out 0x20, al

    pop ecx
    pop ebx
    pop eax
    iret
