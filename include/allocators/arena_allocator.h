#ifndef __ALLOCATORS__ARENA_ALLOCATOR__
#define __ALLOCATORS__ARENA_ALLOCATOR__

#include "allocators/allocator.h"

/**
 * Creates an arena allocator using the backing allocator to do the real memory allocation.
 * @param backing_allocator The backing allocator that will be called for memory management
 * @return An arena allocator
 */
allocator_t* arena_allocator_create(allocator_t* backing_allocator);

/**
 * Destroys an arena allocator.
 * @param arena_allocator An arena allocator
 */
void arena_allocator_destroy(allocator_t* arena_allocator);

#endif
