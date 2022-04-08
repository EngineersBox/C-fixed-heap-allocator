#include <stdlib.h>
#include "allocator/allocator.h"
#include "error/allocator_errno.h"

struct TestStruct {
    int value;
    char str[4];
};

#define print_error(subs, bytes) \
    char *msg = calloc(100, sizeof(*msg)); \
    sprintf(msg, subs, bytes); \
    alloc_perror(msg); \
    free(msg); \
    return 1

int main(int argc, char *argv[]) {
    Allocator* alloc = malloc(sizeof(*alloc));
    if (cfh_new(alloc) == -1) {
        alloc_perror("");
        return 1;
    }
    if (cfh_init(alloc, FIRST_FIT, 100000) == -1) {
        alloc_perror("Initialisation failed for heap size 100000 bytes: ");
        return 1;
    }

    struct TestStruct* test_struct = cfh_malloc(alloc, sizeof(*test_struct));
    if (test_struct == NULL) {
        print_error("Failed to allocate %lu bytes: ", sizeof(*test_struct));
    }

    test_struct->value = 42;
    test_struct->str[0] = 'y';
    test_struct->str[1] = 'a';
    test_struct->str[2] = 'y';
    test_struct->str[3] = '!';

    printf("Test struct:   [Value: %d] [Str: %s]\n", test_struct->value, test_struct->str);
    if (cfh_free(alloc, test_struct) != 0) {
        print_error("Failed to deallocate %p", test_struct);
    }

    char* string = cfh_calloc(alloc, 5, sizeof(*string));
    if (string == NULL) {
        print_error("Failed to allocate %lu bytes: ", 5 * sizeof(*string));
    }

    string[0] = 'T';
    string[1] = 'e';
    string[2] = 's';
    string[3] = 't';
    string[4] = '\0';

    printf("String values: %s\n", string);

    if (cfh_free(alloc, string) != 0) {
        print_error("Failed to deallocate %p", string);
    }

    if (cfh_destruct(alloc) == -1) {
        alloc_perror("");
        return 1;
    }

    return 0;
}