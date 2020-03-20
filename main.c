#include "libmem.h"

#include "allocators/fixed_buffer_allocator.h"

char buffer[4096];

int main()
{
    fixed_buffer_allocator_t fba = fixed_buffer_allocator_init(FBS_FIRST_FIT, &buffer, sizeof(buffer));

    char* _ = allocate(&fba.allocator, 1);
    char* a = allocate(&fba.allocator, 4);
    deallocate(&fba.allocator, _);
    a[0] = 'H';
    a[1] = 'e';
    a[2] = 'l';
    a[3] = 0;

    puts(a);

    fixed_buffer_allocator_debug(&fba, stderr);

    a = reallocate(&fba.allocator, a, 6);
    a[3] = 'l';
    a[4] = 'o';
    a[5] = 0;

    puts(a);

    fixed_buffer_allocator_debug(&fba, stderr);
}
