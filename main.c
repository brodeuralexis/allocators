#include "libmem.h"

char buffer[1000];

int main()
{
    initmem(ms_first_fit, &buffer, sizeof(buffer));

    char* a1 = alloumem(100);
    char* a2 = alloumem(200);
    char* a3 = alloumem(300);
    liberemem(a2);

    affiche_etat_memoire();
    affiche_parametre_memoire(250);
}
