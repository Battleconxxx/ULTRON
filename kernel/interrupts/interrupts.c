#include <stdio.h>
#include <string.h>
#include <kernel/tty.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard_buffer.h>


extern void isr0();
extern void isr8();
extern void isr13();
extern void isr14();
extern void isr32();
extern void isr33();
extern void isr80();
extern void default_isr();

struct idt_entry idt[256];
struct idt_ptr idtp;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

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

    pic_remap();
    idt_install();
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E); // Divide-by-zero
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E); // Double Fault
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // gpf
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault
    idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E); // Timer
    idt_set_gate(0x21, (uint32_t)isr33, 0x08, 0x8E); //Keyboard interrupt
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
    static uint32_t tick = 0;
    tick++;    
}


void keyboard_handler() {
    uint8_t scancode = inb(0x60);
    // Just print for now or store it in a buffer later

    const char scancode_ascii[128] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',  // 0x00 - 0x0E
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',     // 0x0F - 0x1C
        0,  'a','s','d','f','g','h','j','k','l',';','\'','`',          // 0x1D - 0x29
        0,  '\\','z','x','c','v','b','n','m',',','.','/', 0,           // 0x2A - 0x36
        '*', 0,  ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0             // 0x37+
        // Add more if needed
    };

    if (scancode < 128) {
        char c = scancode_ascii[scancode];
        if (c) {

            if(c == '\n'){
                kb_ready = 1;
            }

            int next = (kb_head + 1) % KB_BUFFER_SIZE;
            if (next != kb_tail) {
                kb_buffer[kb_head] = c;
                kb_head = next;
            }

            //  char str[2] = {c, '\0'};
            //  terminal_writestring(str);
        }
    }
}


#define SYSCALL_CLEAR 4
#define SYSCALL_OPEN 5


void syscall_isr_handler(registers_t *regs) {
    uint32_t ret = (uint32_t)-1;

    switch (regs->eax) {
        case SYSCALL_WRITE:
            terminal_writestring((const char*)regs->edx);
            const char* message = "hello from OS\n";
            ret = 0;
            break;

        case SYSCALL_CLEAR:
            terminal_initialize();
            ret = 0;
            break;

        case SYSCALL_OPEN: {
            const char* path = (const char*) regs->ebx;
            if (strncmp(path, "/dev/virtio-ports/socket", 20) == 0) {
                ret = 3;  // Return dummy FD
            } else {
                ret = (uint32_t)-1;
            }
            break;
        }
    }

    regs->eax = ret;
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

