// memfile.h
#ifndef MEMFILE_H
#define MEMFILE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define SEEK_SET 0  // Seek from beginning of file
#define SEEK_CUR 1  // Seek from current file position
#define SEEK_END 2  // Seek from end of file


// A minimal FILE-like handle for an in‑memory buffer
typedef struct {
    uint8_t *data;   // start of buffer
    size_t   size;   // total size
    size_t   pos;    // current read/write offset
} FILE;

// Externally provided by your kernel after loading via multiboot:
extern void   *model_data;
extern size_t  model_size;
extern void   *tokenizer_data;
extern size_t  tokenizer_size;

// Open the in‑memory “file” identified by name.
// Recognizes exactly "stories15M.bin" and "tokenizer.bin".
static inline FILE *fopen(const char *name, const char *mode) {
    (void)mode; // ignored
    static FILE f; 
    if (strcmp(name, "stories15M.bin") == 0) {
        f.data = (uint8_t*)model_data;
        f.size = model_size;
    }
    else if (strcmp(name, "tokenizer.bin") == 0) {
        f.data = (uint8_t*)tokenizer_data;
        f.size = tokenizer_size;
    }
    else {
        return NULL;
    }
    f.pos = 0;
    return &f;
}

// Read up to (size*nmemb) bytes into ptr, return number of items read
static inline size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t total = size * nmemb;
    size_t avail = (stream->pos + total > stream->size)
                   ? (stream->size - stream->pos)
                   : total;
    memcpy(ptr, stream->data + stream->pos, avail);
    stream->pos += avail;
    return avail / size;
}

// Seek to new position; returns 0 on success, -1 on error
static inline int fseek(FILE *stream, long offset, int whence) {
    size_t newpos;
    switch (whence) {
    case SEEK_SET: newpos = offset;           break;
    case SEEK_CUR: newpos = stream->pos + offset; break;
    case SEEK_END: newpos = stream->size + offset; break;
    default: return -1;
    }
    if (newpos > stream->size) return -1;
    stream->pos = newpos;
    return 0;
}

// Return current position, or -1 on error
static inline long ftell(FILE *stream) {
    return (stream->pos <= stream->size) ? (long)stream->pos : -1L;
}

// No-op in a memory-only context
static inline int fclose(FILE *stream) {
    (void)stream;
    return 0;
}

#endif // MEMFILE_H
