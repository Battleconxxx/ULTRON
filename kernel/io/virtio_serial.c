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

#define VIRTQ_DESC_F_NEXT       1
#define VIRTQ_DESC_F_WRITE      2

// Control message definitions
#define VIRTIO_CONSOLE_CTRL_ADD_PORT  1
#define VIRTIO_CONSOLE_CTRL_PORT_NAME 2

static uint8_t virtqueue_mem[4096] __attribute__((aligned(4096)));

static uint8_t ctrl_queue_mem[4096] __attribute__((aligned(4096)));
static uint8_t data_queue_mem[4096] __attribute__((aligned(4096)));


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
    // Set physical address (page frame number)
    outl(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_ADDRESS, pfn);
    printf("outl done\n");
}


#define STATIC_DMA_ADDR 0x400000


void virtio_serial_send(const char* msg, size_t len) {
    volatile char* msg_buf = (char*)STATIC_DMA_ADDR;
    for (size_t i = 0; i < len; i++) msg_buf[i] = msg[i];

    struct virtq_desc* desc = (struct virtq_desc*)data_queue_mem;
    struct virtq_avail* avail = (struct virtq_avail*)(data_queue_mem + 2048);

    desc[0].addr  = STATIC_DMA_ADDR;
    desc[0].len   = len;
    desc[0].flags = 0;
    desc[0].next  = 0;

    avail->ring[avail->idx % 128] = 0;
    __sync_synchronize();
    avail->idx++;

    printf("[guest] about to send %d bytes on queue 2\n", len);
    printf("[guest] desc.addr=%x len=%d\n",
       (unsigned long long)desc[0].addr, (unsigned)desc[0].len);

    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_SELECT, 2);
    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_NOTIFY, 2);
    struct virtq_used* used = (struct virtq_used*)(virtqueue_mem + 3072);
    printf("Used ring idx: %d\n", used->idx);

    printf("[guest] notify(2) done; now polling used ring…\n");
struct virtq_used* data_used = 
    (struct virtq_used*)(data_queue_mem + 3072);
uint16_t seen2 = data_used->idx;
while (data_used->idx == seen2) {
    // spin
}

    printf("Message sent to host via VirtIO port.\n");
}


void virtio_serial_init() {
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, 0); // Reset
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACKNOWLEDGE);
    outb(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

    uint32_t features = inl(VIRTIO_IO_BASE + VIRTIO_REG_DEVICE_FEATURES);
    printf("VirtIO device features: %x\n", features);

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

    setup_queue(0, ctrl_queue_mem); // control queue
    setup_queue(2, data_queue_mem); // transmit queue
    printf("VirtIO-serial initialization completed.\n");
    //setup_virtqueue();


    // Send port add request (port id 1)
    volatile struct virtio_console_control* ctrl_msg = (struct virtio_console_control*)STATIC_DMA_ADDR;
    ctrl_msg->id = 1;
    ctrl_msg->event = VIRTIO_CONSOLE_CTRL_ADD_PORT;
    ctrl_msg->value = 1;

    struct virtq_desc* desc = (struct virtq_desc*)ctrl_queue_mem;
    struct virtq_avail* avail = (struct virtq_avail*)(ctrl_queue_mem + 2048);

    desc[0].addr = STATIC_DMA_ADDR;
    desc[0].len = sizeof(struct virtio_console_control);
    desc[0].flags = 0;
    desc[0].next = 0;

    avail->ring[avail->idx % 128] = 0;
    __sync_synchronize();
    avail->idx++;

    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_SELECT, 0);
    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_NOTIFY, 0);
    printf("Control message to add port sent.\n");

    struct virtio_console_control* name_msg =
        (struct virtio_console_control*)STATIC_DMA_ADDR;

    // port ID must match the one you added (1)
    name_msg->id    = 1;
    name_msg->event = VIRTIO_CONSOLE_CTRL_PORT_NAME;  // == 2

    // copy the null‑terminated name
    const char* port_name = "org.qemu.console.ai";
    size_t namelen = strlen(port_name) + 1;  // include '\0'
    memcpy(name_msg->value, port_name, namelen);

    // build descriptor & avail entry on the CTRL queue
    struct virtq_desc*  ctrl_desc  = (struct virtq_desc*)ctrl_queue_mem;
    struct virtq_avail* ctrl_avail = (struct virtq_avail*)(ctrl_queue_mem + 2048);

    ctrl_desc[0].addr  = (uint64_t)STATIC_DMA_ADDR;
    ctrl_desc[0].len   = (uint32_t)namelen;
    ctrl_desc[0].flags = 0;       // driver→device
    ctrl_desc[0].next  = 0;

    // ring it
    ctrl_avail->ring[ctrl_avail->idx % 128] = 0;
    __sync_synchronize();
    ctrl_avail->idx++;

    // notify the control queue (queue 0)
    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_SELECT, 0);
    outw(VIRTIO_IO_BASE + VIRTIO_REG_QUEUE_NOTIFY, 0);

    printf("Control message (PORT_NAME) sent: '%s'\n", port_name);

    struct virtq_used* ctrl_used =
    (struct virtq_used*)(ctrl_queue_mem + 3072);

    // remember the “already‑used” count
    uint16_t seen = ctrl_used->idx;

    printf("[guest] waiting for ctrl queue to process PORT_NAME\n");
    while (ctrl_used->idx == seen) {
        // you could add a small delay here if you like
    }
    printf("[guest] control handshake complete; port is live!\n");
    
}