[bits 32]

global isr0
extern divide_by_zero_handler

isr0:
    pusha
    push dword 0           ; dummy error code (no error code for #DE)
    push dword 0           ; interrupt number 0
    call divide_by_zero_handler
    add esp, 8
    popa
    iret


global isr8
extern double_fault_handler

isr8:
    cli
    pusha
    push dword 0           ; double fault has error code 0 pushed by CPU
    push dword 8           ; interrupt number 8
    call double_fault_handler
    add esp, 8
    popa
    iret


global isr13
extern gpf_handler

isr13:
    cli
    push ds
    push es
    push fs
    push gs

    pusha                   ; Push general-purpose registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [esp + 44]   ; Push error code (it's at esp+44 after pusha+segment regs)
    push dword 13           ; Push interrupt number
    call gpf_handler

    add esp, 8              ; Clean up pushed int_no and error_code
    popa
    pop gs
    pop fs
    pop es
    pop ds
    iret



global isr14
extern page_fault_handler
isr14:
    cli
    pusha
    mov eax, [esp + 32]    ; Error code (adjusted for pusha)
    push eax
    mov eax, cr2           ; Faulting address
    push eax
    call page_fault_handler
    add esp, 8
    popa
    iret


global isr32
extern timer_handler

isr32:
    cli
    pusha
    mov eax, esp           ; pointer to saved registers on stack
    push eax
    call timer_handler
    pop eax
    mov esp, eax           ; switch to new stack pointer returned by timer_handler
    popa                   ; restore registers from new stack
    sti
    iret

global isr33
extern keyboard_handler

isr33:
    cli
    pusha
    call keyboard_handler
    popa
    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al
    sti
    iret



global isr80
extern syscall_isr_handler

isr80:
    cli

    ; Save segment registers
    push ds
    push es

    ; Load kernel data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    pusha

    ; Push return-to-user values from stack (still safe since we know where they are)
    push DWORD [esp + 44]     ; ss
    push DWORD [esp + 44]     ; user esp
    push DWORD [esp + 44]     ; eflags
    push DWORD [esp + 44]     ; cs
    push DWORD [esp + 44]     ; eip

    push esp                  ; Pointer to full struct
    call syscall_isr_handler
    add esp, 4

    add esp, 20               ; clean up user return values

    popa
    pop es
    pop ds

    iretd




global default_isr
extern fault_handler

default_isr:
    cli
    pusha
    push dword 0          ; Dummy error code
    push eax              ; Push interrupt number (caller sets %eax)
    call fault_handler
    add esp, 8
    popa
    iret
