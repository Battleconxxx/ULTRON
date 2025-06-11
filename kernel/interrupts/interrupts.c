#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/interrupts.h>

extern void isr0();
extern void isr8();
extern void isr13();
extern void isr14();
extern void isr32();
extern void isr80();
extern void default_isr();

struct idt_entry idt[256];
struct idt_ptr idtp;


void idt_set_gate(int num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].base_high = (base >> 16) & 0xFFFF;
}

void idt_install(){
    idtp.limit = (sizeof(struct idt_entry) * 256) -1;
    idtp.base = (uint32_t)&idt;

    for(int i=0; i<256; i++){
        idt_set_gate(i, (uint32_t)default_isr, 0x08, 0x8E);
    }
}

void init_interrupts(){

    idt_install();
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E); // Divide-by-zero
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E); // Double Fault
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault
    idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E); // Timer
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // gpf
    idt_set_gate(0x80, (uint32_t)isr80, 0x08, 0xEE);  // Interrupt gate, DPL=3 (0xEE)

    extern void idt_load();
    idt_load();
}

void divide_by_zero_handler(uint32_t error_code , uint32_t int_no){
    printf("Divide by zero error %x , error_code: %x", int_no , error_code);
    for(;;){}
}

void gpf_handler(uint32_t error_code , uint32_t int_no) {
    printf("General Protection Fault error %x , error_code: %x", int_no , error_code);
    for(;;);
}

void page_fault_handler(uint32_t fault_addr, uint32_t error_code){
    printf("Page Fault fault_arrd: %x , error_code: %x", fault_addr, error_code);
    for(;;){}
}

void timer_handler(uint32_t error_code, uint32_t interrupt_number) {
    asm volatile("cli"); // Disable interrupts
    // Acknowledge the interrupt (send EOI to PIC)
    //outb(0x20, 0x20); // EOI to PIC1 (master)

    // Debug output (optional, use with caution to avoid recursion)
    printf("Timer interrupt (IRQ0) fired, tick %u error_code : %x\n", interrupt_number, error_code);

    // Optionally, increment a system tick counter
    static uint32_t tick = 0;
    tick++;
    // volatile uint32_t *debug = (volatile uint32_t *)0xC0001000; // Mapped address
    // debug[3] = tick; // Store tick count

    // Re-enable interrupts (handled by iret in isr32)
}

void syscall_isr_handler(registers_t *regs) {
    uint32_t ret = (uint32_t)-1; // Default return value for unknown syscall
    uint32_t edx_val;
    asm volatile("mov %%edx, %0" : "=r"(edx_val));
    // terminal_writestring((const char*)regs->edx); // arg1 is pointer to string
    terminal_writestring((const char*)edx_val); // arg1 is pointer to string
    // switch (regs->eax) {
    //     case SYSCALL_WRITE:
    //         terminal_writestring((const char*)regs->ebx); // arg1 is pointer to string
    //         ret = 0;
    //         break;
    // }

    regs->eax = ret; // Store return value in eax
}

void double_fault_handler(uint32_t error_code, uint32_t int_no) {
    // Double Fault usually has error_code = 0 (CPU pushes 0)
    printf("Double Fault Exception (int %d), error code: 0x%x\n", int_no, error_code);
    // Halt the system since this is critical
    for(;;){}
}

void fault_handler(uint32_t int_no, uint32_t error_code) {
    printf("Unhandled exception %x, error: 0x%x\n", int_no, error_code);
    for(;;){}
}

