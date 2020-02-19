#ifndef __ALLOCATORS__SEM__
#define __ALLOCATORS__SEM__

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

#define AS_OK 0
#define AS_ERROR -1

#ifdef __APPLE__
typedef dispatch_semaphore_t allocator_sem_t;
#else
typedef sem_t allocator_sem_t;
#endif

/**
 * Initializes a semaphore with the given initial value.
 * @param semaphore The semaphore to initialize
 * @param value An initial value
 * @return A result code of either `AS_OK`, or something else for an error.
 */
int allocator_sem_init(allocator_sem_t* semaphore, uint value);

/**
 * Deinitializes a semaphore.
 * @param semaphore A semaphore
 */
void allocator_sem_deinit(allocator_sem_t* semaphore);

/**
 * Waits for a semaphore to be available, block the current thread of execution.
 * @param semaphore A semaphore
 */
void allocator_sem_wait(allocator_sem_t* semaphore);

/**
 * Signals a semaphore.
 * @param semaphore A semaphore
 */
void allocator_sem_signal(allocator_sem_t* semaphore);

#endif
