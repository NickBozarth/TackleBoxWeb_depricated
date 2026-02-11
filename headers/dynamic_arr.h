#include <stdlib.h>

#define DYNARR(TYPE) \
struct TYPE##__Dynarr { \
    TYPE* items; \
    size_t item_count; \
}

#define FOR_I_IN_ARR(ARRAY) for (int i = 0; i < ARRAY.item_count; i++)

#define ARR_APPEND(ARRAY, NEW_ITEM) do {\
    void* new_arr = realloc(ARRAY.items, (ARRAY.item_count + 1) * sizeof(*ARRAY.items));\
    if (new_arr == NULL) break;\
    ARRAY.items = new_arr;\
    ARRAY.items[ARRAY.item_count] = NEW_ITEM;\
    ARRAY.item_count++;\
} while(0)

#define ARR_ITEM_AT(ARRAY, INDEX) (ARRAY.item_count < INDEX) ? ARRAY.items[INDEX] : {}
