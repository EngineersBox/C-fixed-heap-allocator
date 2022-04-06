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
| `void* cfh_realloc(Allocator* alloc, void* ap, unsigned nbytes)`            	| Re-size a given block of memory to a new size, that was previously allocated by `cfh_malloc` or `cfh_calloc`.                                                                                                                                                                                                                                            	|
| `void cfh_free(Allocator* alloc, void* ap)`                                 	| Free the memory currently held by the provided pointer to a region of the mapped memory                                                                                                                                                                                                                                                                  	|
| `void* cfh_sbrk(Allocator* alloc, intptr_t increment)`                      	| Increments the data space by increment bytes. Calling `sbrk()` with an increment of 0 can be used to find the current location of the program break. On success, `sbrk()` returns the previous program break. (If the break was increased, then this value is a pointer to the start of the newly allocated memory). On error, `(void*) -1` is returned. 	|
| `void* cfh_brk(Allocator* alloc, void* addr)`                               	| Sets the end of the data segment to the value specified by `addr`, when that value is reasonable, the mapped region has enough memory, and the process does not exceed its maximum data size. On success, `brk()` returns `0`. On error, `-1` is returned.                                                                                               	|

## Error Handling

Errors are handled in the same manner as standard usage of `perror(char* prefix)` follows, except with a custom method `alloc_perror(char* prefix)`.
Any call made with the CFH methods, should follow with a return value check, in the case on an invalid/erroneous value, `alloc_perror(char* prefix)` should be called to display the error, file, line number and function name in stderr.

An example of an error being logged as a result of `alloc_perror("An error occured: ")` is as follows:

```
An error occured: Managed heap has already been allocated
	at main(/Users/EngineersBox/Desktop/Projects/C:C++/C-fixed-heap-allocator/src/main.c:17)
	at cfh_init(/Users/EngineersBox/Desktop/Projects/C:C++/C-fixed-heap-allocator/src/allocator/allocator.c:27)
```

These locations correspond the following:

### Main.c:17

```c
// ... snip ...
if (cfh_init(alloc, FIRST_FIT, 100000) == -1) {
    alloc_perror("An error occured: ");
    return 1;
}
// ... snip ...
```

### Allocator.c:27

```c
// ... snip ...
} else if (alloc->heap != NULL) {
    set_alloc_errno(HEAP_ALREADY_MAPPED);
    return -1;
}
// ... snip ...
```

## Usage

There are two ways to utilise the allocator, one is through handled static construction and dynamic instantiation.

## Static Automatic

Using the static approach will instantiate a static allocator for you before the `main()` entry point is run and also
free itself after `main()` has finished.

In order to use this method, you will need to include the `static_allocator.h` header in your main and supply a set of
compilation definitions for configuration.

This will make the `CFH_ALLOCATOR` static allocator available for you to access and allocate to.

### Compilation Definitions

| Definition                                	| Required 	| Default  	     | Description                                                                                                                                                                       	|
|-------------------------------------------	|----------	|----------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	|
| `-DSTATIC_CFH`                            	| `true`   	| `false`  	     | Enable the static allocator                                                                                                                                                       	|
| `-DSTATIC_CFH_HEAP_SIZE=<bytes>`          	| `false`  	| `100000` 	     | Configure the byte size of the heap for the allocator                                                                                                                             	|
| `-DSTATIC_CFH_CONSTRUCTOR_PRIORITY=<int>` 	| `false`  	| `Not set`    	 | Set the priority of the constructor method for initialising the allocator, this is for compatibility with other constructors using `__attribute__((__constructor__(<priority>)))` 	|
| `-DSTATIC_CFH_DESTRUCTOR_PRIORITY=<int>`  	| `false`  	| `Not set`    	 | Set the priority of the destructor method for initialising the allocator, this is for compatibility with other destructors using `__attribute__((__destructor__(<priority>)))`    	|

An example is as follows, compiled with the following options:

```shell
-DSTATIC_CFH
-DSTATIC_CFH_HEAP_SIZE=200000
-DSTATIC_CFH_DESTRUCTOR_PRIORITY=2
```

