// arena.h - v1.0.1 - MIT License - https://github.com/seajee/arena.h
// single header library for region-based memory management.
//
// License and changelog:
//
//     See end of file.
//
// Compile-time options:
//
//     Note: every compile-time option listed here should be configured before
//           the #include "arena.h"
//
//     #define ARENA_REGION_CAPACITY new_region_capacity_in_bytes (4096)
//
//         This macro defines the default capacity for arena regions.
//
//     #define ARENA_ASSERT my_assert
//
//         This macro defines an alternative function for assertions. In this
//         library, ARENA_ASSERT is only used when ARENA_REALLOC fails.
//
//     #define ARENA_REALLOC my_realloc
//     #define ARENA_FREE my_free
//
//         These macros define alternative functions for dynamic allocation
//         and deallocation on the heap. They are only used for managing
//         Arena_Region structures.
//
// Function documentation:
//
// In this library, arenas are implemented as linked lists of regions. Each
// region will contain the allocated buffers. The following are all of the
// functions
//
//     Arena arena_create(size_t region_capacity)
//
// This function initializes an arena with a specified region capacity. By
// default the region capacity is ARENA_REGION_CAPACITY, which can be
// configured by redefining it before including the header file. This function
// is not strictly necessary for initializing an arena (see example).
//
//     void *arena_alloc(Arena *a,
//                       size_t bytes)
//
// This function allocates a buffer of size <bytes> into the specified arena
// and returns it's pointer.
//
//     void arena_free(Arena *a)
//
// This function frees all of the regions allocated in the specified arena
// which invalidates all the pointers associated with the arena. The arena can
// still be reused. If the arena was created with arena_create() the
// configured region_capacity will be retained.
//
//     void arena_reset(Arena *a)
//
// This function resets an arena by keeping all of the regions allocated but
// invalidates all of the pointers associated with the specified arena.
// Warning: this functions may cause fragmentation, consider setting an
// appropriate region capacity.
//
// Example:
/*
#define ARENA_IMPLEMENTATION
#include "arena.h"

int main(void)
{
    Arena a = {0}; // or arena_create(...) to specify a custom region
                   // capacity just for this arena

    int *x = arena_alloc(&a, sizeof(*x) * 32);
    float *y = arena_alloc(&a, sizeof(*y) * 512);

    arena_free(&a); // The arena can still be reused
    return 0;
}
*/

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

#ifdef __cplusplus
extern "C" { // Prevent name mangling of functions
#endif // __cplusplus

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

Arena arena_create(size_t region_capacity);
void *arena_alloc(Arena *a, size_t bytes);
void arena_free(Arena *a);
void arena_reset(Arena *a);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ARENA_H_

#ifdef ARENA_IMPLEMENTATION

#ifdef __cplusplus
extern "C" { // Prevent name mangling of functions
#endif // __cplusplus

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
    // a->region_capacity = 0;
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

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ARENA_IMPLEMENTATION

/*
 * Revision history:
 *
 *     1.0.1 (2025-08-02) Prevent name mangling of functions; don't reset
 *                        region_capacity in arena_free()
 *     1.0.0 (2025-07-24) Initial release
 */

/*
 * MIT License
 *
 * Copyright (c) 2025 seajee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
