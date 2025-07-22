#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define malloc(n) (printf("%s:%d:%s: malloc(%ld)\n",\
            __FILE__, __LINE__, __func__, (n)), malloc((n)));
#define free(p)   (printf("%s:%d:%s: free(%p)\n",\
            __FILE__, __LINE__, __func__, (p)), free((p)));
#endif // DEBUG

#define ARENA_MIN_CAPACITY 400
#define ARENA_IMPLEMENTATION
#include "arena.h"

void arena_print(Arena arena)
{
    int i = 0;
    for (Arena_Region *cur = arena.head; cur != NULL; cur = cur->next) {
        printf("Arena_Region #%d:\n", ++i);
        printf("    count    = %ld\n", cur->count);
        printf("    capacity = %ld\n", cur->capacity);
        printf("    next     = %p\n", cur->next);
        printf("    data     = %p\n", cur->data);
    }
}

int main(void)
{
    Arena a = {0};

    printf("------ small alloc ------\n");

    arena_alloc(&a, 100);
    arena_alloc(&a, 100);
    arena_alloc(&a, 100);
    arena_alloc(&a, 100);
    arena_print(a);

    printf("\n=========================================================\n\n");
    printf("------ big alloc ------\n");

    arena_alloc(&a, 8000);
    arena_print(a);

    printf("\n=========================================================\n\n");
    printf("------ reset ------\n");

    arena_reset(&a);
    arena_print(a);

    printf("\n=========================================================\n\n");
    printf("------ allocs after reset ------\n");

    arena_alloc(&a, 300);
    arena_alloc(&a, 400);
    arena_alloc(&a, 9000);
    arena_print(a);

    arena_free(&a);
    return 0;
}
