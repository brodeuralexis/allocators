#ifndef __ALLOCATORS__SYNCHRONIZED_ALLOCATOR__
#define __ALLOCATORS__SYNCHRONIZED_ALLOCATOR__

#include "allocators/allocator.h"

/**
 * Creates a synchronized allocator, making its child allocator thread safe by synchronizing allocations with a mutex.
 *
 * If the provided child allocator is already synchronized, this function is a noop.
 * @param child_allocator The allocator to synchronize
 * @return A synchronized allocator
 */
allocator_t* synchronized_allocator_create(allocator_t* child_allocator);

/**
 * Destroys the given synchronized allocator
 *
 * It may happen that the given allocator is not a synchronized allocator during cleanup, if so, this function is a
 * noop.
 * @param synchronized_allocator A synchronized allocator
 */
void synchronized_allocator_destroy(allocator_t* synchronized_allocator);

#endif
