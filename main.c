#include <stdio.h>
#include <string.h>

#include "allocators/fixed_buffer_allocator.h"

char buffer[1000];

#define ARRAY_SIZE 10
#define SUBARRAY_SIZE 10

int main()
{
    fixed_buffer_allocator_t fba = fixed_buffer_allocator_init(FBS_FIRST_FIT, &buffer, sizeof(buffer));
    allocator_t* allocator = &fba.allocator;

    char* a1 = create_array(allocator, char, 100);
    char* a2 = create_array(allocator, char, 200);
    char* a3 = create_array(allocator, char, 300);

    fixed_buffer_allocator_debug(&fba, stderr);
    fprintf(stderr, "a1 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a1));
    fprintf(stderr, "a2 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a2));
    fprintf(stderr, "a3 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a3));
    fprintf(stderr, "\n\n");

    destroy_array(allocator, a2);

    fixed_buffer_allocator_debug(&fba, stderr);
    fprintf(stderr, "a1 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a1));
    fprintf(stderr, "a2 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a2));
    fprintf(stderr, "a3 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a3));
    fprintf(stderr, "\n\n");

    a2 = create_array(allocator, char, 1);

    fixed_buffer_allocator_debug(&fba, stderr);
    fprintf(stderr, "a1 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a1));
    fprintf(stderr, "a2 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a2));
    fprintf(stderr, "a3 = %zu\n", fixed_buffer_allocator_debug_memory(&fba, a3));
}
