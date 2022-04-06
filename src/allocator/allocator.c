#include "allocator.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../error/allocator_errno.h"

#define __cfh_lock_lock_handled(lock) ({ \
    int lock_result = 0; \
    if (__cfh_lock_lock(lock) == EINVAL) { \
        set_alloc_errno_msg(MUTEX_LOCK_LOCK, strerror(EINVAL)); \
        lock_result = -1; \
    } \
    lock_result; \
})

#define __cfh_lock_unlock_handled(lock) ({ \
    int unlock_result = 0; \
    if ((unlock_result = __cfh_lock_unlock(lock)) != 0) { \
        set_alloc_errno_msg(MUTEX_LOCK_UNLOCK, strerror(unlock_result)); \
        unlock_result = -1; \
    } \
    unlock_result; \
})

int cfh_new(Allocator* alloc) {
    if (alloc == NULL) {
        set_alloc_errno(NULL_ALLOCATOR_INSTANCE);
        return -1;
    }
    int lock_init_result;
    if ((lock_init_result = __cfh_lock_init(&alloc->mutex, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        set_alloc_errno_msg(MUTEX_LOCK_INIT, strerror(lock_init_result));
        return -1;
    }
    alloc->heap_size = 0;
    alloc->freep = NULL;
    alloc->current_brk = NULL;
    alloc->heap = NULL;
    return 0;
}

int cfh_init(Allocator* alloc,
             AllocationMethod method,
             size_t heap_size) {
    if (alloc == NULL) {
        set_alloc_errno(NULL_ALLOCATOR_INSTANCE);
        return -1;
    }
    if (__cfh_lock_lock_handled(&alloc->mutex) != 0) {
        return -1;
    }
    if (alloc->heap != NULL) {
        set_alloc_errno(HEAP_ALREADY_MAPPED);
        __cfh_lock_unlock_handled(&alloc->mutex);
        return -1;
    }
    alloc->method = method;
    alloc->heap_size = heap_size;
    alloc->current_brk = alloc->heap = mmap(
        NULL,
        heap_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    if (alloc->heap == NULL) {
        set_alloc_errno(HEAP_MMAP_FAILED);
        __cfh_lock_unlock_handled(&alloc->mutex);
        return -1;
    }
    return __cfh_lock_unlock_handled(&alloc->mutex);
}

int cfh_destruct(Allocator* alloc) {
    if (alloc == NULL) {
        set_alloc_errno(BAD_DEALLOC);
        return -1;
    }
    if (__cfh_lock_lock_handled(&alloc->mutex) != 0) {
        return -1;
    }
    if (alloc->heap != NULL && munmap(alloc->heap, alloc->heap_size)) {
        set_alloc_errno(HEAP_UNMAP_FAILED);
        __cfh_lock_unlock_handled(&alloc->mutex);
        return -1;
    }
    if (__cfh_lock_unlock_handled(&alloc->mutex) != 0) {
        return -1;
    }
    free(alloc);
    return 0;
}

int cfh_brk(Allocator* alloc, void* addr) {
    if (addr > (alloc->heap + (uintptr_t) alloc->heap_size)) {
        return -1;
    }
    alloc->current_brk = addr;
    return 0;
}

void* cfh_sbrk(Allocator* alloc, intptr_t increment) {
    void* oldbrk;
    if (alloc->current_brk == NULL && cfh_brk(alloc, 0) < 0) {
        return (void*) -1;
    }
    if (increment == 0) {
        return alloc->current_brk;
    }
    oldbrk = alloc->current_brk;
    if (increment > 0
        ? ((uintptr_t) oldbrk + (uintptr_t) increment < (uintptr_t) oldbrk)
        : ((uintptr_t) oldbrk < (uintptr_t) -increment)){
        return (void*) -1;
    }
    if (cfh_brk(alloc, oldbrk + increment) < 0) {
        return (void*) -1;
    }
    return oldbrk;
}

void cfh_free(Allocator* alloc, void* ap) {
    Header* bp, *p;

    bp = (Header*) ap - 1;
    for (p = alloc->freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
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
    alloc->freep = p;
}

Header* more_core(Allocator* alloc, unsigned int nu) {
    char* cp;
    Header* up;

    if (nu < NALLOC) {
        nu = NALLOC;
    }
    cp = cfh_sbrk(alloc, nu * sizeof(Header));
    if (cp == (char*) - 1) {
        return NULL;
    }
    up = (Header*) cp;
    up->s.size = nu;
    cfh_free(alloc, (void*)(up + 1));
    return alloc->freep;
}

void* cfh_malloc(Allocator* alloc, unsigned nbytes) {
    if (__cfh_lock_lock_handled(&alloc->mutex) != 0) {
        return NULL;
    }
    Header *p, *prevp;
    unsigned nunits = (nbytes + sizeof(Header) + 1) / sizeof(Header) + 1;

    if ((prevp = alloc->freep) == NULL) {
        alloc->base.s.ptr = alloc->freep = prevp = &alloc->base;
        alloc->base.s.size = 0;
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
            alloc->freep = prevp;
            return __cfh_lock_unlock_handled(&alloc->mutex) == 0 ? (void*) (p+1) : NULL;
        }
        if (p == alloc->freep && ((p = more_core(alloc, nunits)) == NULL)) {
            set_alloc_errno(MALLOC_FAILED);
            __cfh_lock_unlock_handled(&alloc->mutex);
            return NULL;
        }
    }
}

void* cfh_calloc(Allocator* alloc, unsigned count, unsigned nbytes) {
    return NULL;
}

void* cfh_realloc(Allocator* alloc, void* ap, unsigned nbytes) {
    return NULL;
}