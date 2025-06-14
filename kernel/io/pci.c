#include <kernel/io.h>
#include <stdint.h>
#include <stdio.h>  // Or your own kprintf

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}


uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1U << 31) |  // enable bit
                       ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) |
                       (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_scan_for_virtio() {
    for (uint8_t bus = 0; bus < 256; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            uint32_t vendor_device = pci_config_read(bus, slot, 0, 0x00);
            uint16_t vendor = vendor_device & 0xFFFF;
            uint16_t device = (vendor_device >> 16) & 0xFFFF;

            if (vendor == 0x1AF4 && device == 0x1003) {
                printf("Found VirtIO-serial at bus %d, slot %d\n", bus, slot);

                uint32_t bar0 = pci_config_read(bus, slot, 0, 0x10);
                uint32_t mmio_base = bar0 & ~0xF;

                printf("VirtIO-serial MMIO base: %x\n", mmio_base);

                uint32_t irq_data = pci_config_read(bus, slot, 0, 0x3C);
                uint8_t irq_line = irq_data & 0xFF;
                printf("VirtIO IRQ line: %d\n", irq_line);

                // Save bus/slot/mmio_base for next step
                // Call virtio_serial_init(mmio_base);
                return;
            }
        }
    }
}
