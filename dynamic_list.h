/*
    dynamic_list.h -- Single-Header Dynamic List/Array


    Do this:
        #define DYNAMIC_LIST_IMPL
    before you include this file in *one* C file to create the implementation.


    Optionally provide the following defines with your own implementations

        DEFAULT_LIST_CAPACITY       - starting capacity allocated for new lists (default: 16)

    If you're compiling in MSVC, Visual Studio, or Rider, you may need to define this project-wide:

        DYNAMIC_LIST_DEF_MAXALIGN

    which will create a definition of max_align_t, which is missing in recent versions of the
    Windows SDK.


    Basic Usage
    ===========
    --- to create a list:

            int* list = list_new(int);

        this will allocate enough space for the starting default capacity + a header. It returns
        a pointer to the beginning of the usable memory, after the header.

    --- to add to the list:
            list_append(list, 10);
            list_append(list, 20);
            list_append(list, 30);

        list_append will reallocate the entire list if there isnt enough capacity for the new item.
        the list double in capacity every time capacity is reached. Since reallocating may return
        a new pointer, list_append may re-assign a new pointer to the arg passed.

        assume that reallocation invalidates the old pointer, do not keep references to the old
        pointer around.

    --- to access the list:

            list[1] = 21;
            for (size_t i = 0; i < list_len(list); i++)
                printf("%d\n", list[i]);

    --- to free the list:

            list_free(list);

    --- to clear a list:

            list_clear(list);

    --- to declare a typedef for the list:

            list_type(int);

        this will typedef int* to list_int, so that list use is more obvious:

            list_int list = list_new(int);


    Allocators
    ==========
    The default allocator uses the standard C lib `malloc`, `realloc`, and `free`.

    You may provide your own allocator:

        Allocator allocator = {
            .alloc = alloc_func,
            .realloc = realloc_func,
            .free = free_func,
            .context = NULL,
        };

        list_int list = list_new_alloc(int, &allocator);

    The functions must have these signatures:

        void* alloc(size_t size, void* context);
        void* realloc(void* p, size_t new_size, void* context);
        void  free(void* p, void* context);

    The context pointer passed to the Allocator struct is for custom allocator state. Whenever any
    of the allocator functions are called, the context pointer is passed to the function.

    License
    =======
    Copyright 2024 dresswithpockets (dresswithpockets@pm.me)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

#include <stddef.h>

#ifdef DYNAMIC_LIST_DEF_MAXALIGN
// this definition of max_align_t is only really necessary when using MSVC, since max_align_t isnt
// included in stddef.h on some versions of the Windows SDK for some ungodly reason, even in newer C
// standards
typedef struct {
    _Alignas(_Alignof(long long)) long long max_align_ll;
    _Alignas(_Alignof(long double)) long double max_align_ld;
#ifdef _M_X86
    _Alignas(_Alignof(__float128)) max_align_f128;
#endif
} max_align_t;
#endif

#ifndef DEFAULT_LIST_CAPACITY
#define DEFAULT_LIST_CAPACITY 16
#endif



#define list_type(T) typedef T* list_##T
#define list_prelude(list) ((ListPrelude*)(list)-1)
#define list_new(T) ((T*)create_list(sizeof(T), DEFAULT_LIST_CAPACITY, NULL))
#define list_new_alloc(T, allocator) ((T*)create_list(sizeof(T), DEFAULT_LIST_CAPACITY, allocator))
#define list_free(list) list_prelude(list)->allocator->free(list_prelude(list), list_prelude(list)->allocator->context)
#define list_len(list) (list_prelude(list)->length)
#define list_cap(list) (list_prelude(list)->capacity)
#define list_clear(list) (list_prelude(list)->length = 0)
#define list_resize(list, desired) ( \
    (list) = list_ensure_capacity(list, desired, sizeof(*(list))), \
    &(list)[list_prelude(list)->length += (desired)])
#define list_append(list, item) ( \
    (list) = list_ensure_capacity(list, 1, sizeof(item)), \
    (list)[list_prelude(list)->length] = (item), \
    &(list)[list_prelude(list)->length++])
#define list_remove_at(list, index) do { \
    ListPrelude *h = list_prelude(list); \
    if ((index) == h->length - 1) { \
        h->length -= 1; \
    } else if (h->length > 1) { \
        void *ptr = &(list)[index]; \
        void *last = &(list)[h->length - 1]; \
        h->length -= 1; \
        memcpy(ptr, last, sizeof(*(list))); \
    } \
} while (0)

#define list_pop_back(a) (array_header(a)->length -= 1)

typedef struct
{
    void* (*alloc)(size_t, void*);
    void* (*realloc)(void*, size_t, void*);
    void (*free)(void*, void*);
    void* context;
} Allocator;

typedef struct
{
    size_t capacity;
    size_t length;
    _Alignas(max_align_t) Allocator* allocator;
} ListPrelude;

void* create_list(size_t stride, size_t capacity, Allocator* allocator);
void* list_ensure_capacity(void *list, size_t item_count, size_t item_size);

#ifdef DYNAMIC_LIST_IMPL

#include <stdlib.h>

void* default_allocator_alloc(const size_t size, void* context)
{
    (void)context;
    return malloc(size);
}

void* default_allocator_realloc(void* ptr, const size_t size, void* context)
{
    (void)context;
    return realloc(ptr, size);
}

void default_allocator_free(void* ptr, void* context)
{
    (void)context;
    free(ptr);
}

static Allocator default_allocator = {
    .alloc = default_allocator_alloc,
    .realloc = default_allocator_realloc,
    .free = default_allocator_free,
    .context = NULL,
};

void* create_list(const size_t stride, const size_t capacity, Allocator* allocator)
{
    if (allocator == NULL)
        allocator = &default_allocator;

    void* result = NULL;
    ListPrelude* prelude = allocator->alloc(sizeof(ListPrelude) + stride * capacity, allocator->context);

    if (prelude)
    {
        prelude->capacity = capacity;
        prelude->length = 0;
        prelude->allocator = allocator;
        result = prelude + 1;
    }

    return result;
}

void* list_ensure_capacity(void *list, const size_t item_count, const size_t item_size) {
    ListPrelude* prelude = list_prelude(list);
    const size_t desired_capacity = prelude->length + item_count;

    if (prelude->capacity < desired_capacity) {
        size_t new_capacity = prelude->capacity * 2;
        while (new_capacity < desired_capacity) {
            new_capacity *= 2;
        }

        const size_t new_size = sizeof(ListPrelude) + new_capacity * item_size;
        prelude = prelude->allocator->realloc(prelude, new_size, prelude->allocator->context);
        prelude->capacity = new_capacity;
    }

    return prelude + 1;
}

#endif
