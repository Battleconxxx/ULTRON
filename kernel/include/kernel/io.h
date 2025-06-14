#ifndef IO_H
#define IO_H 

#pragma once

#include <stdint.h>

static inline uint32_t inl(uint16_t port);
static inline void outl(uint16_t port, uint32_t val);


#endif