#include <stdio.h>
#include <string.h>

#include "allocators/direct_allocator.h"

#define ASTR_LEN 3
const char* STR = "Hello World!";

#define ARRAY_SIZE (4)
#define SUBARRAY_SIZE (1024)

int main()
{
    allocator_t* allocator = direct_allocator;

    double** array = create_array(allocator, double*, ARRAY_SIZE);

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        array[i] = create_array(allocator, double, SUBARRAY_SIZE);
    }

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
    {
        destroy_array(allocator, array[i]);
    }

    destroy_array(allocator, array);
}
