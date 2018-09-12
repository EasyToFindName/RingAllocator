#ifndef RING_ALLOCATOR
#define RING_ALLOCATOR

#include <stdlib.h>
#include <stdbool.h> //remove me
#include <stdint.h>

typedef struct _ringAllocatorPtrsQueue
{
    size_t *_offsets;
    size_t _capacity;
    size_t _count;
    size_t _front;
    size_t _rear;
} _ringQueue;

typedef struct RingAllocator
{
    uint8_t *_memory_pool;
    size_t _memory_pool_size;
    size_t _head;
    size_t _tail;
    bool _carry_flag;

    _ringQueue *_queue;
} RingAllocator;

RingAllocator *create_ring_allocator(size_t memory_pool_size, size_t max_possible_allocated_ptrs_amount);

void *ring_alloc(RingAllocator *alloc, size_t size);
void ring_free(RingAllocator *alloc, void *memory);

void destroy_ring_allocator(RingAllocator *allocator);
void ring_allocator_debug_print(RingAllocator *allocator);

#endif