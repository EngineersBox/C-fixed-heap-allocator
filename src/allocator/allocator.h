#pragma once

#ifndef H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_
#define H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_

#include <stdint.h>
#include <stddef.h>

typedef long Align;

typedef union header {
    struct {
        union header *ptr;
        unsigned size;
    } s;
    Align x;
} Header;

typedef enum allocation_method {
    BEST_FIT,
    FIRST_FIT,
    WORST_FIT,
    BUDDYS_SYSTEM,
    NEXT_FIT,
} AllocationMethod;

#define NALLOC 1024

typedef struct allocator {
    AllocationMethod method;
    size_t heap_size;
    Header base;
    Header* freep;
    void* current_brk;
    void* heap;
} Allocator;

int cfh_new(Allocator* alloc);
int cfh_init(Allocator* alloc, AllocationMethod method, size_t heap_size);
int cfh_destruct(Allocator* alloc);

void* cfh_malloc(Allocator* alloc, unsigned nbytes);
void cfh_free(Allocator* alloc, void* ap);
void* cfh_sbrk(Allocator* alloc, intptr_t increment);
int cfh_brk(Allocator* alloc, void* addr);

#endif // H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_