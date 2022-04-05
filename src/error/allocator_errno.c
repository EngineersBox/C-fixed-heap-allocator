#include "allocator_errno.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__thread int alloc_errno = NONE;
__thread char __alloc__errno_location[MAX_ERR_LINE_LENGTH];
__thread char __alloc__errno_msg[MAX_ERR_STRING_LENGTH];

void get_alloc_errmsg(AllocatorErrno err) {
    switch (err) {
        case NULL_ALLOCATOR_INSTANCE: strcpy(__alloc__errno_msg, "Allocator is not initialised"); break;
        case HEAP_ALREADY_MAPPED: strcpy(__alloc__errno_msg, "Managed heap has already been allocated"); break;
        case NONE: strcpy(__alloc__errno_msg, ""); break;
    }
}

void alloc_perror(char* prefix) {
    char* trunc_prefix = calloc(MAX_PREFIX_LENGTH, sizeof(char));
    snprintf(trunc_prefix, MAX_PREFIX_LENGTH, "%s", prefix);
    get_alloc_errmsg(alloc_errno);
    fprintf(
        stderr,
        "%s%s\n\tat %s",
        trunc_prefix,
        __alloc__errno_msg,
        __alloc__errno_location
    );
    free(trunc_prefix);
}