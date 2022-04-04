#include "allocator.h"
#include <stddef.h>
#include <unistd.h>

#define NALLOC 1024

static Header base;
static Header *freep = NULL;

void* __cfh_curbrk = 0;

int cfh_brk(void* addr) {
    void* newbrk;
    __cfh_curbrk = newbrk = (void*) ({
        unsigned long int resultvar = ({
            unsigned long int resultvar;
            __typeof__ ((addr) - (addr)) __arg1 = ((__typeof__ ((addr) - (addr))) (addr));
            register __typeof__ ((addr) - (addr)) _a1 asm ("rdi") = __arg1;
            asm volatile ( "syscall\n\t" : "=a" (resultvar) : "0" (12), "r" (_a1) : "memory", "cc", "r11", "cx");
            (long int) resultvar;
        });
        if (__builtin_expect ((((unsigned long int) (long int) (resultvar) >= -4095L)), 0)) {
            (rtld_errno = ((-(resultvar))));
            resultvar = (unsigned long int) -1;
        }
        (long int) resultvar;
    }) (brk, 1, addr);

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