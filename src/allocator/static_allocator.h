#pragma once

#ifndef _H_C_FIXED_HEAP_ALLOCATOR_STATIC_ALLOCATOR_
#define _H_C_FIXED_HEAP_ALLOCATOR_STATIC_ALLOCATOR_

#include "allocator.h"
#include "../error/allocator_errno.h"

#ifdef STATIC_CFH

#ifndef STATIC_CFH_HEAP_SIZE
#define STATIC_CFH_HEAP_SIZE 100000
#endif

#ifndef STATIC_CFH_CONSTRUCTOR_PRIORITY
#define STATIC_CFH_CONSTRUCTOR_PRIORITY 0
#endif

#ifndef STATIC_CFH_DESTRUCTOR_PRIORITY
#define STATIC_CFH_DESTRUCTOR_PRIORITY 0
#endif

static Allocator* CFH_ALLOCATOR = NULL;

__attribute__((__constructor__(STATIC_CFH_CONSTRUCTOR_PRIORITY))) int __static_cfh_constructor() {
    CFH_ALLOCATOR = malloc(sizeof(*CFH_ALLOCATOR));
    if (cfh_new(CFH_ALLOCATOR) == -1) {
        alloc_perror("");
        return 1;
    }
    if (cfh_init(CFH_ALLOCATOR, FIRST_FIT, STATIC_CFH_HEAP_SIZE) == -1) {
        alloc_perror("Initialisation failed for heap :");
        return 1;
    }
    return 0;
}

__attribute__((__destructor__(STATIC_CFH_DESTRUCTOR_PRIORITY))) int __static_cfh_destructor() {
    if (cfh_destruct(CFH_ALLOCATOR) == -1) {
        alloc_perror("");
        return 1;
    }
    return 0;
}

#endif

#endif // _H_C_FIXED_HEAP_ALLOCATOR_STATIC_ALLOCATOR_