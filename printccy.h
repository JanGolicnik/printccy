#ifndef _PRINTCCY_H
#define _PRINTCCY_H

#include <stdio.h>
#include <stdarg.h> 

#define  _MY_COPY_ARGS(to, from, len) do { for (size_t i = 0; i < (len); i++) { char c = *((from)+i); if(c != '*') (to)[i] = c; } } while(0)

int my_print_int(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    int val = va_arg(*list, int);
    char buf[2 + args_len ? args_len : 0];
    buf[0] = '%'; buf[1] = args_len > 0 ? args[0] : 'd';
    if (args_len) _MY_COPY_ARGS(buf + 2, args + 1, args_len - 1);
    return snprintf(output, output_len, buf, val);
}

int my_print_long_long(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    long long val = va_arg(*list, long long);
    char buf[2 + args_len ? args_len : 0];
    buf[0] = '%'; buf[1] = args_len > 0 ? args[0] : 'd';
    if (args_len) _MY_COPY_ARGS(buf + 2, args + 1, args_len - 1);
    return snprintf(output, output_len, buf, val);
}

int my_print_double(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    double val = va_arg(*list, double);
    char buf[2 + args_len ? args_len : 0];
    buf[0] = '%'; buf[1] = args_len > 0 ? args[0] : 'f';
    if (args_len) _MY_COPY_ARGS(buf + 2, args + 1, args_len - 1);
    return snprintf(output, output_len, buf, val);
}

int my_print_char_ptr(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    const char* val = va_arg(*list, char*);
    char buf[2 + args_len ? args_len : 0];
    buf[0] = '%'; buf[1] = args_len > 0 ? args[0] : 's';
    if (args_len) _MY_COPY_ARGS(buf + 2, args + 1, args_len - 1);
    return snprintf(output, output_len, buf, val);
}

int my_print_float(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    return my_print_double(output, output_len, list, args, args_len);
}

typedef int(_my_print_func)(char*, size_t, va_list*, const char*, size_t);
_Thread_local _my_print_func* _my_print_funcs[128];
_Thread_local _my_print_func** _my_funcs_ptr = NULL;

int _my_print(char* output, int output_len, const char* fmt, int n_args, ...)
{
    va_list args; va_start(args, n_args);
    _my_funcs_ptr = _my_print_funcs;

    int bytes_written = 0;
    char c, is_escape = 0;
    while ((c = *(fmt++))) 
    {
        if (is_escape || (c != '{' && c != '%'))
        {
            is_escape = 0;
            if(output) *(output + bytes_written) = c;
            bytes_written++;
            continue;
        }

        if (c == '%') 
        {
            is_escape = 1;
            continue;
        }

        const char* start = fmt;
        while(*fmt && *fmt != '}') {fmt++;}
        bytes_written += (*_my_funcs_ptr++)(output + bytes_written, output_len, &args, start, fmt++ - start);
    }

    va_end(args);

    return bytes_written;
}

#define __NARG__(...)  __NARG_I_( __VA_ARGS__, __RSEQ_N() )
#define __NARG_I_(...) __ARG_N( __VA_ARGS__ )
#define __ARG_N( _1, _2, _3, _4, _5, _6, _7, N, ... ) N
#define __RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

#define _MY_MATCH_ARG_TYPE_BASE int: my_print_int, float: my_print_float, double: my_print_double, long long: my_print_long_long, char*: my_print_char_ptr

// should be defined with a comma in front: #define MY_MATCH_ARG_TYPE_CUSTOM my_type: ...
#ifdef MY_MATCH_ARG_TYPE_CUSTOM
#define _MY_MATCH_ARG_COMMA ,
#else
#define MY_MATCH_ARG_TYPE_CUSTOM
#define _MY_MATCH_ARG_COMMA
#endif // MY_MATCH_ARG_TYPE_CUSTOM

#ifndef _MY_MATCH_ARG_TYPE
#define _MY_MATCH_ARG_TYPE(X) _Generic((X), _MY_MATCH_ARG_TYPE_BASE _MY_MATCH_ARG_COMMA MY_MATCH_ARG_TYPE_CUSTOM, default: NULL)
#endif // _MY_MATCH_ARG_TYPE

#define _MY_ASSERT(expr, msg) sizeof(struct { _Static_assert(expr, msg); int _dummy; })
#define _MY_ASSERT_NON_ZERO_OR_EVAL(expr, msg) ((void)_MY_ASSERT(expr != 0, msg), expr)

#define _MY_FILL_FPTR(X) *(_my_funcs_ptr++) = _MY_ASSERT_NON_ZERO_OR_EVAL(_MY_MATCH_ARG_TYPE(X), "unsupported type of variable " #X)

#define _MY_FILL_FPTR1(_0) _MY_FILL_FPTR(_0)
#define _MY_FILL_FPTR2(_0, _1) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1)
#define _MY_FILL_FPTR3(_0, _1, _2) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2)
#define _MY_FILL_FPTR4(_0, _1, _2, _3) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3)
#define _MY_FILL_FPTR5(_0, _1, _2, _3, _4) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3), _MY_FILL_FPTR(_4)
#define _MY_FILL_FPTR6(_0, _1, _2, _3, _4, _5) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3), _MY_FILL_FPTR(_4), _MY_FILL_FPTR(_5)
#define _MY_FILL_FPTR7(_0, _1, _2, _3, _4, _5, _6) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3), _MY_FILL_FPTR(_4), _MY_FILL_FPTR(_5), _MY_FILL_FPTR(_6)

#define _MY_OVERLOAD_MACRO(_0, _1, _2, _3, _4, _5, _6, MACRO_NAME, ...) MACRO_NAME

#define print(output_buffer, output_len, fmt, ...) ( _my_funcs_ptr = _my_print_funcs,\
                            _MY_OVERLOAD_MACRO( __VA_ARGS__, _MY_FILL_FPTR7, _MY_FILL_FPTR6, _MY_FILL_FPTR5, _MY_FILL_FPTR4, _MY_FILL_FPTR3, _MY_FILL_FPTR2, _MY_FILL_FPTR1 )( __VA_ARGS__ ),\
                            _my_print(output_buffer, output_len, fmt, __NARG__( __VA_ARGS__ ), __VA_ARGS__ ))

_Thread_local char _my_buffer[2<<16];

#define printfb(fb, fmt, ...) (fwrite(_my_buffer, sizeof(_my_buffer[0]), print(_my_buffer, 2<<16, fmt, __VA_ARGS__), fb))


#define printout(fmt, ...) printfb(stdout, fmt, __VA_ARGS__)

#endif // _PRINTCCY_H