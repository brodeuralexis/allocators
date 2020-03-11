#include "allocators/fixed_buffer_allocator.h"

#include <string.h>

#include "./macros.h"

typedef struct node {
    bool is_hole;
    size_t size;
    struct node* previous;
} node_t;

static inline void* node_memory(node_t* node)
{
    return (void*) (node + 1);
}

static void* node_end(node_t* node)
{
    return (void*) ((char*) (node + 1) + node->size);
}

static inline node_t* node_first(fixed_buffer_allocator_t* allocator)
{
    return ((node_t*) allocator->buffer);
}

static node_t* node_next(fixed_buffer_allocator_t* allocator, node_t* node)
{
    node_t* next = (node_t*) node_end(node);

    if (((void*) next) < allocator->buffer || ((char*) next) >= ((char*) allocator->buffer) + allocator->size - sizeof(node_t))
    {
        return NULL;
    }

    return next;
}

static node_t* node_find(fixed_buffer_allocator_t* allocator, void* memory)
{
    node_t* node = node_first(allocator);

    while (node != NULL)
    {
        if (memory == node_memory(node))
        {
            break;
        }

        node = node_next(allocator,  node);
    }

    return node;
}

static void* node_reserve(node_t* node, size_t size)
{
    if (node->size - size > sizeof(node_t))
    {
        node_t* next = (node_t*) (((char*) node_memory(node)) + size);
        next->is_hole = true;
        next->size = node->size - size - sizeof(node_t);
        next->previous = node;
        node->size = size;
    }

    node->is_hole = false;

    return node_memory(node);
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
            if (node->is_hole && node->size >= size)
            {
                break;
            }

            node = node_next(allocator, node);
        }

        if (node == NULL)
        {
            return NULL;
        }

        return node_reserve(node, size);
    }
    else if (size == 0)
    {
        node_t* node = node_find(allocator, memory);

        if (node == NULL)
        {
            return NULL;
        }

        node_t* next = node_next(allocator, node);

        /// If the following node is a hole, ensure that it becomes part of this
        /// hole.  The combined node will be referenced by `node`.
        if (next && next->is_hole)
        {
            node->size += next->size + sizeof(node_t);
        }

        /// If the previous node is a hole, ensure that this node becomes part
        /// of it.  The combined node will be referenced by `node->previous`.
        if (node->previous && node->previous->is_hole)
        {
            node->previous->size += node->size + sizeof(node_t);
        }
        /// The previous node is not a hole.
        else
        {
            node->is_hole = true;
        }

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
