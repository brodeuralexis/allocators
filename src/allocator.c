#include "allocators/allocator.h"

void* allocate(allocator_t* allocator, size_t size)
{
    return allocator->reallocate_fn(allocator, NULL, size);
}

void deallocate(allocator_t* allocator, void* memory)
{
    allocator->reallocate_fn(allocator, memory, 0);
}

void* reallocate(allocator_t* allocator, void* memory, size_t size)
{
    return allocator->reallocate_fn(allocator, memory, size);
}
