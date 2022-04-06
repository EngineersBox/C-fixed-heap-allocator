#pragma once

#ifndef _H_C_FIXED_HEAP_ALLOCATOR_ERRNO_
#define _H_C_FIXED_HEAP_ALLOCATOR_ERRNO_

#include <stdio.h>
#include <string.h>

typedef enum allocator_error_num {
    NONE,
    NULL_ALLOCATOR_INSTANCE,
    HEAP_ALREADY_MAPPED,
    HEAP_MMAP_FAILED,
    HEAP_UNMAP_FAILED,
    BAD_DEALLOC,
    MALLOC_FAILED,
    MUTEX_LOCK_INIT,
    MUTEX_LOCK_LOCK,
    MUTEX_LOCK_UNLOCK,
    MUTEX_LOCK_DESTROY,
} AllocatorErrno;


#define MAX_PREFIX_LENGTH 1024
#define MAX_ERR_LINE_LENGTH 1024
#define MAX_ERR_STRING_LENGTH 2048

extern __thread int alloc_errno;
extern __thread char __alloc__errno_location[MAX_ERR_LINE_LENGTH];
extern __thread char __alloc__errno_msg[MAX_ERR_STRING_LENGTH];
extern __thread char __alloc__errno_strerr[MAX_ERR_LINE_LENGTH];

extern inline void get_alloc_errmsg(AllocatorErrno err);

#define set_alloc_errno(err) alloc_errno = err; sprintf(__alloc__errno_location, "%s(%s:%d)", __func__, __FILE__, __LINE__)
#define set_alloc_errno_msg(err, msg) sprintf(__alloc__errno_strerr, ": [%s]:", msg); set_alloc_errno(err)

#define alloc_perror(prefix) { \
    char trunc_prefix[MAX_PREFIX_LENGTH]; \
    size_t str_len = strlen(prefix); \
    if (str_len > MAX_PREFIX_LENGTH) { \
        str_len = MAX_PREFIX_LENGTH; \
    } \
    strncpy(trunc_prefix, prefix, str_len); \
    trunc_prefix[str_len] = 0; \
    get_alloc_errmsg(alloc_errno); \
    fprintf( \
        stderr, \
        "%s%s%s\n\tat %s(%s:%d)\n\tat %s\n", \
        trunc_prefix, \
        __alloc__errno_msg, \
        __alloc__errno_strerr, \
        __func__, __FILE__, __LINE__, \
        __alloc__errno_location \
    ); \
}

#endif // _H_C_FIXED_HEAP_ALLOCATOR_ERRNO_