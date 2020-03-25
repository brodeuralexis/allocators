#include "allocators/fixed_buffer_allocator.h"

#include <assert.h>
#include <string.h>

#include "./macros.h"

void* fixed_buffer_node_memory(fixed_buffer_node_t* node)
{
    return (void*) (node + 1);
}

void* fixed_buffer_node_end(fixed_buffer_node_t* node)
{
    return (void*) ((char*) (node + 1) + node->size);
}

fixed_buffer_node_t* fixed_buffer_node_first(fixed_buffer_allocator_t* allocator)
{
    return ((fixed_buffer_node_t*) allocator->buffer);
}

fixed_buffer_node_t* fixed_buffer_node_next(fixed_buffer_allocator_t* allocator, fixed_buffer_node_t* node)
{
    fixed_buffer_node_t* next = (fixed_buffer_node_t*) fixed_buffer_node_end(node);

    // Ensures that the next node is still in the buffer owned by our allocator.
    if (((void*) next) < allocator->buffer || ((char*) next) >= ((char*) allocator->buffer) + allocator->size - sizeof(fixed_buffer_node_t))
    {
        return NULL;
    }

    return next;
}

fixed_buffer_node_t* fixed_buffer_node_find(fixed_buffer_allocator_t* allocator, void* memory)
{
    fixed_buffer_node_t* node = fixed_buffer_node_first(allocator);

    while (node != NULL)
    {
        // TODO(Alexis Brodeur): Consider allowing one to find a node when the memory is a pointer in the memory owned
        // by a node, and not only the start of the node's memory.
        if (memory == fixed_buffer_node_memory(node))
        {
            break;
        }

        node = fixed_buffer_node_next(allocator,  node);
    }

    return node;
}

/**
 * Reserves a node in the list, ensuring that the doubly linked list of nodes is kept valid.
 * @param allocator A fixed buffer allocator
 * @param node A node
 * @param size A size
 * @return The memory owned by the reserved node.
 */
static void* fixed_buffer_node_reserve(fixed_buffer_allocator_t* allocator, fixed_buffer_node_t* node, size_t size)
{
    UNUSED(allocator);

    assert(node != NULL);
    assert(node->is_hole);

    size_t remaining_size = node->size - size - sizeof(fixed_buffer_node_t);

    // If we were to create a new node after this one, do we have enough bytes left.
    // If not, this node does not change size.
    if (remaining_size > 0)
    {
        node->size = size;

        fixed_buffer_node_t* split = (fixed_buffer_node_t*) fixed_buffer_node_end(node);
        split->is_hole = true;
        split->size = remaining_size;
        split->previous = node;

        fixed_buffer_node_t* next = fixed_buffer_node_next(allocator, split);
        if (next != NULL)
        {
            next->previous = split;
        }
    }

    node->is_hole = false;
    allocator->last_node_reserved = node;

    return fixed_buffer_node_memory(node);
}

/**
 * Releases the node from use, updating neighboring nodes if the are also holes.
 * @param allocator A fixed buffer allocator
 * @param node A node to release
 */
static void fixed_buffer_node_release(fixed_buffer_allocator_t* allocator, fixed_buffer_node_t* node)
{
    assert(node != NULL);
    assert(!node->is_hole);

    fixed_buffer_node_t* previous = node->previous;
    fixed_buffer_node_t* next = fixed_buffer_node_next(allocator, node);

    // The previous node is a hole, make `node` a part of `previous` node.
    if (previous && previous->is_hole)
    {
        previous->size += node->size + sizeof(fixed_buffer_node_t);
        node = previous;

        if (next != NULL) {
            next->previous = node;
        }
    }

    // The next node is a hole, make it a part of `node`.
    if (next && next->is_hole)
    {
        fixed_buffer_node_t* after_next = fixed_buffer_node_next(allocator, next);

        // Update backward references.
        if (after_next != NULL)
        {
            after_next->previous = node;
        }

        node->size += next->size + sizeof(fixed_buffer_node_t);
    }

    // Override the tag to ensure that we have a hole in the buffer.
    node->is_hole = true;
}

struct fixed_buffer_strategy {
    allocator_t allocator;
};

