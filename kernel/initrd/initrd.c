#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define CPIO_NEWC_MAGIC "070701"
#define ALIGN4(x) (((x) + 3) & ~3)

typedef struct {
    const char* name;
    void* data;
    size_t size;
} initrd_file_t;

static const char* initrd_start = NULL;
static const char* initrd_end = NULL;

void initrd_setup(const void* addr) {
    initrd_start = (const char*)addr;
}

void* initrd_find_file(const char* filename, size_t* out_size) {
    const char* p = initrd_start;

    while (1) {
        if (memcmp(p, CPIO_NEWC_MAGIC, 6) != 0)
            break;

        // Parse header
        uint32_t namesize = strtol(p + 94, NULL, 16);
        uint32_t filesize = strtol(p + 54, NULL, 16);

        const char* name = p + 110;
        const char* data = (const char*)ALIGN4((uintptr_t)(name + namesize));
        const char* next = (const char*)ALIGN4((uintptr_t)(data + filesize));

        if (strncmp(name, "TRAILER!!!", 10) == 0)
            break;

        if (strncmp(name, filename, strlen(filename)) == 0) {
            if (out_size) *out_size = filesize;
            printf("File found in initrd: %s\n", name);
            return (void*)data;
        }

        p = next;
    }

    return NULL; // not found
}
