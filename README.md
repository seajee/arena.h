# arena.h

A single header library that implements
[https://en.wikipedia.org/wiki/Region-based_memory_management](region-based memory management) in C.

## Example

```c
#define ARENA_IMPLEMENTATION
#include "arena.h"

int main(void)
{
    Arena a = {0};

    int *x = arena_alloc(&a, sizeof(*x) * 69);
    float *y = arena_alloc(&a, sizeof(*y) * 420);

    arena_free(&a);
    return 0;
}
```
