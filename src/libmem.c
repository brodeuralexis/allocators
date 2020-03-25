#include "libmem.h"

#include "allocators/fixed_buffer_allocator.h"

fixed_buffer_allocator_t fba;

static fixed_buffer_node_t* getNodeAtIndex(fixed_buffer_allocator_t* fba, size_t block)
{
    int i = 0;
    fixed_buffer_node_t* node = fixed_buffer_node_first(fba);

    while (node && i < block)
    {
        node = fixed_buffer_node_next(fba, node);
        ++i;
    }

    if (node)
    {
        return node;
    }
    else
    {
        return NULL;
    }
}

void initmem(enum mem_strategy strategy, void* buffer, size_t size)
{
    switch (strategy)
    {
        case ms_first_fit:
            fba = fixed_buffer_allocator_init(FBS_FIRST_FIT, buffer, size);
            break;
        case ms_best_fit:
            fba = fixed_buffer_allocator_init(FBS_BEST_FIT, buffer, size);
            break;
        case ms_worst_fit:
            fba = fixed_buffer_allocator_init(FBS_WORST_FIT, buffer, size);
            break;
        case ms_next_fit:
            fba = fixed_buffer_allocator_init(FBS_NEXT_FIT, buffer, size);
            break;
        default:
            fputs("unknown strategy\n", stderr);
            exit(EXIT_FAILURE);
    }
}

void setstrategy(enum mem_strategy strategy)
{
    switch (strategy)
    {
        case ms_first_fit:
            fixed_buffer_allocator_set_strategy(&fba, FBS_FIRST_FIT);
            break;
        case ms_best_fit:
            fixed_buffer_allocator_set_strategy(&fba, FBS_BEST_FIT);
            break;
        case ms_worst_fit:
            fixed_buffer_allocator_set_strategy(&fba, FBS_WORST_FIT);
            break;
        case ms_next_fit:
            fixed_buffer_allocator_set_strategy(&fba, FBS_NEXT_FIT);
            break;
    }
    
}

void* alloumem(size_t size)
{
    return allocate(&fba.allocator, size);
}

void liberemem(void* mem)
{
    deallocate(&fba.allocator, mem);
}

void liberebloc(size_t block)
{
    fixed_buffer_node_t* node = getNodeAtIndex(&fba, block);

    if (node != NULL)
    {
        if (!node->is_hole)
        {
            deallocate(&fba.allocator, fixed_buffer_node_memory(node));
        }
    }
}

void clearmem()
{
    fixed_buffer_node_t* node = fixed_buffer_node_first(&fba);
    while (node)
    {
        if (!node->is_hole)
        {
            deallocate(&fba.allocator, fixed_buffer_node_memory(node));
        }
        node = fixed_buffer_node_next(&fba, node);
    }
}

size_t nbloclibre()
{
    size_t usable = 0;

    fixed_buffer_node_t* node = fixed_buffer_node_first(&fba);
    while (node != NULL)
    {
        if (node->is_hole)
        {
            usable += node->size;
        }

        node = fixed_buffer_node_next(&fba, node);
    }

    return usable;
}

size_t nblocalloue()
{
    size_t used = 0;

    fixed_buffer_node_t* node = fixed_buffer_node_first(&fba);
    while (node != NULL)
    {
        if (!node->is_hole)
        {
            used += node->size;
        }

        used += sizeof(fixed_buffer_node_t);

        node = fixed_buffer_node_next(&fba, node);
    }

    return used;
}

size_t mem_pgrand_libre()
{
    fixed_buffer_node_t* largest = NULL;

    fixed_buffer_node_t* node = fixed_buffer_node_first(&fba);
    while (node != NULL)
    {
        if (node->is_hole && (largest == NULL || node->size > largest->size))
        {
            largest = node;
        }

        node = fixed_buffer_node_next(&fba, node);
    }

    if (largest == NULL)
    {
        return 0;
    }

    return largest->size;
}

size_t mem_small_free(size_t threshold)
{
    size_t n = 0;

    fixed_buffer_node_t* node = fixed_buffer_node_first(&fba);
    while (node != NULL)
    {
        if (node->is_hole && node->size <= threshold)
        {
            n++;
        }

        node = fixed_buffer_node_next(&fba, node);
    }

    return n;
}

bool mem_est_alloue(void* mem)
{
    return fixed_buffer_node_find(&fba, mem) != NULL;
}

void affiche_etat_memoire()
{
    fixed_buffer_allocator_debug(&fba, stderr);
}

void affiche_parametre_memoire(size_t maxTaillePetit)
{
    fprintf(stderr, "mem_pgrand_libre() == %zu\n", mem_pgrand_libre());
    fprintf(stderr, "memlibre() == %zu\n", memlibre());
    fprintf(stderr, "nbloclibre() == %zu\n", nbloclibre());
    fprintf(stderr, "nblocalloue() == %zu\n", nblocalloue());
    fprintf(stderr, "mem_small_free(%zu) == %zu\n", maxTaillePetit, mem_small_free(maxTaillePetit));
}
