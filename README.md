# C Dynamic List
`dynamic_list.h` -- Single-Header Dynamic List/Array for C11 and above

Do this before you include this file in *one* C file to create the implementation:
```c
#define DYNAMIC_LIST_IMPL
```

**Optionally** provide the following defines with your own implementations:
```c
// the starting capacity always allocated for the list (default: 16)
#define DEFAULT_LIST_CAPACITY 16
```

If you're compiling in MSVC, Visual Studio, or Rider, you may need to define
`DYNAMIC_LIST_DEF_MAXALIGN` project-wide. This will create a definition of `max_align_t`, which is
missing in recent versions of the Windows SDK.

## Basic Usage

See [example.c](example.c) for some basic usage. You can also compile and run it:
```sh
$ gcc example.c -std=c11 -O3
$ ./a
```

### Create a List
```c
int* list = list_new(int);
```
this will allocate enough space for the starting default capacity + a header. It returns
a pointer to the beginning of the usable memory, after the header.

### Add to a List
```c
list_append(list, 10);
list_append(list, 20);
list_append(list, 30);
```

`list_append` will reallocate the entire list if there isnt enough capacity for the new item.
the list double in capacity every time capacity is reached. Since reallocating may return
a new pointer, list_append may re-assign a new pointer to the arg passed.

Assume that reallocation invalidates the old pointer, do not keep references to the old
pointer around.

### Access a List

```c
list[1] = 21;
for (size_t i = 0; i < list_len(list); i++)
    printf("%d\n", list[i]);
```

### Free a List

```c
list_free(list);
```

### Clear a List:

```c
list_clear(list);
```

### Declare List Typedef:

```c
list_type(int);
```

this will typedef int* to list_int, so that list use is more obvious:

```c
list_int list = list_new(int);
```


## Allocators
The default allocator uses the standard C lib `malloc`, `realloc`, and `free`.

You may provide your own allocator:

```c
Allocator allocator = {
    .alloc = alloc_func,
    .realloc = realloc_func,
    .free = free_func,
    .context = NULL,
};

list_int list = list_new_alloc(int, &allocator);
```

The functions must have these signatures:

```c
void* alloc(size_t size, void* context);
void* realloc(void* p, size_t new_size, void* context);
void  free(void* p, void* context);
```

The context pointer passed to the Allocator struct is for custom allocator state. Whenever any
of the allocator functions are called, the context pointer is passed to the function.

## Help

### Getting error - 'max_align_t': undeclared identifier

First, make sure youre compiling with at least the C11 standard.

If you're compiling with MSVC, Visual Studio, or Rider - that is, youre using the Windows SDK - then `max_align_t` may not be declared in `stddef.h`.

`dynamic_list.h` can declare `max_align_t` for you - to enable it, define `DYNAMIC_LIST_DEF_MAXALIGN`.
