#include "RingAllocator.h"
#include <stdio.h>

static bool is_alloc_possible(RingAllocator *alloc, size_t size);

static _ringQueue *create_ring_queue(size_t capacity);
static bool ring_queue_push(_ringQueue *queue, size_t offset);
static bool ring_queue_top(_ringQueue *queue, size_t *offset);
static bool ring_queue_pop(_ringQueue *queue, size_t *offset);
static void destroy_ring_queue(_ringQueue *queue);
static void ring_queue_debug_print(_ringQueue *queue);

RingAllocator *create_ring_allocator(size_t memory_pool_size, size_t max_allocated_ptrs_amount)
{
    RingAllocator *alloc = malloc(sizeof(RingAllocator));

    alloc->_memory_pool = malloc(memory_pool_size);
    alloc->_head = 0;
    alloc->_tail = 0;
    alloc->_memory_pool_size = memory_pool_size;
    alloc->_carry_flag = false;
    alloc->_queue = create_ring_queue(max_allocated_ptrs_amount);

    return alloc;
}

void ring_allocator_debug_print(RingAllocator *alloc)
{
    printf("{StartAddr: %p, EndAddr: %p, Head: %d, Tail: %d, MemPoolSize: %d, CarryFlag: %d}\n",
           alloc->_memory_pool, &alloc->_memory_pool[alloc->_memory_pool_size],
           alloc->_head, alloc->_tail, alloc->_memory_pool_size, alloc->_carry_flag);
}

void *ring_alloc(RingAllocator *alloc, size_t size)
{
    // printf("Ring alloc called: \n");
    // ring_allocator_debug_print(alloc);

    if (!is_alloc_possible(alloc, size))
    {
        return NULL;
    }

    if (alloc->_head + size > alloc->_memory_pool_size)
    {
        alloc->_head = 0;
        alloc->_carry_flag = true;
    }

    if (!ring_queue_push(alloc->_queue, alloc->_head))
    {
        printf("EXCEEDED MAX AMOUNT OF ALLOCATED POINTERS");
        exit(0);
    }

    void *retPtr = &alloc->_memory_pool[alloc->_head];

    alloc->_head += size;
    if (alloc->_head == alloc->_memory_pool_size)
    {
        alloc->_head = 0;
        alloc->_carry_flag = true;
    }

    return retPtr;
}

void ring_free(RingAllocator *alloc, void *mem)
{
    // printf("Ring free called\n");
    // ring_allocator_debug_print(alloc);
    size_t offset_value = (uint8_t *)mem - alloc->_memory_pool;
    size_t control_value = -1;

    if (!ring_queue_pop(alloc->_queue, &control_value))
    {
        printf("\nFREE OF NOT ALLCOATED MEMORY OCCURED!\n");
        exit(0);
    }
    else if (offset_value != control_value)
    {
        printf("\nINVALID FREE ORDER DETECTED (Expected: %llu, Got: %llu) \n", control_value, offset_value);
        exit(0);
    }

    size_t new_tail_pos = 0;
    if (!ring_queue_top(alloc->_queue, &new_tail_pos))
    {
        alloc->_head = 0;
        alloc->_tail = 0;
        alloc->_carry_flag = false;
    }
    else
    {
        if (new_tail_pos <= alloc->_tail)
        {
            alloc->_carry_flag = false;
        }

        alloc->_tail = new_tail_pos;
    }
}

void destroy_ring_allocator(RingAllocator *alloc)
{
    destroy_ring_queue((alloc->_queue));
    free(alloc->_memory_pool);
    free(alloc);
}

// private funcs impl

static bool is_alloc_possible(RingAllocator *alloc, size_t size)
{

    if (size == 0 || size > alloc->_memory_pool_size)
    {
        return false;
    }
    else if (alloc->_head + size >= alloc->_memory_pool_size) // handle head carry(size == next_head_pos in that case)
    {
        if (alloc->_head + size == alloc->_memory_pool_size)
            size = 0;

        return !(alloc->_carry_flag || size > alloc->_tail);
    }
    else
    {
        if (!alloc->_carry_flag)
            return true;
        else
            return alloc->_head + size <= alloc->_tail;
    }
}

static _ringQueue *create_ring_queue(size_t capacity)
{
    _ringQueue *queue = malloc(sizeof(_ringQueue));
    queue->_offsets = malloc(capacity * sizeof(size_t));
    queue->_capacity = capacity;
    queue->_count = 0;
    queue->_front = 0;
    queue->_rear = 0;

    return queue;
}

static bool ring_queue_push(_ringQueue *queue, size_t offset)
{
    if (queue->_count == queue->_capacity)
        return false;

    queue->_offsets[queue->_rear] = offset;
    ++queue->_rear;
    ++queue->_count;

    if (queue->_rear >= queue->_capacity)
        queue->_rear = 0;

    return true;
}

static bool ring_queue_top(_ringQueue *queue, size_t *offset)
{
    if (queue->_count == 0)
        return false;

    if (offset != NULL)
        *offset = queue->_offsets[queue->_front];

    return true;
}

static bool ring_queue_pop(_ringQueue *queue, size_t *offset)
{
    if (queue->_count == 0)
        return false;

    if (offset != NULL)
        *offset = queue->_offsets[queue->_front];

    ++queue->_front;
    --queue->_count;

    if (queue->_front >= queue->_capacity)
        queue->_front = 0;

    return true;
}

static void destroy_ring_queue(_ringQueue *queue)
{
    free(queue->_offsets);
    free(queue);
}

static void ring_queue_debug_print(_ringQueue *queue)
{
    printf("[");
    for (int i = queue->_front; i < queue->_rear; ++i)
    {
        printf("%llu, ", queue->_offsets[i]);
    }

    printf("]\n");
}