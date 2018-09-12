cmake_minimum_required(VERSION 2.8)

project(allocators LANGUAGES C)

set(SOURCES 
    ./main.c
    ./RingAllocator.c    
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE -O2)