static void* first_fit_reallocate(allocator_t* _allocator, void* memory, size_t size)
{
    fixed_buffer_allocator_t* allocator = FIELD_PARENT_PTR(fixed_buffer_allocator_t, allocator, _allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    // No memory, but a size given, do memory allocation.
    else if (memory == NULL)
    {
        fixed_buffer_node_t* node = fixed_buffer_node_first(allocator);

        while (node)
        {
            // We have a hole with enough bytes available.
            if (node->is_hole && node->size >= size)
            {
                break;
            }

            node = fixed_buffer_node_next(allocator, node);
        }

        // `node_next` returns NULL when traversal of the list finishes.
        // This check indicates that we have not found a suitable block of memory to allocate.
        if (node == NULL)
        {
            return NULL;
        }

        // We found a node, so we allocate a size out of it.
        return fixed_buffer_node_reserve(allocator, node, size);
    }
    // Memory is not null, do a free.
    else if (size == 0)
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        fixed_buffer_node_release(allocator, node);

        return NULL;
    }
    // We have a pointer to memory, and a size, do a resize.
    else
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        if (size <= node->size)
        {
            return memory;
        }

        void* new_memory = first_fit_reallocate(_allocator, NULL, size);

        if (new_memory == NULL)
        {
            return NULL;
        }

        memcpy(new_memory, memory, node->size);
        fixed_buffer_node_release(allocator, node);

        return new_memory;
    }
}

static fixed_buffer_strategy_t first_fit_strategy_state = {
    .allocator = { first_fit_reallocate },
};

fixed_buffer_strategy_t* FBS_FIRST_FIT = &first_fit_strategy_state;

static void* best_fit_reallocate(allocator_t* _allocator, void* memory, size_t size)
{
    fixed_buffer_allocator_t* allocator = FIELD_PARENT_PTR(fixed_buffer_allocator_t, allocator, _allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    // No memory, but a size given, do memory allocation.
    else if (memory == NULL)
    {
        fixed_buffer_node_t* best_fit = NULL;
        fixed_buffer_node_t* node = fixed_buffer_node_first(allocator);

        while (node)
        {
            // We have a hole with enough bytes available.
            if (node->is_hole && node->size >= size && (best_fit == NULL || node->size < best_fit->size))
            {
                best_fit = node;
            }

            node = fixed_buffer_node_next(allocator, node);
        }

        // `node_next` returns NULL when traversal of the list finishes.
        // This check indicates that we have not found a suitable block of memory to allocate.
        if (best_fit == NULL)
        {
            return NULL;
        }

        // We found a node, so we allocate a size out of it.
        return fixed_buffer_node_reserve(allocator, best_fit, size);
    }
    // Memory is not null, do a free.
    else if (size == 0)
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        fixed_buffer_node_release(allocator, node);

        return NULL;
    }
    // We have a pointer to memory, and a size, do a resize.
    else
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        if (size <= node->size)
        {
            return memory;
        }

        void* new_memory = best_fit_reallocate(_allocator, NULL, size);

        if (new_memory == NULL)
        {
            return NULL;
        }

        memcpy(new_memory, memory, node->size);
        fixed_buffer_node_release(allocator, node);

        return new_memory;
    }
}

static fixed_buffer_strategy_t best_fit_strategy_state = {
    .allocator = { best_fit_reallocate },
};

fixed_buffer_strategy_t* FBS_BEST_FIT = &best_fit_strategy_state;

static void* worst_fit_reallocate(allocator_t* _allocator, void* memory, size_t size)
{
    fixed_buffer_allocator_t* allocator = FIELD_PARENT_PTR(fixed_buffer_allocator_t, allocator, _allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    // No memory, but a size given, do memory allocation.
    else if (memory == NULL)
    {
        fixed_buffer_node_t* worst_fit = NULL;
        fixed_buffer_node_t* node = fixed_buffer_node_first(allocator);

        while (node)
        {
            // We have a hole with enough bytes available.
            if (node->is_hole && node->size >= size && (worst_fit == NULL || node->size > worst_fit->size))
            {
                worst_fit = node;
            }

            node = fixed_buffer_node_next(allocator, node);
        }

        // `node_next` returns NULL when traversal of the list finishes.
        // This check indicates that we have not found a suitable block of memory to allocate.
        if (worst_fit == NULL)
        {
            return NULL;
        }

        // We found a node, so we allocate a size out of it.
        return fixed_buffer_node_reserve(allocator, worst_fit, size);
    }
    // Memory is not null, do a free.
    else if (size == 0)
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        fixed_buffer_node_release(allocator, node);

        return NULL;
    }
    // We have a pointer to memory, and a size, do a resize.
    else
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        if (size <= node->size)
        {
            return memory;
        }

        void* new_memory = worst_fit_reallocate(_allocator, NULL, size);

        if (new_memory == NULL)
        {
            return NULL;
        }

        memcpy(new_memory, memory, node->size);
        fixed_buffer_node_release(allocator, node);

        return new_memory;
    }
}

