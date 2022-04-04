#pragma once

#ifndef H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_
#define H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_

#include <stdint.h>

typedef long Align;

typedef union header {
    struct {
        union header *ptr;
        unsigned size;
    } s;
    Align x;
} Header;

typedef struct allocator {

} Allocator;

void* cfh_malloc(unsigned nbytes);
void cfh_free(void* ap);
void* cfh_sbrk(intptr_t increment);
int cfh_brk(void* addr);

#endif // H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_