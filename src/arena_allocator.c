#include "allocators/arena_allocator.h"

#include <string.h>

#include "./macros.h"

// A node in an arena allocator's singly linked list of blocks of memory.
typedef struct node {
    struct node* tail;
    size_t size;
} node_t;

// Finds a node that points to the given block of memory from a root node.
static node_t* find_node(node_t* node, void* memory)
{
    while (node != NULL)
    {
        if ((void*) (node + 1) == memory)
        {
            break;
        }

        node = node->tail;
    }

    return node;
}

typedef struct {
    allocator_t allocator;
    allocator_t* backing_allocator;
    node_t* head;
} arena_allocator_t;

static void* arena_allocator_allocate(arena_allocator_t* arena_allocator, size_t size)
{
    node_t* node = (node_t*) allocate(arena_allocator->backing_allocator, sizeof(node_t) + size);

    if (node == NULL)
    {
        return NULL;
    }

    node->size = size;
    node->tail = arena_allocator->head;
    arena_allocator->head = node;

    return (void*) (node + 1);
}

static void* arena_allocator_reallocate(arena_allocator_t* arena_allocator, void* memory, size_t size)
{
    node_t* existing_node = find_node(arena_allocator->head, memory);

    if (existing_node == NULL)
    {
        return NULL;
    }

    if (size <= existing_node->size)
    {
        return memory;
    }

    node_t* node = (node_t*) allocate(arena_allocator->backing_allocator, sizeof(node_t) + size);

    if (node == NULL)
    {
        return NULL;
    }

    void* new_memory = (void*) (node + 1);

    memcpy(new_memory, memory, node->size < size ? node->size : size);

    node->size = size;
    node->tail = arena_allocator->head;
    arena_allocator->head = node;

    return new_memory;
}

static void* arena_allocator_reallocate_callback(allocator_t* allocator, void* memory, size_t size)
{
    arena_allocator_t* arena_allocator = FIELD_PARENT_PTR(arena_allocator_t, allocator, allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    else if (memory == 0)
    {
        return arena_allocator_allocate(arena_allocator, size);
    }
    else if (size == 0)
    {
        // Cleanup is handled in `arena_allocator_destroy/1`.
        return NULL;
    }
    else
    {
        return arena_allocator_reallocate(arena_allocator, memory, size);
    }
}

allocator_t* arena_allocator_create(allocator_t* backing_allocator)
{
    if (backing_allocator == NULL)
    {
        return NULL;
    }

    arena_allocator_t* arena_allocator = create(backing_allocator, arena_allocator_t);

    if (arena_allocator == NULL)
    {
        return NULL;
    }

    arena_allocator->allocator.__synchronized = false;
    arena_allocator->allocator.__reallocate_fn = arena_allocator_reallocate_callback;
    arena_allocator->backing_allocator = backing_allocator;
    arena_allocator->head = NULL;

    return &arena_allocator->allocator;
}

void arena_allocator_destroy(allocator_t* allocator)
{
    arena_allocator_t* arena_allocator = FIELD_PARENT_PTR(arena_allocator_t, allocator, allocator);

    node_t* node = arena_allocator->head;
    while (node != NULL)
    {
        node_t* next = node->tail;
        destroy(arena_allocator->backing_allocator, node);
        node = next;
    }

    destroy(arena_allocator->backing_allocator, arena_allocator);
}
