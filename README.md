# C Fixed Heap Allocator

C UNIX only fixed heap allocator for managing pre-allocated heap memory without kernel sbrk calls.
Managed memory is held as an anonymous memory map with custom `sbrk()` and `brk()` calls to the mapped area.

## Methods

| Signature                                                                   	| Description                                                                                                                                                                                                                                                                                                                                              	|
|-----------------------------------------------------------------------------	|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	|
| `int cfh_new(Allocator* alloc)`                                             	| Instantiates an allocated allocator with default values                                                                                                                                                                                                                                                                                                  	|
| `int cfh_init(Allocator* alloc, AllocationMethod method, size_t heap_size)` 	| Initialise memory mapped region for given heap size, utilising specified method for fragmentation resolution and allocation                                                                                                                                                                                                                              	|
| `int cfh_destruct(Allocator* alloc)`                                        	| Handled freeing of allocator with checking on heap state                                                                                                                                                                                                                                                                                                 	|
| `void* cfh_malloc(Allocator* alloc, unsigned nbytes)`                       	| Allocate memory from the mapped region for a given size                                                                                                                                                                                                                                                                                                  	|
| `void* cfh_calloc(Allocator* alloc, unsigned count, unsigned nbytes)`       	| Allocate contiguous memory from the mapped region for a given number of elements of given size                                                                                                                                                                                                                                                           	|
| `void cfh_free(Allocator* alloc, void* ap)`                                 	| Free the memory currently held by the provided pointer to a region of the mapped memory                                                                                                                                                                                                                                                                  	|
| `void* cfh_sbrk(Allocator* alloc, intptr_t increment)`                      	| Increments the data space by increment bytes. Calling `sbrk()` with an increment of 0 can be used to find the current location of the program break. On success, `sbrk()` returns the previous program break. (If the break was increased, then this value is a pointer to the start of the newly allocated memory). On error, `(void*) -1` is returned. 	|
| `void* cfh_brk(Allocator* alloc, void* addr)`                               	| Sets the end of the data segment to the value specified by `addr`, when that value is reasonable, the mapped region has enough memory, and the process does not exceed its maximum data size. On success, `brk()` returns `0`. On error, `-1` is returned.                                                                                               	|

## Usage

```c
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

    struct TestStruct* test_struct = cfh_malloc(alloc, sizeof(struct TestStruct));
    if (test_struct == NULL) {
        char *msg = calloc(100, sizeof(char));
        sprintf(msg, "Failed to allocate %lu bytes for struct TestStruct: ", sizeof(struct TestStruct));
        alloc_perror(msg);
        free(msg);
        return 1;
    }

    test_struct->value = 42;
    test_struct->str[0] = 'y';
    test_struct->str[1] = 'a';
    test_struct->str[2] = 'y';
    test_struct->str[3] = '!';

    printf("Test struct: [Value: %d] [Str: %s]", test_struct->value, test_struct->str);
    // Prints: Test struct: [Value: 42] [Str: yay!]

    cfh_free(alloc, test_struct);

    if (cfh_destruct(alloc) == -1) {
        alloc_perror("");
        return 1;
    }

    return 0;
}
```