isr80:
    cli

    ; Save segment registers
    push ds
    push es

    ; Load kernel data segments (safely using cx)
    mov cx, 0x10
    mov ds, cx
    mov es, cx

    pusha                   ; saves eax, ecx, edx, ebx, esp (dummy), ebp, esi, edi

    ; Capture return-to-user values *without* touching eax/edx
    mov ebx, [esp + 36]     ; user_ss
    mov ecx, [esp + 40]     ; user_esp
    mov esi, [esp + 44]     ; eflags
    mov edi, [esp + 48]     ; user_cs
    mov ebp, [esp + 52]     ; user_eip

    ; Push return frame in correct order: SS, ESP, EFLAGS, CS, EIP
    push ebx
    push ecx
    push esi
    push edi
    push ebp

    push esp                ; pointer to registers_t (after pusha)
    call syscall_isr_handler
    add esp, 4

    add esp, 20             ; clean up return frame

    popa
    pop es
    pop ds

    iretd
