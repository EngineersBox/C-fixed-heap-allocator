#include "allocator_errno.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__thread int alloc_errno = NONE;
__thread char __alloc__errno_location[MAX_ERR_LINE_LENGTH];
__thread char __alloc__errno_msg[MAX_ERR_STRING_LENGTH];

#define enum_error(enum_val, err_msg) case enum_val: strcpy(__alloc__errno_msg, err_msg); break;

void get_alloc_errmsg(AllocatorErrno err) {
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

void alloc_perror(char* prefix) {
    char* trunc_prefix = calloc(MAX_PREFIX_LENGTH, sizeof(char));
    snprintf(trunc_prefix, MAX_PREFIX_LENGTH, "%s", prefix);
    get_alloc_errmsg(alloc_errno);
    fprintf(
        stderr,
        "%s%s\n\tat %s\n",
        trunc_prefix,
        __alloc__errno_msg,
        __alloc__errno_location
    );
    free(trunc_prefix);
}