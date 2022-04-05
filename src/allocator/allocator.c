#include "allocator.h"
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../error/allocator_errno.h"

#define NALLOC 1024

static Header base;
static Header *freep = NULL;

void* __cfh_curbrk = 0;

int cfh_new(Allocator* alloc) {
    if (alloc == NULL) {
        set_alloc_errno(NULL_ALLOCATOR_INSTANCE);
        return -1;
    }
    alloc->heap_size = 0;
    alloc->heap = NULL;
    return 0;
}

int cfh_init(Allocator* alloc,
             AllocationMethod method,
             size_t heap_size) {
    if (alloc == NULL) {
        set_alloc_errno(NULL_ALLOCATOR_INSTANCE);
        return -1;
    } else if (alloc->heap != NULL) {
        set_alloc_errno(HEAP_ALREADY_MAPPED);
        return -1;
    }
    alloc->method = method;
    alloc->heap_size = heap_size;
    alloc->heap = mmap(
        NULL,
        heap_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );
    if (alloc->heap == NULL) {
        set_alloc_errno(HEAP_MMAP_FAILED);
        return -1;
    }
    return 0;
}

int cfh_destruct(Allocator* alloc) {
    if (alloc == NULL) {
        set_alloc_errno(BAD_DEALLOC);
        return -1;
    } else if (alloc->heap != NULL && munmap(alloc->heap, alloc->heap_size)) {
        set_alloc_errno(HEAP_UNMAP_FAILED);
        return -1;
    }
    free(alloc);
    return 0;
}

int cfh_brk(void* addr) {
    void* newbrk;

    if (newbrk < addr) {
        return -1;
    }
    return 0;
}

void* cfh_sbrk(intptr_t increment) {
    void* oldbrk;
    if (__cfh_curbrk == NULL && cfh_brk(0) < 0) {
        return (void*) -1;
    }
    if (increment == 0) {
        return __cfh_curbrk;
    }
    oldbrk = __cfh_curbrk;
    if (increment > 0
        ? ((uintptr_t) oldbrk + (uintptr_t) increment < (uintptr_t) oldbrk)
        : ((uintptr_t) oldbrk < (uintptr_t) -increment)){
        return (void*) -1;
    }
    if (cfh_brk(oldbrk + increment) < 0) {
        return (void*) -1;
    }
    return oldbrk;
}

void cfh_free(void* ap) {
    Header* bp, *p;

    bp = (Header*) ap - 1;
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
            break;
        }
    }
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }
    freep = p;
}

static Header* morecore(unsigned int nu) {
    char* cp;
    Header* up;

    if (nu < NALLOC) {
        nu = NALLOC;
    }
    cp = sbrk(nu * sizeof(Header));
    if (cp == (char*) - 1) {
        return NULL;
    }
    up = (Header*) cp;
    up->s.size = nu;
    cfh_free((void*)(up + 1));
    return freep;
}

void* cfh_malloc(unsigned nbytes) {
    Header *p, *prevp;
    unsigned nunits = (nbytes + sizeof(Header) + 1) / sizeof(Header) + 1;

    if ((prevp = freep) == NULL) {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) {
            if (p->s.size == nunits) {
                prevp->s.ptr = p->s.ptr;
            } else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*) (p+1);
        }
        if (p == freep && ((p = morecore(nunits)) == NULL)) {
            return NULL;
        }
    }
}