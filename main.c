#include "libmem.h"

#include "allocators/fixed_buffer_allocator.h"

#include <stdlib.h>
#include <errno.h>
#include <limits.h>

char buffer[4096];
int userChoice;

fixed_buffer_strategy_t* currentStrategy;
fixed_buffer_strategy_t* strategies[4];

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
    if (currentStrategy == strategies[0])
    {
        printf("FIRST FIT\n");
    }
    else if (currentStrategy == strategies[1])
    {
        printf("BEST FIT\n");
    }
    else if (currentStrategy == strategies[2])
    {
        printf("WORST FIT\n");
    }
    else if (currentStrategy == strategies[3])
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
    if (choice >= 1 && choice <= 4)
    {
        currentStrategy = strategies[choice - 1];
    }
}

fixed_buffer_node_t* getNodeAtIndex(fixed_buffer_allocator_t* fba, int block)
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

void clear(fixed_buffer_allocator_t* fba)
{
    fixed_buffer_node_t* node = fixed_buffer_node_first(fba);
    while (node)
    {
        if (!node->is_hole)
        {
            deallocate(&fba->allocator, fixed_buffer_node_memory(node));
        }
        node = fixed_buffer_node_next(fba, node);
    }
}

int main()
{
    strategies[0] = FBS_FIRST_FIT;
    strategies[1] = FBS_BEST_FIT;
    strategies[2] = FBS_WORST_FIT;
    strategies[3] = FBS_NEXT_FIT;
    currentStrategy = FBS_FIRST_FIT;
    bool running = true;

    fixed_buffer_allocator_t fba = fixed_buffer_allocator_init(currentStrategy, &buffer, sizeof(buffer));

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
                    fixed_buffer_allocator_set_strategy(&fba, currentStrategy);
                }
            } break;

            case 2:
            {
                fixed_buffer_allocator_debug(&fba, stderr);
            } break;

            case 3:
            {
                clear(&fba);
            } break;

            case 4:
            {
                int bytes = 0;
                printf("How many bytes? ");
                if (safeInput(&bytes))
                {
                    allocate(&fba.allocator, bytes);
                }
            } break;

            case 5:
            {
                int block = 0;
                printf("Which memory block? ");
                if (safeInput(&block))
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
            } break;

            case 6:
            {
                running = false;
            } break;
        }
    }
}
