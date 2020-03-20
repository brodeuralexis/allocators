#include "allocators/c_allocator.h"

#include <stdlib.h>

#include "./macros.h"

static void* c_allocator_reallocate_callback(allocator_t* allocator, void* memory, size_t size)
{
    UNUSED(allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    else if (memory == 0)
    {
        return malloc(size);
    }
    else if (size == 0)
    {
        free(memory);
        return NULL;
    }
    else
    {
        return realloc(memory, size);
    }
}

static allocator_t c_allocator_state = { c_allocator_reallocate_callback };
allocator_t* c_allocator = &c_allocator_state;
