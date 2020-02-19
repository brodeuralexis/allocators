#include "./sem.h"

#ifndef __APPLE__
#include <errno.h>
#include <stdbool.h>
#endif

int allocator_sem_init(allocator_sem_t* semaphore, uint value)
{
#ifdef __APPLE__
    *semaphore = dispatch_semaphore_create(value);

    if (*semaphore == NULL)
    {
        return AS_ERROR;
    }
#else
    if (sem_init(semaphore, 0, value) != 0)
    {
        return AS_ERROR;
    }
#endif

    return AS_OK;
}

void allocator_sem_deinit(allocator_sem_t* semaphore)
{
#ifdef __APPLE__
    dispatch_release(*semaphore);
#else
    // Only the EINVAL can happen, and we assume that it never will.
    sem_destroy(semaphore);
#endif
}

void allocator_sem_wait(allocator_sem_t* semaphore)
{
#ifdef __APPLE__
    // The result is guaranteed to be 0 when `DISPATCH_TIME_FOREVER` is passed as the timeout parameter.
    dispatch_semaphore_wait(*semaphore, DISPATCH_TIME_FOREVER);
#else
    // We have to handle the case where waiting for a semaphore was interrupted by a signal.
    while (true)
    {
        // We awaited succesfully, break from the loop.
        if (sem_wait(semaphore) == 0)
        {
            break;
        }

        // Only the following 2 errors are returned by sem_wait/1:
        // EINTR: A signal handler was called and interrupted the sem_wait/1 call.
        // EINVAL: The given semaphare was not, in fact, a semaphore.
        //
        // If our semaphore wrapper is well used, EINVAL should never happen, so we assert `errno`.  Since in non DEBUG
        // builds, asserts are discarded, we have a check to break out of the loop.
        assert(errno == EINTR);
        if (errno != EINTR)
        {
            break;
        }
    }
#endif
}

void allocator_sem_signal(allocator_sem_t* semaphore)
{
#ifdef __APPLE__
    // The result only indicate if a thread was woken.  We do not care about the result.
    dispatch_semaphore_signal(*semaphore);
#else
    // Ignore the result as only 2 cases can happen:
    // EINVAL: The semaphore should always be a valid semaphore.
    // EOVERFLOW: The value of the semaphore should never overflow.
    sem_post(semaphore);
#endif
}
