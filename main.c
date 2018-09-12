#include "RingAllocator.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ERR_MESSAGE(s) \
    printf("%s\n", s); \
    exit(1);

void test_full_memory_pull()
{

    printf("\n\n\tTest: Requesting full memory pool multiple times\n");

    RingAllocator *alloc = create_ring_allocator(100, 5);
    void *allocPtr = ring_alloc(alloc, 100);
    if (allocPtr == NULL)
    {
        ERR_MESSAGE("Failed to make a valid request");
    }

    allocPtr = ring_alloc(alloc, 100);

    if (allocPtr != NULL)
    {
        ERR_MESSAGE("Succeeded to make the first invalid request");
    }

    allocPtr = ring_alloc(alloc, 100);
    if (allocPtr != NULL)
    {
        ERR_MESSAGE("Succeeded to make the second invalid request");
    }

    printf("[OK]\n");

    destroy_ring_allocator(alloc);
}

void test_request_after_free()
{
    printf("\n\n\tTest: Request full memory pool release it and request again\n");

    RingAllocator *alloc = create_ring_allocator(100, 5);

    void *allocPtr = ring_alloc(alloc, 100);
    if (allocPtr == NULL)
    {
        ERR_MESSAGE("Failed to make a valid request");
    }

    ring_free(alloc, allocPtr);

    for (int i = 0; i < 3; ++i)
    {
        allocPtr = ring_alloc(alloc, 100);

        if (allocPtr == NULL)
        {
            ERR_MESSAGE("Failed to request after free");
        }

        ring_free(alloc, allocPtr);
    }

    destroy_ring_allocator(alloc);

    printf("[OK]\n");
}

void test_head_carry()
{
    printf("\n\n\tTest: allocating mem at the begining of a memory pool if head reached the end of the buffer  \n");
    RingAllocator *alloc = create_ring_allocator(100, 5);

    void *allocPtr = ring_alloc(alloc, 50);
    if (allocPtr == NULL)
    {
        ERR_MESSAGE("Failed to make a valid request");
    }

    void *anotherPtr = ring_alloc(alloc, 25);

    if (anotherPtr == NULL)
    {
        ERR_MESSAGE("Failed to make the second valid request");
    }

    ring_free(alloc, allocPtr);
    allocPtr = ring_alloc(alloc, 50);

    if (allocPtr == NULL)
    {
        ERR_MESSAGE("Failed to carry head");
    }

    allocPtr = ring_alloc(alloc, 10);

    if (allocPtr != NULL)
    {
        ERR_MESSAGE("Succeed to make an invalid request");
    }

    printf("[OK]\n");
    destroy_ring_allocator(alloc);
}

void test_multiple_allocs_and_frees()
{
    printf("\n\n\tTest: Many allocations and frees \n");
    RingAllocator *alloc = create_ring_allocator(2 * 1024 * 1024, 10);

    for (int i = 0; i < 100; ++i)
    {
        void *ptr = ring_alloc(alloc, 300000);

        if (ptr == NULL)
        {
            ERR_MESSAGE("Failed to perform a valid request");
        }

        ring_free(alloc, ptr);
    }

    printf("[OK]\n");
    destroy_ring_allocator(alloc);
}

void test_limited_space()
{
    printf("\n\n\tTest: Limited space\n");
    RingAllocator *alloc = create_ring_allocator(10, 5);

    void *ptr = ring_alloc(alloc, 7);

    if (ptr == NULL)
    {
        ERR_MESSAGE("Failed to perform a valid request");
    }

    void *ptr2 = ring_alloc(alloc, 3);

    if (ptr2 == NULL)
    {
        ERR_MESSAGE("Failed to perform the second valid request");
    }

    ring_free(alloc, ptr);

    ptr2 = ring_alloc(alloc, 8);

    if (ptr2 != NULL)
    {
        ERR_MESSAGE("Succedded to perform an invalid request");
    }

    printf("[OK]\n");

    destroy_ring_allocator(alloc);
}

void test_bounds()
{
    printf("\n\n\tTest: Checking bounds\n");
    RingAllocator *alloc = create_ring_allocator(10, 5);

    void *ptr = ring_alloc(alloc, 9);

    if (ptr == NULL)
    {
        ERR_MESSAGE("Failed to perform a valid request");
    }

    ptr = ring_alloc(alloc, 1);

    if (ptr == NULL)
    {
        ERR_MESSAGE("Failed to perform the second valid request");
    }

    ptr = ring_alloc(alloc, 5);

    if (ptr != NULL)
    {
        ERR_MESSAGE("Succedded to perform an invalid request");
    }

    printf("[OK]\n");
    destroy_ring_allocator(alloc);
}

