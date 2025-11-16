# PRINTCCY

Printccy is a header only library that enables type safe printing for arbitrary print arguments in C!
It uses some macro magic to infer types from variadic arguments and calls the appropriate print functions automatically.
Users can also provide printing functions for custom types.

## Usage

```c
#define PRINTCCY_CUSTOM_TYPES vector: print_vector

#include "printccy.h"

typedef struct {
    float x, y, z;
} vector;

int print_vector(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    vector val = va_arg(*list, vector);
    if (args_len && args[0] == 'd')
        return print(output, output_len, "vector %{ x: {.2f}, y: {.2f}, z: {.2f} }", val.x, val.y, val.z);
    return print(output, output_len, "%{ x: {}, y: {}, z: {} }", val.x, val.y, val.z);
}

int main()
{
    printout("Hey {}\n", "mister");

    printout("{.2d} {.2f} {}\n", 1, 2.0f, 2.0);

    printout("{#0+24.12E}\n", 123.456);
    printf("%#0+24.12E\n", 123.456);

    vector vec = {1.0f, 2.0f, 3.0f};
    printout("{d}\n", vec);
}
```

## Overview

Printccy provides three main macros:

- **`print`** - Prints to a provided buffer.
- **`printfb`** - Prints to a provided file.
- **`printout`** - Prints to `stdout`.

Internally, it uses `stdio` for formatting basic types and supports all standard format specifiers **except `*`** as in `printf`.

Arguments are printed at their respective `{}` placeholders, which can include format specifiers passed to the printing functions. By default you escape the `{}` using `%`.

User defineable macros:

```c
#define PRINTCCY_ESCAPE_CHARACTER '%'
#define PRINTCCY_TEMP_BUFFER_SIZE (2<<12) // internal buffer used when printing to files, which includes stdout
#define PRINTCCY_CUSTOM_TYPES // type: printing_function, type2: printing_function2
```

## Notes

Doesn't work well with `-Wdouble-promotion` as variadic arguments automatically promote a bunch of basic types.

Empty argument lists are also a problem: `printout("hey", )` which is a bit mid for declaring your own macros on top of these as `__VA_ARGS__` without `__VA_OPT__` or `##` cannot filter out the comma.

## To-Do

1. Implement va args ourselves :D
2. ifdef all std stuff :3

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
