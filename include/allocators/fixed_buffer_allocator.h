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

void fixed_buffer_allocator_debug(fixed_buffer_allocator_t* allocator, FILE* file);

size_t fixed_buffer_allocator_debug_memory(fixed_buffer_allocator_t* allocator, void* memory);

#endif
