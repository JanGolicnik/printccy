#define MY_MATCH_ARG_TYPE_CUSTOM test: print_test

#include "../printccy.h"

typedef struct {
    int x;
} test;

int print_test(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) 
{
    test t = va_arg(*list, test);
    if(output)(void)printout("12345");
    return print(output, output_len, "{}", t.x);
}

int main()
{
    test t = {.x = 123};
    (void)printout("hey whats your name {}", t);
    return 0;
}