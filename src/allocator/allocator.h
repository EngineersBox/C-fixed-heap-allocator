#pragma once

#ifndef _H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_
#define _H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_

#include <stdint.h>
#include <stddef.h>
#include "../thread/lock.h"

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
    __cfh_lock_t mutex;

    Header base;
    Header* freep;
    void* current_brk;

    size_t heap_size;
    void* heap;
} Allocator;

int cfh_new(Allocator* alloc);
int cfh_init(Allocator* alloc, AllocationMethod method, size_t heap_size)  __attribute__((nonnull));
int cfh_destruct(Allocator* alloc)  __attribute__((nonnull));

void cfh_free(Allocator* alloc, void* ap);
__attribute__((malloc
#if __GNUC__ >= 10
    , malloc (cfh_free, 2)
#endif
)) void* cfh_malloc(Allocator* alloc, unsigned nbytes) __attribute__((nonnull));
__attribute__((malloc
#if __GNUC__ >= 10
    , malloc (cfh_free, 2)
#endif
)) void* cfh_calloc(Allocator* alloc, unsigned count, unsigned nbytes) __attribute__((nonnull));
void* cfh_realloc(Allocator* alloc, void* ap, unsigned nbytes) __attribute__((nonnull(1)));

void* cfh_sbrk(Allocator* alloc, intptr_t increment) __attribute__((nonnull(1)));
int cfh_brk(Allocator* alloc, void* addr) __attribute__((nonnull(1)));

#endif // _H_C_FIXED_HEAP_ALLOCATOR_ALLOCATOR_