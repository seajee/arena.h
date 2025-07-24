# arena.h

A single header library that implements
[region-based memory management](https://en.wikipedia.org/wiki/Region-based_memory_management)
in C using a linked list approach.

## Example

```c
#define ARENA_IMPLEMENTATION
#include "arena.h"

int main(void)
{
    Arena a = {0}; // or arena_create(...) to specify the region capacity

    int *x = arena_alloc(&a, sizeof(*x) * 69);
    float *y = arena_alloc(&a, sizeof(*y) * 420);

    arena_free(&a);
    return 0;
}
```
