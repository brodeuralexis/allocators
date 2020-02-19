#ifndef __ALLOCATORS__ALLOCATOR__
#define __ALLOCATORS__ALLOCATOR__

#include <stdbool.h>
#include <stddef.h>

/**
 * A structure representing a virtual table for an allocator.
 *
 * All fields of an allocator are considered private.
 */
typedef struct allocator {
    bool __synchronized;
    void* (*__reallocate_fn)(struct allocator* allocator, void* memory, size_t size);
} allocator_t;

/**
 * Using the given allocator, allocates a block of `size` bytes of memory.
 * @param allocator An allocator
 * @param size A size in bytes
 * @return A pointer
 */
void* allocate(allocator_t* allocator, size_t size);

/**
 * Using the given allocator, deallocates a block of previously allocated memory.
 * @param allocator An allocator
 * @param memory A pointer to memory
 */
void deallocate(allocator_t* allocator, void* memory);

/**
 * Using the given allocator, reallocates a block of previously allocated memory to be of the new given size in bytes.
 * If the memory is NULL, an allocation is performed.  If the size is 0, the memory is deallocated.
 * @param allocator An allocator
 * @param memory A pointer to memory
 * @param size A size in bytes
 * @return The reallocated memory
 */
void* reallocate(allocator_t* allocator, void* memory, size_t size);

/**
 * Using the given allocator, creates new block of memory with at least enough bytes to hold the given type in memory.
 * @param _allocator An allocator
 * @param _Type The type
 * @return A pointer to memory
 */
#define create(_allocator, _Type) \
    ((_Type*) allocate((_allocator), sizeof(_Type)))

/**
 * Using the given allocator, creates a new block of memory with at least enough bytes to hold a specified amount of
 * the given type.
 * @param _allocator An allocator
 * @param _Type The type to allocate an array for
 * @param _size The number of elements to allocate
 * @return A pointer to memory
 */
#define create_array(_allocator, _Type, _size) \
    ((_Type*) allocate((_allocator), sizeof(_Type) * (_size)))

/**
 * Using the given allocator, destroys a pointer to memory previously allocated with the given allocator.
 * @param _allocator An allocator
 * @param _memory A pointer to memory
 */
#define destroy(_allocator, _memory) \
    deallocate((_allocator), (void*) (_memory))

/**
 * Using the given allocator, destroys a pointer to memory previously allocated with the given allocator using
 * `create_array/3`.
 * @param _allocator An allocator
 * @param _memory A pointer to memory
 */
#define destroy_array(_allocator, _memory) \
    deallocate((_allocator), (void*) (_memory))

#endif
