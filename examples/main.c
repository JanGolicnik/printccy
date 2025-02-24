#define PRINTCCY_CUSTOM_TYPES vector: print_vector

#include "../printccy.h"

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