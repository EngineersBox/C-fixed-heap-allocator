#include "allocator/allocator.h"
#include "error/allocator_errno.h"

int main(int argc, char *argv[]) {
    Allocator* alloc;
    if (cfh_new(alloc) == -1) {
        alloc_perror("");
        return 1;
    }
    printf("Size: %zu\n", alloc->heap_size);
    printf("Heap addr: %p\n", alloc->heap);
    if (cfh_init(alloc, FIRST_FIT, 1000)) {
        alloc_perror("Initialisation failed for heap size 1000 bytes: ");
        return 1;
    }
    if (cfh_destruct(alloc) == -1) {
        alloc_perror("");
        return 1;
    }
    return 0;
}