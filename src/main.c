#include "allocator/allocator.h"
#include "error/allocator_errno.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    set_alloc_errno(NULL_ALLOCATOR_INSTANCE);
    alloc_perror("Test: ");
    return 0;
}