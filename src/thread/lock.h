#pragma once

#ifndef _H_C_FIXED_HEAP_ALLOCATOR_THREAD_LOCK_
#define _H_C_FIXED_HEAP_ALLOCATOR_THREAD_LOCK_

#include <pthread.h>
#include <sys/errno.h>

#if!defined _LIBC
typedef pthread_mutex_t __cfh_lock_t;
typedef struct { pthread_mutex_t mutex; } __cfh_lock_recursive_t;
#else
typedef int __cfh_lock_t;
typedef struct { int lock; int cnt; void *owner; } __cfh_lock_recursive_t;
#endif

typedef pthread_key_t __cfh_key_t;
typedef pthread_mutexattr_t __cfh_mutexattr_t;

#define __cfh_lock_init(lock, type) ({ \
    int result = 0; \
    __cfh_mutexattr_t attr; \
    if ((result = pthread_mutexattr_init(&attr)) == 0) { \
        if ((result = pthread_mutexattr_settype(&attr, type)) == 0) { \
            result = pthread_mutex_init(lock, &attr) == EINVAL ? EINVAL : 0; \
        } \
    } \
    result; \
})
#define __cfh_lock_lock(lock) pthread_mutex_lock(lock)
#define __cfh_lock_unlock(lock) pthread_mutex_unlock(lock)

#endif // _H_C_FIXED_HEAP_ALLOCATOR_THREAD_LOCK_