#ifndef VIRTIO_SERIAL_H
#define VIRTIO_SERIAL_H

#define VIRTQUEUE_NUM 128

struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[VIRTQUEUE_NUM];
};

struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
};

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[VIRTQUEUE_NUM];
};

struct virtqueue {
    struct virtq_desc desc[128];
    struct virtq_avail avail;
    uint8_t pad[4096 - sizeof(struct virtq_desc) * 128 - sizeof(struct virtq_avail) - sizeof(struct virtq_used)];
    struct virtq_used used;
} __attribute__((packed, aligned(4096)));

struct virtio_console_control {
    uint32_t id;
    uint16_t event;
    uint16_t value;
};

static volatile struct virtqueue vq;

void setup_virtqueue();
void virtio_serial_send(const char* msg, size_t len);
void virtio_serial_init();



#endif