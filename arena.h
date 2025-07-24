#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef ARENA_MIN_CAPACITY
#define ARENA_MIN_CAPACITY 4096
#endif // ARENA_MIN_CAPACITY

typedef struct Arena_Region Arena_Region;

struct Arena_Region {
    size_t count;
    size_t capacity;
    Arena_Region *next;
    uint8_t data[];
};

typedef struct {
    Arena_Region *head;
    Arena_Region *tail;
} Arena;

void *arena_alloc(Arena *a, size_t bytes);
void arena_free(Arena *a);
void arena_reset(Arena *a);

#endif // ARENA_H_

#ifdef ARENA_IMPLEMENTATION

void *arena_alloc(Arena *a, size_t bytes)
{
    if (a == NULL) {
        return NULL;
    }

    // Empty arena
    if (a->head == NULL) {
        size_t size = (bytes > ARENA_MIN_CAPACITY ? bytes : ARENA_MIN_CAPACITY);
        a->head = (Arena_Region*)malloc(sizeof(*a->head) + size);
        if (a->head == NULL) {
            return NULL;
        }
        memset(a->head, 0, sizeof(*a->head));
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

        // If found allocate on that region
        if (bytes <= a->tail->capacity - a->tail->count) {
            a->tail->count += bytes;
            return a->tail->data + a->tail->count;
        }

        // If not create a new region
        a->tail->next = (Arena_Region*)malloc(sizeof(*a->tail) + bytes);
        if (a->tail->next == NULL) {
            return NULL;
        }
        a->tail = a->tail->next;
        memset(a->tail, 0, sizeof(*a->tail));
        a->tail->count = bytes;
        a->tail->capacity = bytes;
        return a->tail->data;
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

    memset(a, 0, sizeof(*a));
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
