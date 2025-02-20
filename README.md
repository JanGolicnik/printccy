# PRINTCCY

Printccy is a header-only library that provides a new way to print text in C !
It uses some slight macro magic to infer types from variadic arguments and calls the appropriate print functions. It also allows users to register their own types for printing.

```c
const char* name = "Mister";
float height = 1.7843f;

printout("User ( name: {}, height: {} ) ", name, height);
```

or even better:

```c
typedef struct
{
    const char* name;
    float height;
} User;

...

User user = { name: "Mister", height: 1.7843f };
printout("User: {}", user);
```

## Overview

There are three provided macros `print`, `printfb` and `printout`.

`print` prints to the provided buffer, `printfb` to the provided file and `printout` to stdout.

Internally it uses stdio to format all the basic types and supports all the same format specifiers besides \* as printf.

Arguments are printed at their respective `{}`, which may contain additional arguments that are then passes down to their printing functions. Curly braces can be escaped with a `%`.

## Examples

Custom type:

```c
typedef struct
{
    float x, y, z;
} vector;

int print_vector(char* output, size_t output_len, va_list* list, const char* args, size_t args_len)
{
    vector val = va_arg(*list, vector);
    return print(output, output_len, "vector %{ x: {.2f}, y: {.2f}, z: {.2f} }", val.x, val.y, val.z);
}

#define MY_MATCH_ARG_TYPE_CUSTOM vector: print_vector

#include "printtcy.h"

printout("my vector is {}", (vector){ x: 1.0f, y: 2.0f, z: 3.0f });

```
