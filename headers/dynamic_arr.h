#include <stdlib.h>

#define DYNARR(TYPE)        \
    struct TYPE##__Dynarr { \
        TYPE* items;        \
        size_t item_count;  \
    }

#define FOR_I_IN_ARR(ARRAY) for (int i = 0; i < ARRAY.item_count; i++)

#define ARR_APPEND(ARRAY, NEW_ITEM) do {                                                    \
    void* new_arr = realloc(ARRAY.items, (ARRAY.item_count + 1) * sizeof(*ARRAY.items));    \
    if (new_arr == NULL) break;                                                             \
    ARRAY.items = new_arr;                                                                  \
    ARRAY.items[ARRAY.item_count] = NEW_ITEM;                                               \
    ARRAY.item_count++;                                                                     \
} while(0)

#define ARR_ITEM_AT(ARRAY, INDEX, DEFAULT) (ARRAY.item_count < INDEX) ? ARRAY.items[INDEX] : DEFAULT

#define FOR_ITEM_IN_ARR(ARRAY, ITEM_NAME) \
    for(typeof(ARRAY.items) ITEM_NAME = ARRAY.items; ITEM_NAME < ARRAY.items + ARRAY.item_count; ITEM_NAME++)

#define ARR_GET(ARRAY, ITEM_NAME, CONDITIONAL)  \
    ({                                          \
        typeof(ARRAY.items) ret = NULL;         \
        FOR_ITEM_IN_ARR(ARRAY, ITEM_NAME) {     \
            if (CONDITIONAL) {                  \
                ret = ITEM_NAME;                \
                break;                          \
            }                                   \
        }                                       \
     ret;                                       \
     })
