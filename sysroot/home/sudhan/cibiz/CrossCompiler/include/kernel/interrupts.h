#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct registers {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

#define SYSCALL_WRITE 2
#define SYSCALL_CLEAR 4

void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install();
void init_interrupts();
void divide_by_zero_handler(uint32_t error_code , uint32_t int_no);
void gpf_handler(uint32_t error_code , uint32_t int_no);
void page_fault_handler(uint32_t fault_addr, uint32_t error_code);
void timer_handler(uint32_t error_code, uint32_t interrupt_number);
void syscall_isr_handler(registers_t *regs);
void double_fault_handler(uint32_t error_code, uint32_t int_no);
void fault_handler(uint32_t int_no, uint32_t error_code);

#endif