static fixed_buffer_strategy_t worst_fit_strategy_state = {
    .allocator = { worst_fit_reallocate },
};

fixed_buffer_strategy_t* FBS_WORST_FIT = &worst_fit_strategy_state;

static void* next_fit_reallocate(allocator_t* _allocator, void* memory, size_t size)
{
    fixed_buffer_allocator_t* allocator = FIELD_PARENT_PTR(fixed_buffer_allocator_t, allocator, _allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    // No memory, but a size given, do memory allocation.
    else if (memory == NULL)
    {
        fixed_buffer_node_t* node = allocator->last_node_reserved;

        // We take the next node after previous allocation
        if (node != NULL)
        {
            node = fixed_buffer_node_next(allocator, node);
        }

        if (allocator->last_node_reserved == NULL)
        {
            allocator->last_node_reserved = fixed_buffer_node_first(allocator);
        }

        while (node != allocator->last_node_reserved)
        {
            // We have a hole with enough bytes available.
            if (node->is_hole && node->size >= size)
            {
                break;
            }

            node = fixed_buffer_node_next(allocator, node);

            if (node == NULL)
            {
                node = fixed_buffer_node_first(allocator);
            }
        }

        // `node_next` returns NULL when traversal of the list finishes.
        // This check indicates that we have not found a suitable block of memory to allocate.
        if (node == NULL)
        {
            return NULL;
        }

        // We found a node, so we allocate a size out of it.
        return fixed_buffer_node_reserve(allocator, node, size);
    }
    // Memory is not null, do a free.
    else if (size == 0)
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        fixed_buffer_node_release(allocator, node);

        return NULL;
    }
    // We have a pointer to memory, and a size, do a resize.
    else
    {
        fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        if (size <= node->size)
        {
            return memory;
        }

        void* new_memory = first_fit_reallocate(_allocator, NULL, size);

        if (new_memory == NULL)
        {
            return NULL;
        }

        memcpy(new_memory, memory, node->size);
        fixed_buffer_node_release(allocator, node);

        return new_memory;
    }
}

static fixed_buffer_strategy_t next_fit_strategy_state = {
    .allocator = { next_fit_reallocate },
};

fixed_buffer_strategy_t* FBS_NEXT_FIT = &next_fit_strategy_state;

fixed_buffer_allocator_t fixed_buffer_allocator_init(fixed_buffer_strategy_t* strategy, void* buffer, size_t size)
{
    if (strategy == NULL)
    {
        strategy = FBS_FIRST_FIT;
    }

    fixed_buffer_allocator_t allocator;
    allocator.allocator = strategy->allocator;
    allocator.buffer = buffer;
    allocator.size = size;

    fixed_buffer_node_t* node = (fixed_buffer_node_t*) buffer;
    node->is_hole = true;
    node->size = size - sizeof(fixed_buffer_node_t);
    node->previous = NULL;

    return allocator;
}

void fixed_buffer_allocator_set_strategy(fixed_buffer_allocator_t* allocator, fixed_buffer_strategy_t* strategy)
{
    allocator->allocator = strategy->allocator;
}

void fixed_buffer_allocator_debug(fixed_buffer_allocator_t* allocator, FILE* file)
{
    int i = 0;
    fixed_buffer_node_t* node = fixed_buffer_node_first(allocator);

    while (node != NULL)
    {
        fprintf(file, "[BLOCK %d] - %s (%zu bytes)\n", i, node->is_hole ? "free" : "used", 24 + node->size);
        fprintf(file, "\theader (%d bytes) \t@ %zu\n", 24, (size_t)(((char*)node) - ((char*)allocator->buffer)));
        fprintf(file, "\tmemory (%zu bytes) \t@ %zu\n\n", node->size, (size_t)(((char*)fixed_buffer_node_memory(node)) - ((char*)allocator->buffer)));

        node = fixed_buffer_node_next(allocator, node);
        ++i;
    }
}

size_t fixed_buffer_allocator_debug_memory(fixed_buffer_allocator_t* allocator, void* memory)
{
    fixed_buffer_node_t* node = fixed_buffer_node_find(allocator, memory);

    if (node == NULL)
    {
        return -1;
    }
    else
    {
        return (size_t) (((char*) memory) - ((char*) allocator->buffer));
    }
}