```c
#include <stdlib.h>
#include "allocator/static_allocator.h"
#include "error/allocator_errno.h"

struct TestStruct {
    int value;
    char str[4];
};

int main(int argc, char *argv[]) {
    struct TestStruct* test_struct = cfh_malloc(CFH_ALLOCATOR, sizeof(*test_struct));
    if (test_struct == NULL) {
        char *msg = calloc(100, sizeof(*msg));
        sprintf(msg, "Failed to allocate %lu bytes for struct TestStruct: ", sizeof(*test_struct));
        alloc_perror(msg);
        free(msg);
        return 1;
    }

    test_struct->value = 42;
    test_struct->str[0] = 'y';
    test_struct->str[1] = 'a';
    test_struct->str[2] = 'y';
    test_struct->str[3] = '!';

    printf("Test struct:   [Value: %d] [Str: %s]\n", test_struct->value, test_struct->str);
    // Prints: Test struct: [Value: 42] [Str: yay!]

    cfh_free(CFH_ALLOCATOR, test_struct);

    struct TestStruct* test_struct2 = cfh_malloc(CFH_ALLOCATOR, sizeof(*test_struct2));
    if (test_struct2 == NULL) {
        char *msg = calloc(100, sizeof(*msg));
        sprintf(msg, "Failed to allocate %lu bytes for struct TestStruct: ", sizeof(*test_struct2));
        alloc_perror(msg);
        free(msg);
        return 1;
    }

    test_struct2->value = 84;
    test_struct2->str[0] = 'd';
    test_struct2->str[1] = 'o';
    test_struct2->str[2] = 'n';
    test_struct2->str[3] = 'e';

    printf("Test Struct 2: [Value: %d] [Str: %s]\n", test_struct2->value, test_struct2->str);
    // Prints: Test struct 2: [Value: 84] [Str: done]

    cfh_free(CFH_ALLOCATOR, test_struct2);

    printf("Deallocated memory from Test Struct: %d", test_struct->value);
    // Prints: Deallocated memory from Test Struct: 84

    return 0;
}
```

## Dynamic

The second use case is dynamic allocator initialisation and free handling. This allows you to create as many allocators as you need and handle them yourself.

Note that you can use the dynamic method alongside static allocation without overlap, however you will still need to handle the allocation and freeing of the allocators that are dynamic yourself.

```c
#include <stdlib.h>
#include "allocator/allocator.h"
#include "error/allocator_errno.h"

struct TestStruct {
    int value;
    char str[4];
};

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
        char *msg = calloc(100, sizeof(*msg));
        sprintf(msg, "Failed to allocate %lu bytes for struct TestStruct: ", sizeof(*test_struct));
        alloc_perror(msg);
        free(msg);
        return 1;
    }

    test_struct->value = 42;
    test_struct->str[0] = 'y';
    test_struct->str[1] = 'a';
    test_struct->str[2] = 'y';
    test_struct->str[3] = '!';

    printf("Test struct:   [Value: %d] [Str: %s]\n", test_struct->value, test_struct->str);
    // Prints: Test struct: [Value: 42] [Str: yay!]
    
    cfh_free(alloc, test_struct);

    struct TestStruct* test_struct2 = cfh_malloc(alloc, sizeof(*test_struct2));
    if (test_struct2 == NULL) {
        char *msg = calloc(100, sizeof(*msg));
        sprintf(msg, "Failed to allocate %lu bytes for struct TestStruct: ", sizeof(*test_struct2));
        alloc_perror(msg);
        free(msg);
        return 1;
    }

    test_struct2->value = 84;
    test_struct2->str[0] = 'd';
    test_struct2->str[1] = 'o';
    test_struct2->str[2] = 'n';
    test_struct2->str[3] = 'e';

    printf("Test Struct 2: [Value: %d] [Str: %s]\n", test_struct2->value, test_struct2->str);
    // Prints: Test struct 2: [Value: 84] [Str: done]
    
    cfh_free(alloc, test_struct2);

    printf("Deallocated memory from Test Struct: %d", test_struct->value);
    // Prints: Deallocated memory from Test Struct: 84
    
    if (cfh_destruct(alloc) == -1) {
        alloc_perror("");
        return 1;
    }

    return 0;
}
```

## TODO

* Implement `void* cfh_calloc(Allocator* alloc, unsigned count, unsigned nbytes)`
* Implement `void* cfh_realloc(Allocator* alloc, void* ap, unsigned nbytes)`
* Refactor to change allocation method based on the one provided in `cfh_init(...)`
