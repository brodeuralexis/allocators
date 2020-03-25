#include "libmem.h"

#include <stdlib.h>
#include <errno.h>
#include <limits.h>

char buffer[4096];
int userChoice;
enum mem_strategy currentStrategy;

/* Utility function found on StackOverflow by "user2371524" (Felix Palmen, felix@palmen-it.de) to read a number*/
// return success as boolean (0, 1), on success write result through *number:
int safeInput(int* number)
{
    long a;
    char buf[1024]; // use 1KiB just to be sure

    if (!fgets(buf, 1024, stdin))
    {
        // reading input failed:
        return 0;
    }

    // have some input, convert it to integer:
    char* endptr;

    errno = 0; // reset error number
    a = strtol(buf, &endptr, 10);
    if (errno == ERANGE)
    {
        // out of range for a long
        return 0;
    }
    if (endptr == buf)
    {
        // no character was read
        return 0;
    }
    if (*endptr && *endptr != '\n')
    {
        // *endptr is neither end of string nor newline,
        // so we didn't convert the *whole* input
        return 0;
    }
    if (a > INT_MAX || a < INT_MIN)
    {
        // result will not fit in an int
        return 0;
    }

    // write result through the pointer passed
    *number = (int)a;
    return 1;
}

void printMainMenu()
{
    printf("Current allocating strategy : ");
    if (currentStrategy == ms_first_fit)
    {
        printf("FIRST FIT\n");
    }
    else if (currentStrategy == ms_best_fit)
    {
        printf("BEST FIT\n");
    }
    else if (currentStrategy == ms_worst_fit)
    {
        printf("WORST FIT\n");
    }
    else if (currentStrategy == ms_next_fit)
    {
        printf("NEXT FIT\n");
    }
    else
    {
        printf("UNKNOWN\n");
    }

    printf("What do you want to do?\n");
    printf("1 - Change current allocating strategy?\n");
    printf("2 - Print current memory state?\n");
    printf("3 - Clear memory?\n");
    printf("4 - Allocate byte?\n");
    printf("5 - Free byte?\n");
    printf("6 - Quit?\n");

    printf("Your choice : ");

    userChoice = 0;
    safeInput(&userChoice);
    printf("\n");
}

void printChangingStrategyMenu()
{
    printf("1 - FIRST FIT\n");
    printf("2 - BEST FIT\n");
    printf("3 - WORST FIT\n");
    printf("4 - NEXT FIT\n");

    userChoice = 0;
    safeInput(&userChoice);
}

void changeStrategy(int choice)
{
    if (choice == 1)
    {
        currentStrategy = ms_first_fit;
    }
    else if (choice == 2)
    {
        currentStrategy = ms_best_fit;
    }
    else if (choice == 3)
    {
        currentStrategy = ms_worst_fit;
    }
    else if (choice == 4)
    {
        currentStrategy = ms_next_fit;
    }
}

int main()
{
    currentStrategy = ms_first_fit;
    bool running = true;

    initmem(currentStrategy, &buffer, sizeof(buffer));

    while (running)
    {
        printMainMenu();

        switch (userChoice)
        {
            case 1:
            {
                printChangingStrategyMenu();
                changeStrategy(userChoice);

                if (currentStrategy != NULL)
                {
                    setstrategy(currentStrategy);
                }
            } break;

            case 2:
            {
                affiche_etat_memoire();
            } break;

            case 3:
            {
                clearmem();
            } break;

            case 4:
            {
                int bytes = 0;
                printf("How many bytes? ");
                if (safeInput(&bytes))
                {
                    alloumem(bytes);
                }
            } break;

            case 5:
            {
                int block = 0;
                printf("Which memory block? ");
                if (safeInput(&block))
                {
                    liberebloc(block);
                }
            } break;

            case 6:
            {
                running = false;
            } break;
        }
    }
}
