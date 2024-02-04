#include <stdio.h>

#define DYNAMIC_LIST_IMPL
#include "dynamic_list.h"

int main(int argc, char* argv[])
{
    int* list = list_new(int);

    // append
    for (size_t i = 0; i < 100; i++)
    {
        list_append(list, rand());
    }

    for (size_t i = 0; i < list_len(list); i++)
    {
        printf("Item %zu: %d\n", i, list[i]);
    }

    // resize-and-set
    list_clear(list);
    list_resize(list, 100);
    for (size_t i = 0; i < 100; i++)
    {
        list[i] = rand();
    }

    for (size_t i = 0; i < list_len(list); i++)
    {
        printf("Item %zu: %d\n", i, list[i]);
    }

    // always free it
    list_free(list);
    return 0;
}