void test_pointers()
{
    printf("\n\n\tTest: Validating returned pointers \n");
    RingAllocator *alloc = create_ring_allocator(1000, 2);

    void *base = ring_alloc(alloc, 400);
    if (base == NULL)
    {
        ERR_MESSAGE("Failed to perform a valid request");
    }

    void *next_ptr = ring_alloc(alloc, 500);
    if (next_ptr == NULL)
    {
        ERR_MESSAGE("Failed to perform a valid request");
    }

    if (next_ptr != base + 400)
    {
        ERR_MESSAGE("The second pointer is invalid");
    }

    ring_free(alloc, base);
    ring_free(alloc, next_ptr);

    for (int i = 1; i < 100; ++i)
    {
        size_t request = i * 13 % (1000);
        void *ptr = ring_alloc(alloc, request);

        if (ptr >= &alloc->_memory_pool[1000])
        {
            ERR_MESSAGE("Heap overflow!");
        }
        else if (ptr == NULL)
        {
            ERR_MESSAGE("Can't handle the valid request");
        }

        ring_free(alloc, ptr);
    }

    printf("[OK]\n");
    destroy_ring_allocator(alloc);
}

void test_memory_integrity()
{
    printf("\n\n\tTest: Memory integrity check\n");
    RingAllocator *alloc = create_ring_allocator(10, 10);
    char *ptrs[10];

    char letter = 'A';
    for (int i = 0; i < 10; ++i)
    {
        ptrs[i] = ring_alloc(alloc, 1);
        *ptrs[i] = letter++;
    }

    if (strncmp(alloc->_memory_pool, "ABCDEFGHIJ", 10) != 0)
    {
        ERR_MESSAGE("Memory corruption detected");
    }

    for (int i = 0; i < 5; ++i)
    {
        ring_free(alloc, ptrs[i]);
        ptrs[i] = ring_alloc(alloc, 1);

        *ptrs[i] = --letter;
    }

    if (strncmp(alloc->_memory_pool, "JIHGFFGHIJ", 10) != 0)
    {
        ERR_MESSAGE("Memory corruption detected");
    }

    printf("[OK]\n");
    destroy_ring_allocator(alloc);
}

void test_perfomance_1()
{
    size_t requested_size = 30000;
    int N = 1000000;
    struct timeval t0, t1;
    RingAllocator *alloc = create_ring_allocator(2 * 1024 * 1024, 3);

    printf("\n\n\tTest: Perfomance test(Malloc and free immediately)\n");
    printf("Requesting and freeing %u bytes %d times with malloc...\n", requested_size, N);
    gettimeofday(&t0, NULL);

    for (int i = 0; i < N; ++i)
    {
        volatile char *ptr = malloc(requested_size);
        ptr[0] = '\0';
        free(ptr);
    }

    gettimeofday(&t1, NULL);

    printf("Elapsed time: %.2g seconds\n\n", t1.tv_sec - t0.tv_sec + 1E-6 * (t1.tv_usec - t0.tv_usec));

    printf("Requesting and freeing %u bytes %d times with ring alloc...\n", requested_size, N);
    gettimeofday(&t0, NULL);

    for (int i = 0; i < N; ++i)
    {
        volatile void *allocPtr = ring_alloc(alloc, requested_size);
        ring_free(alloc, allocPtr);
    }

    gettimeofday(&t1, NULL);

    printf("Elapsed time: %.2g seconds\n\n", t1.tv_sec - t0.tv_sec + 1E-6 * (t1.tv_usec - t0.tv_usec));

    destroy_ring_allocator(alloc);
}

void test_perfomance_2()
{
    size_t requested_size = 30000;
    int N = 100000;
    struct timeval t0, t1;
    RingAllocator *alloc = create_ring_allocator(requested_size * N, N);
    void **ptrs = malloc(N * sizeof(void *));
    void **ringPtrs = malloc(N * sizeof(void *));

    printf("\n\n\tTest: Perfomance test(Multiple mallocs and then multiple frees)\n");
    printf("Requesting and freeing %u bytes %d times with malloc...\n", requested_size, N);

    gettimeofday(&t0, NULL);

    for (int i = 0; i < N; ++i)
    {
        ptrs[i] = malloc(requested_size);
    }

    for (int i = 0; i < N; ++i)
    {
        free(ptrs[i]);
    }

    gettimeofday(&t1, NULL);

    printf("Elapsed time: %.2g seconds\n\n", t1.tv_sec - t0.tv_sec + 1E-6 * (t1.tv_usec - t0.tv_usec));
    printf("Requesting and freeing %u bytes %d times with ring alloc...\n", requested_size, N);

    gettimeofday(&t0, NULL);

    for (int i = 0; i < N; ++i)
    {
        ringPtrs[i] = ring_alloc(alloc, requested_size);
    }

    for (int i = 0; i < N; ++i)
    {
        ring_free(alloc, ringPtrs[i]);
    }

    gettimeofday(&t1, NULL);

    printf("Elapsed time: %.2g seconds\n\n", t1.tv_sec - t0.tv_sec + 1E-6 * (t1.tv_usec - t0.tv_usec));
}

int main()
{
    test_full_memory_pull();
    test_request_after_free();
    test_head_carry();
    test_multiple_allocs_and_frees();
    test_limited_space();
    test_pointers();
    test_bounds();
    test_memory_integrity();

    test_perfomance_1();
    test_perfomance_2();
    return 0;
}