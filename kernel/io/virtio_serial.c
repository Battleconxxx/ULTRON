#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel/virtio_serial.h>

#define VIRTIO_IO_BASE 0xC040

#define VIRTIO_REG_DEVICE_FEATURES  0x00
#define VIRTIO_REG_DRIVER_FEATURES  0x04
#define VIRTIO_REG_QUEUE_ADDRESS    0x08
#define VIRTIO_REG_QUEUE_SIZE       0x0C
#define VIRTIO_REG_QUEUE_SELECT     0x0E
#define VIRTIO_REG_QUEUE_NOTIFY     0x10
#define VIRTIO_REG_DEVICE_STATUS    0x12
#define VIRTIO_REG_ISR_STATUS       0x13

#define VIRTIO_STATUS_ACKNOWLEDGE  (1 << 0)
#define VIRTIO_STATUS_DRIVER       (1 << 1)
#define VIRTIO_STATUS_DRIVER_OK    (1 << 2)
#define VIRTIO_STATUS_FEATURES_OK  (1 << 3)
#define VIRTIO_STATUS_FAILED       (1 << 7)

#define VIRTQ_DESC_F_WRITE      2

static uint8_t virtqueue_mem[4096] __attribute__((aligned(4096)));


static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void setup_virtqueue() {
    memset(virtqueue_mem, 0, sizeof(virtqueue_mem));

    // void* queue_mem = alloc_phys_page();  // This must be 4K aligned, total size for queue
    // if (!queue_mem) {
    //     printf("Failed to allocate virtqueue memory.\n");
    //     return;
    // }

    uintptr_t phys_addr = (uintptr_t)virtqueue_mem;
    uint32_t pfn = phys_addr >> 12;

    // Select queue 0 (receive-transmit queue)
    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_SELECT, 0);

    uint16_t queue_size = inw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_SIZE);
    if (queue_size == 0) {
        printf("Queue size is 0 — device not ready\n");
        return;
    }

    printf("Virtqueue size: %d\n", queue_size);
    printf("This prints\n");
    // Set physical address (page frame number)
    outl(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_ADDRESS, pfn);
    printf("outl done\n");
}

#define STATIC_DMA_ADDR 0x400000

void virtio_serial_send(const char* msg, size_t len) {
    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_SELECT, 0);

    printf("Inside virtio_serial_send\n");
    // struct virtq_desc* desc = (struct virtq_desc*)(virtqueue_mem);
    // struct virtq_avail* avail = (struct virtq_avail*)(virtqueue_mem + 2048);

    struct virtq_desc* desc = vq.desc;
    struct virtq_avail* avail = &vq.avail;
    struct virtq_used* used = &vq.used;

    // used ring is at offset 3072 if you want to read later

    // static uint8_t msg_buf[512];  // Simple buffer; in real use you'd allocate
    // for (size_t i = 0; i < len; i++)
    //     msg_buf[i] = msg[i];

    // uintptr_t msg_phys = (uintptr_t)msg_buf;

    
    volatile char* msg_buf = (char*)STATIC_DMA_ADDR;
    for (size_t i = 0; i < len; i++)
        msg_buf[i] = msg[i];


    desc[0].addr  = (uint64_t)STATIC_DMA_ADDR;
    desc[0].len   = len;
    desc[0].flags = 0;     // device reads, not writes
    desc[0].next  = 0;

    avail->ring[avail->idx % 128] = 0;  // descriptor index 0
    avail->flags = 0;

    __sync_synchronize(); // memory barrier
    avail->idx++;

    // Notify the device (queue 0)
    printf("Notifying VirtIO port...\n");

    printf("desc.addr = %x, len = %x, flags = %x\n", desc[0].addr, desc[0].len, desc[0].flags);
    printf("avail.idx = %x\n", avail->idx);

    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_NOTIFY, 0);
    uint8_t isr = inb(VIRTIO_IO_BASE + VIRTIO_REG_ISR_STATUS);
    printf("ISR status: 0x%x\n", isr);
    //struct virtq_used* used = (struct virtq_used*)(virtqueue_mem + 3072);
    printf("Used ring idx: %d\n", used->idx);

    printf("Should be notified\n");
}


void virtio_serial_init() {
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, 0); // Reset
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACKNOWLEDGE);
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

    uint32_t features = inl(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_FEATURES);
    printf("VirtIO device features: 0x%x\n", features);

    // (optional) Mask features here if needed
    outl(VIRTIO_IO_BASE + VIRTIO_REG_DRIVER_FEATURES, features);  // Accept all for now

    uint8_t status = inb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS);
    status |= VIRTIO_STATUS_FEATURES_OK;
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, status);

    // Check if device accepted features
    status = inb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS);
    if (!(status & VIRTIO_STATUS_FEATURES_OK)) {
        printf("VirtIO feature negotiation failed\n");
        outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_FAILED);
        return;
    }

    // Finally tell the device we’re ready
    status |= VIRTIO_STATUS_DRIVER_OK;
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, status);

    printf("VirtIO-serial initialization completed.\n");
    setup_virtqueue();
    const char* message = "hello from OS\n";
    virtio_serial_send(message, 14);
}