# PRINTCCY

Printccy is a **header-only** library that introduces a new way to print text in C! It uses some **macro magic** to infer types from variadic arguments and calls the appropriate print functions automatically. Users can also register their own types for printing.

### Example Usage

```c
const char* name = "Mister";
float height = 1.7843f;

printout("User: ( name: {}, height: {} )", name, height);
```

Or rather with a custom types:

```c
typedef struct {
    const char* name;
    float height;
} User;

int print_user(...)
{
    ...
}

#define PRINTCCY_MATCH_ARG_TYPE_CUSTOM User: print_user


...

User user = { .name = "Mister", .height = 1.7843f };
printout("User: {}", user);
```

## Overview

Printccy provides three main macros:

- **`print`** - Prints to a provided buffer.
- **`printfb`** - Prints to a provided file.
- **`printout`** - Prints to `stdout`.

Internally, it uses `stdio` for formatting basic types and supports all standard format specifiers **except `*`** as in `printf`.

Arguments are printed at their respective `{}` placeholders, which can include format specifiers passed to the printing functions. To escape `{}`, use `%`, or define a custom escape character using:

```c
#define PRINTCCY_ESCAPE_CHARACTER '%'
```

## Custom Type Example

You can define custom types for printing:

```c
typedef struct {
    float x, y, z;
} vector;

int print_vector(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    vector val = va_arg(*list, vector);
    return print(output, output_len, "vector %{ x: {.2f}, y: {.2f}, z: {.2f} }", val.x, val.y, val.z);
}

#define PRINTCCY_MATCH_ARG_TYPE_CUSTOM vector: print_vector
#include "printccy.h"

printout("my vector is {}", (vector){ .x = 1.0f, .y = 2.0f, .z = 3.0f });
```

**Note:** The function must return the number of printed characters, which is necessary for correct printing to files (which includes stdout).

## Implementation Details

While developing Printccy, I learned a few macro tricks. The most important was **macro overloading**, which allows calling different macros based on the number of arguments:

```c
#define OVERLOAD1(_0) ...
#define OVERLOAD2(_0, _1) ...
#define OVERLOAD3(_0, _1, _2) ...

#define OVERLOAD_MACRO_HELPER(_2, _1, _0, NAME, ...) NAME
#define OVERLOAD_MACRO(_0, _1, _2, ...) OVERLOAD_MACRO_HELPER(__VA_ARGS__, OVERLOAD3, OVERLOAD2, OVERLOAD1)(__VA_ARGS__)
```

However, this approach doesn't work with **empty variadic arguments**, making `print("hey")` invalid. The workaround? Ensuring that the **format string is always the first argument**:

```c
#define print(...) ...
```

This lets us apply macro overloading without issues.

### Type Matching

After selecting the correct overload, `_Generic` is used to add the function pointer of each argument's printing function to a **thread-local buffer**. The main printing function then processes `{}` in the string and calls the functions in sequence.

### Printing to file

The code around the printfb is very ugly, mostly because I couldnt figure out a way to declare a local VA array based on the print(0, 0, ...) return value.

At the end I couldn't really figure out a different option so it stayed as a global thread local buffer of some size that the functions write to.

This also means that handling recursive printing calls (they shouldnt really be used aside from debugging purposes) is a huge pain but I somehow made it work.
