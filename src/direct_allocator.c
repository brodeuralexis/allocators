#include "allocators/direct_allocator.h"

#include <string.h>
#include <sys/mman.h>
#include <stdio.h>

#include "./macros.h"

#define NODE_MAGIC 0xCAFEBABE

typedef struct {
    size_t magic;
    size_t size;
} node_t;

static void* direct_allocator_allocate(size_t size)
{
    node_t* node = mmap(NULL, sizeof(node_t) + size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (node == NULL)
    {
        return NULL;
    }

    node->magic = NODE_MAGIC;
    node->size = size;

    return (void*) (node + 1);
}

static void direct_allocator_deallocate(void* memory)
{
    node_t* node = ((node_t*) memory) - 1;

    if (node->magic != NODE_MAGIC)
    {
        return;
    }

    memory = (void*) (node + 1);

    munmap((void*) node, node->size);
}

static void* direct_allocator_reallocate(void* memory, size_t size)
{
    node_t* node = (node_t*) memory;

    if (node->magic != NODE_MAGIC)
    {
        return NULL;
    }

    if (size <= node->size)
    {
        return memory;
    }

    void* new_memory = direct_allocator_allocate(size);

    if (new_memory == NULL)
    {
        return NULL;
    }

    memcpy(new_memory, memory, node->size < size ? node->size : size);

    return new_memory;
}

static void* direct_allocator_reallocate_callback(allocator_t* allocator, void* memory, size_t size)
{
    UNUSED(allocator);

    if (memory == NULL && size == 0)
    {
        return NULL;
    }
    else if (memory == NULL)
    {
        return direct_allocator_allocate(size);
    }
    else if (size == 0)
    {
        direct_allocator_deallocate(memory);
        return NULL;
    }
    else
    {
        return direct_allocator_reallocate(memory, size);
    }
}

static allocator_t direct_allocator_state = { true, direct_allocator_reallocate_callback };
allocator_t* direct_allocator = &direct_allocator_state;
