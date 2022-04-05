#include <stdio.h>

typedef enum allocator_error_num {
    NONE,
    NULL_ALLOCATOR_INSTANCE,
    HEAP_ALREADY_MAPPED,
    HEAP_MMAP_FAILED,
    HEAP_UNMAP_FAILED,
    BAD_DEALLOC,
    MALLOC_FAILED,
} AllocatorErrno;


#define MAX_PREFIX_LENGTH 1024
#define MAX_ERR_LINE_LENGTH 1024
#define MAX_ERR_STRING_LENGTH 2048

extern __thread int alloc_errno;
extern __thread char __alloc__errno_location[MAX_ERR_LINE_LENGTH];
extern __thread char __alloc__errno_msg[MAX_ERR_STRING_LENGTH];

#define set_alloc_errno(err) alloc_errno = err; sprintf(__alloc__errno_location, "%s(%s:%d)", __func__, __FILE__, __LINE__)

__attribute__((__nothrow__)) void alloc_perror(char* prefix);