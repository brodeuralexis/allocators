#include <stdio.h>
#include <string.h>

#include "allocators/c_allocator.h"
#include "allocators/arena_allocator.h"

#define ASTR_LEN 3
const char* STR = "Hello World!";

int main()
{
    allocator_t* allocator = arena_allocator_create(c_allocator);

    if (allocator == NULL)
    {
        fprintf(stderr, "Could not create arena allocator\n");
        return 1;
    }

    char** astr = create_array(allocator, char*, ASTR_LEN);
    for (size_t i = 0; i < ASTR_LEN; ++i)
    {
        astr[i] = create_array(allocator, char, strlen(STR) + 1);
        strcpy(astr[i], STR);
    }

    for (size_t i = 0; i < ASTR_LEN; ++i)
    {
        printf("astr[%zu] = %s\n", i, astr[i]);
        destroy_array(allocator, astr[i]);
    }

    destroy_array(allocator, astr);

    arena_allocator_destroy(allocator);
}
