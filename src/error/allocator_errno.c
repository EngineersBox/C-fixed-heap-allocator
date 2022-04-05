#include "allocator_errno.h"
#include <string.h>

__thread int alloc_errno = NONE;
__thread char __alloc__errno_location[MAX_ERR_LINE_LENGTH];
__thread char __alloc__errno_msg[MAX_ERR_STRING_LENGTH];

#define enum_error(enum_val, err_msg) case enum_val: strcpy(__alloc__errno_msg, err_msg); break;

inline void get_alloc_errmsg(AllocatorErrno err) {
    switch (err) {
        enum_error(NULL_ALLOCATOR_INSTANCE, "Allocator is not initialised")
        enum_error(HEAP_ALREADY_MAPPED, "Managed heap has already been allocated")
        enum_error(HEAP_MMAP_FAILED, "Failed to map memory for heap")
        enum_error(HEAP_UNMAP_FAILED, "Failed to unmap anonymous memory for heap")
        enum_error(BAD_DEALLOC, "Unable to destruct Allocator instance")
        enum_error(MALLOC_FAILED, "Unable to reserve memory")
        enum_error(NONE, "")
    }
}
