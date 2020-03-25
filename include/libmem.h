#ifndef __LIBMEM__
#define __LIBMEM__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define memlibre nbloclibre

enum mem_strategy {
    ms_first_fit,
    ms_best_fit,
    ms_worst_fit,
    ms_next_fit,
};

void initmem(enum mem_strategy strategy, void* buffer, size_t size);

void setstrategy(enum mem_strategy strategy);

void* alloumem(size_t size);

void liberemem(void* mem);

void liberebloc(size_t block);

void clearmem();

size_t nbloclibre();

size_t nblocalloue();

size_t mem_pgrand_libre();

size_t mem_small_free(size_t threshold);

bool mem_est_alloue(void* mem);

void affiche_etat_memoire();

void affiche_parametre_memoire(size_t maxTaillePetit);

#endif
