#ifndef __ALLOCATORS__FIXED_BUFFER_ALLOCATOR__
#define __ALLOCATORS__FIXED_BUFFER_ALLOCATOR__

#include "allocators/allocator.h"

#include <stdio.h>

typedef struct fixed_buffer_strategy fixed_buffer_strategy_t;
extern fixed_buffer_strategy_t* FBS_FIRST_FIT;
extern fixed_buffer_strategy_t* FBS_BEST_FIT;
extern fixed_buffer_strategy_t* FBS_WORST_FIT;
extern fixed_buffer_strategy_t* FBS_NEXT_FIT;

typedef struct {
    allocator_t allocator;
    void* buffer;
    size_t size;
} fixed_buffer_allocator_t;

fixed_buffer_allocator_t fixed_buffer_allocator_init(fixed_buffer_strategy_t* strategy, void* buffer, size_t size);

/**
 * A node in the allocation list of a fixed buffer allocator.
 */
typedef struct fixed_buffer_node {
    /**
     * Indicates if this node is a hole.
     */
    bool is_hole;
    /**
     * The size of the memory following this node.
     */
    size_t size;
    /**
     * A pointer to the previous node in memory.
     */
    struct fixed_buffer_node* previous;
} fixed_buffer_node_t;

/**
 * Returns a pointer to the memory associated with this node.
 * @param node A node
 * @return A pointer
 */
void* fixed_buffer_node_memory(fixed_buffer_node_t* node);

/**
 * Returns a pointer to the end of this node and its associated memory.
 * @param node A node
 * @return A pointer
 */
void* fixed_buffer_node_end(fixed_buffer_node_t* node);

/**
 * Returns the first node from an allocator
 * @param allocator A fixed buffer allocator
 * @return A node
 */
fixed_buffer_node_t* fixed_buffer_node_first(fixed_buffer_allocator_t* allocator);

/**
 * Returns the node following another node in the list of nodes associated with an allocator.
 *
 * If the following node would be out of the allocator's buffer, `NULL` is returned.
 * @param allocator A fixed buffer allocator
 * @param node A node
 * @return The following node
 */
fixed_buffer_node_t* fixed_buffer_node_next(fixed_buffer_allocator_t* allocator, fixed_buffer_node_t* node);

/**
 * Finds a node for the given memory location.
 *
 * If the memory is not managed by this allocator, `NULL` is returned.
 * @param allocator A fixed buffer allocator
 * @param memory A pointer
 * @return A node
 */
fixed_buffer_node_t* fixed_buffer_node_find(fixed_buffer_allocator_t* allocator, void* memory);

/**
 * Prints debug information associated with a fixed buffer allocator.
 * @param allocator An allocator
 * @param file A file handle
 */
void fixed_buffer_allocator_debug(fixed_buffer_allocator_t* allocator, FILE* file);

/**
 * Returns the memory offset of the given pointer.
 * @param allocator A fixed buffer allocator
 * @param memory A pointer
 * @return An offset
 */
size_t fixed_buffer_allocator_debug_memory(fixed_buffer_allocator_t* allocator, void* memory);

#endif
