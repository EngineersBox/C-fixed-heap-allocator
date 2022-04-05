#include <stdlib.h>
#include "allocator/allocator.h"
#include "error/allocator_errno.h"

struct TestStruct {
    int value;
    char str[4];
};

int main(int argc, char *argv[]) {
    Allocator* alloc = malloc(sizeof(Allocator));
    if (cfh_new(alloc) == -1) {
        alloc_perror("");
        return 1;
    }
    if (cfh_init(alloc, FIRST_FIT, 100000) == -1) {
        alloc_perror("Initialisation failed for heap size 100000 bytes: ");
        return 1;
    }
    if (cfh_destruct(alloc) == -1) {
        alloc_perror("");
        return 1;
    }

    struct TestStruct* test_struct = cfh_malloc(alloc, sizeof(struct TestStruct));
    if (test_struct == NULL) {
        char *msg = calloc(100, sizeof(char));
        sprintf(msg, "Failed to allocate %lu bytes for struct TestStruct: ", sizeof(struct TestStruct));
        alloc_perror(msg);
        free(msg);
        return 1;
    }

    return 0;
}