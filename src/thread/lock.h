#pragma once

#ifndef _H_C_FIXED_HEAP_ALLOCATOR_THREAD_LOCK_
#define _H_C_FIXED_HEAP_ALLOCATOR_THREAD_LOCK_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/errno.h>

typedef pthread_mutex_t __cfh_lock_t;

#define __cfh_lock_init(lock, type) ({ \
    int result = 0; \
    pthread_mutexattr_t attr; \
    if ((result = pthread_mutexattr_init(&attr)) == 0) { \
        if ((result = pthread_mutexattr_settype(&attr, type)) == 0) { \
            if ((result = pthread_mutex_init(lock, &attr)) == 0) { \
                result = pthread_mutexattr_destroy(&attr) == EINVAL ? EINVAL : 0; \
            } \
        } \
    } \
    result; \
})

#define __cfh_lock_lock(lock) pthread_mutex_lock(lock)
#define __cfh_lock_unlock(lock) pthread_mutex_unlock(lock)

#ifdef __cplusplus
}
#endif

#endif // _H_C_FIXED_HEAP_ALLOCATOR_THREAD_LOCK_