#include "allocators/synchronized_allocator.h"

#include "./sem.h"
#include "./macros.h"

typedef struct {
    allocator_t allocator;
    allocator_t* child_allocator;
    allocator_sem_t mutex;
} synchronized_allocator_t;

static void* synchronized_allocator_reallocate_callback(allocator_t* allocator, void* memory, size_t size)
{
    synchronized_allocator_t* synchronized_allocator = FIELD_PARENT_PTR(synchronized_allocator_t, allocator, allocator);
    allocator_sem_wait(&synchronized_allocator->mutex);
    memory = reallocate(synchronized_allocator->child_allocator, memory, size);
    allocator_sem_signal(&synchronized_allocator->mutex);
    return memory;
}

allocator_t* synchronized_allocator_create(allocator_t* child_allocator)
{
    if (child_allocator == NULL)
    {
        return NULL;
    }

    // Already synchronized, do nothing.
    if (child_allocator->__synchronized)
    {
        return child_allocator;
    }

    synchronized_allocator_t* synchronized_allocator = create(child_allocator, synchronized_allocator_t);

    if (synchronized_allocator == NULL)
    {
        return NULL;
    }

    if (allocator_sem_init(&synchronized_allocator->mutex, 1) != AS_OK)
    {
        destroy(child_allocator, synchronized_allocator);
        return NULL;
    }

    synchronized_allocator->allocator.__synchronized = true;
    synchronized_allocator->allocator.__reallocate_fn = synchronized_allocator_reallocate_callback;
    synchronized_allocator->child_allocator = child_allocator;

    return &synchronized_allocator->allocator;
}

void synchronized_allocator_destroy(allocator_t* allocator)
{
    // Not a a synchronized allocator, do nothing.
    if (allocator->__reallocate_fn != synchronized_allocator_reallocate_callback)
    {
        return;
    }

    synchronized_allocator_t* synchronized_allocator = FIELD_PARENT_PTR(synchronized_allocator_t, allocator, allocator);

    allocator_sem_wait(&synchronized_allocator->mutex);
    allocator_sem_signal(&synchronized_allocator->mutex);
    allocator_sem_deinit(&synchronized_allocator->mutex);

    destroy(synchronized_allocator->child_allocator, synchronized_allocator);
}
