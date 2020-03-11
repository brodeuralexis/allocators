#include "allocators/fixed_buffer_allocator.h"

#include <assert.h>
#include <string.h>

#include "./macros.h"

/**
 * A node in the allocation list of a fixed buffer allocator.
 */
typedef struct node {
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
    struct node* previous;
} node_t;

/**
 * Returns a pointer to the memory associated with this node.
 * @param node A node
 * @return A pointer
 */
static inline void* node_memory(node_t* node)
{
    return (void*) (node + 1);
}

/**
 * Returns a pointer to the end of this node and its associated memory.
 * @param node A node
 * @return A pointer
 */
static void* node_end(node_t* node)
{
    return (void*) ((char*) (node + 1) + node->size);
}

/**
 * Returns the first node from an allocator
 * @param allocator A fixed buffer allocator
 * @return A node
 */
static inline node_t* node_first(fixed_buffer_allocator_t* allocator)
{
    return ((node_t*) allocator->buffer);
}

/**
 * Returns the node following another node in the list of nodes associated with an allocator.
 *
 * If the following node would be out of the allocator's buffer, `NULL` is returned.
 * @param allocator A fixed buffer allocator
 * @param node A node
 * @return The following node
 */
static node_t* node_next(fixed_buffer_allocator_t* allocator, node_t* node)
{
    node_t* next = (node_t*) node_end(node);

    // Ensures that the next node is still in the buffer owned by our allocator.
    if (((void*) next) < allocator->buffer || ((char*) next) >= ((char*) allocator->buffer) + allocator->size - sizeof(node_t))
    {
        return NULL;
    }

    return next;
}

/**
 * Finds a node for the given memory location.
 *
 * If the memory is not managed by this allocator, `NULL` is returned.
 * @param allocator A fixed buffer allocator
 * @param memory A pointer
 * @return A node
 */
static node_t* node_find(fixed_buffer_allocator_t* allocator, void* memory)
{
    node_t* node = node_first(allocator);

    while (node != NULL)
    {
        // TODO(Alexis Brodeur): Consider allowing one to find a node when the memory is a pointer in the memory owned
        // by a node, and not only the start of the node's memory.
        if (memory == node_memory(node))
        {
            break;
        }

        node = node_next(allocator,  node);
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
static void* node_reserve(fixed_buffer_allocator_t* allocator, node_t* node, size_t size)
{
    UNUSED(allocator);

    assert(node != NULL);
    assert(node->is_hole);

    size_t remaining_size = node->size - size - sizeof(node_t);

    // If we were to create a new node after this one, do we have enough bytes left.
    // If not, this node does not change size.
    if (remaining_size > 0)
    {
        node->size = size;

        node_t* split = (node_t*) node_end(node);
        split->is_hole = true;
        split->size = remaining_size;
        split->previous = node;

        node_t* next = node_next(allocator, node);
        if (next != NULL)
        {
            next->previous = split;
        }
    }

    node->is_hole = false;

    return node_memory(node);
}

/**
 * Releases the node from use, updating neighboring nodes if the are also holes.
 * @param allocator A fixed buffer allocator
 * @param node A node to release
 */
static void node_release(fixed_buffer_allocator_t* allocator, node_t* node)
{
    assert(node != NULL);
    assert(!node->is_hole);

    node_t* previous = node->previous;
    node_t* next = node_next(allocator, node);

    // The previous node is a hole, make `node` a part of `previous` node.
    if (previous && previous->is_hole)
    {
        previous->size += node->size + sizeof(node_t);
        node = previous;
    }

    // The next node is a hole, make it a part of `node`.
    if (next && next->is_hole)
    {
        node_t* after_next = node_next(allocator, next);

        // Update backward references.
        if (after_next != NULL)
        {
            after_next->previous = node;
        }

        node->size += next->size + sizeof(node_t);
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
    else if (memory == NULL)
    {
        node_t* node = node_first(allocator);

        while (node)
        {
            // We have a hole with enough bytes available.
            if (node->is_hole && node->size >= size)
            {
                break;
            }

            node = node_next(allocator, node);
        }

        // `node_next` returns NULL when traversal of the list finishes.
        // This check indicates that we have not found a suitable block of memory to allocate.
        if (node == NULL)
        {
            return NULL;
        }

        // We found a node, so we allocate a size out of it.
        return node_reserve(allocator, node, size);
    }
    else if (size == 0)
    {
        node_t* node = node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        node_release(allocator, node);

        return NULL;
    }
    else
    {
        node_t* node = node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        if (size <= node->size)
        {
            return node;
        }

        void* new_memory = first_fit_reallocate(_allocator, NULL, size);
        memcpy(new_memory, memory, node->size);
        return new_memory;
    }
}

static fixed_buffer_strategy_t first_fit_strategy_state = {
    .allocator = {false, first_fit_reallocate},
};

fixed_buffer_strategy_t* FBS_FIRST_FIT = &first_fit_strategy_state;

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

    node_t* node = (node_t*) buffer;
    node->is_hole = true;
    node->size = size - sizeof(node_t);
    node->previous = NULL;

    return allocator;
}

void fixed_buffer_allocator_debug(fixed_buffer_allocator_t* allocator, FILE* file)
{
    node_t* node = node_first(allocator);

    fprintf(file, "[ (sizeof(node) == %zu)\n", sizeof(node_t));

    while (node != NULL)
    {
        fprintf(file, "\t%s @ %zu {\n", node->is_hole ? "hole" : "used", (size_t) (((char*) node) - ((char*) allocator->buffer)));

        fprintf(file, "\t\tsize   = %zu\n", node->size);
        fprintf(file, "\t\tmemory = @ %zu\n", (size_t) (((char*) node_memory(node)) - ((char*) allocator->buffer)));

        fprintf(file, "\t}\n");

        node = node_next(allocator, node);
    }

    fprintf(file, "]\n");
}

size_t fixed_buffer_allocator_debug_memory(fixed_buffer_allocator_t* allocator, void* memory)
{
    node_t* node = node_find(allocator, memory);

    if (node == NULL)
    {
        return -1;
    }
    else
    {
        return (size_t) (((char*) memory) - ((char*) allocator->buffer));
    }
}
