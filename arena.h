#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>
#include <stdint.h>

#ifndef ARENA_REGION_CAPACITY
#    define ARENA_REGION_CAPACITY (8*1024)
#endif // ARENA_REGION_CAPACITY

#ifndef ARENA_ASSERT
#    include <assert.h>
#    define ARENA_ASSERT assert
#endif // ARENA_ASSERT

#ifndef ARENA_REALLOC
#    include <stdlib.h>
#    define ARENA_REALLOC realloc
#endif // ARENA_REALLOC

#ifndef ARENA_FREE
#    include <stdlib.h>
#    define ARENA_FREE free
#endif // ARENA_FREE

typedef struct Arena_Region Arena_Region;

struct Arena_Region {
    Arena_Region *next;
    size_t count;
    size_t capacity;
    uint8_t data[];
};

typedef struct Arena {
    Arena_Region *head;
    Arena_Region *tail;
    size_t region_capacity;
} Arena;

Arena arena_create(size_t min_capacity);
void *arena_alloc(Arena *a, size_t bytes);
void arena_free(Arena *a);
void arena_reset(Arena *a);

#endif // ARENA_H_

#ifdef ARENA_IMPLEMENTATION

Arena arena_create(size_t region_capacity)
{
    Arena a = {0};
    a.region_capacity = region_capacity;
    return a;
}

void *arena_alloc(Arena *a, size_t bytes)
{
    if (a == NULL) {
        return NULL;
    }

    size_t region_capacity = (a->region_capacity == 0
            ? ARENA_REGION_CAPACITY : a->region_capacity);

    // Empty arena
    if (a->head == NULL) {
        size_t size = (bytes > region_capacity ? bytes : region_capacity);
        a->head = (Arena_Region*)ARENA_REALLOC(NULL, sizeof(*a->head) + size);
        ARENA_ASSERT(a->head != NULL);
        if (a->head == NULL) {
            return NULL;
        }
        a->head->count = bytes;
        a->head->capacity = size;
        a->tail = a->head;
        return a->head->data;
    }

    // Not enough capacity
    if (bytes > a->tail->capacity - a->tail->count) {
        // Find first suitable region
        while (a->tail->next != NULL && bytes > a->tail->capacity - a->tail->count) {
            a->tail = a->tail->next;
        }

        // If not found create a new region
        if (bytes > a->tail->capacity - a->tail->count) {
            size_t size = (bytes > region_capacity ? bytes : region_capacity);
            a->tail->next = (Arena_Region*)ARENA_REALLOC(NULL, sizeof(*a->tail) + size);
            ARENA_ASSERT(a->head != NULL);
            if (a->tail->next == NULL) {
                return NULL;
            }
            a->tail = a->tail->next;
            a->tail->count = bytes;
            a->tail->capacity = size;
            return a->tail->data;
        }
    }

    a->tail->count += bytes;
    return a->tail->data + a->tail->count;
}

void arena_free(Arena *a)
{
    if (a == NULL) {
        return;
    }

    Arena_Region *cur = a->head;
    while (cur != NULL) {
        Arena_Region *next = cur->next;
        free(cur);
        cur = next;
    }

    a->head = NULL;
    a->tail = NULL;
    a->region_capacity = 0;
}

void arena_reset(Arena *a)
{
    if (a == NULL) {
        return;
    }

    for (Arena_Region *cur = a->head; cur != NULL; cur = cur->next) {
        cur->count = 0;
    }

    a->tail = a->head;
}

#endif // ARENA_IMPLEMENTATION
