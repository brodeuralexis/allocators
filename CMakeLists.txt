cmake_minimum_required(VERSION 3.14)
project(allocators
    VERSION 1.0.0
    LANGUAGES C
)

add_library(allocators
    include/allocators/allocator.h src/allocator.c
    include/allocators/c_allocator.h src/c_allocator.c
    include/allocators/arena_allocator.h src/arena_allocator.c
    include/allocators/synchronized_allocator.h src/synchronized_allocator.c
    include/allocators/direct_allocator.h src/direct_allocator.c

    src/macros.h
    src/sem.h src/sem.c
)

target_compile_features(allocators
    PUBLIC
        c_std_11
)

target_include_directories(allocators
    SYSTEM INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_options(allocators
    PRIVATE
        -Wall -Wextra -Wpedantic -Werror
)

add_executable(test
    main.c
)

target_link_libraries(test PRIVATE allocators)
