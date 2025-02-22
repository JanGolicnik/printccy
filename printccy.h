#ifndef _PRINTCCY_H
#define _PRINTCCY_H

#include <stdio.h>
#include <stdarg.h> 

#ifndef PRINTCCY_ESCAPE_CHARACTER
#define PRINTCCY_ESCAPE_CHARACTER '%'
#endif // PRINTCCT_ESCAPE_CAHRACTER

// filters out the * symbol
#define  _MY_COPY_ARGS(to, from, len) do { for (size_t i = 0; i < (len); i++) { char c = (from)[i]; if(c != '*') (to)[i] = c; } } while(0)

int my_print_int(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    int val = va_arg(*list, int);
    char buf[2 + args_len];
    buf[0] = '%';
    if (args_len) _MY_COPY_ARGS(buf + 1, args + 1, args_len - 1);
    else          buf[1] = 'd';
    return snprintf(output, output_len, buf, val);
}

int my_print_long_long(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    long long val = va_arg(*list, long long);
    char buf[3 + args_len];
    buf[0] = '%'; 
    if (args_len) _MY_COPY_ARGS(buf + 1, args + 1, args_len - 1);
    else          _MY_COPY_ARGS(buf + 1, "lld", 3);
    return snprintf(output, output_len, buf, val);
}

int my_print_double(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    double val = va_arg(*list, double);
    char buf[2 + args_len];
    buf[0] = '%';
    if (args_len) _MY_COPY_ARGS(buf + 1, args + 1, args_len - 1);
    else          buf[1] = 'f';
    return snprintf(output, output_len, buf, val);
}

int my_print_float(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    return my_print_double(output, output_len, list, args, args_len);
}

int my_print_char_ptr(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    const char* val = va_arg(*list, char*);
    char buf[2 + args_len];
    buf[0] = '%'; 
    if (args_len) _MY_COPY_ARGS(buf + 1, args + 1, args_len - 1);
    else          buf[1] = 's';
    return snprintf(output, output_len, buf, val);
}

// should return the number of characters printed
typedef int(_my_print_func)(char*, size_t, va_list*, const char*, size_t);
_Thread_local _my_print_func* _my_print_funcs[128];
_Thread_local _my_print_func** _my_funcs_ptr = 0;

// copies to the output buffer from the fmt string and calls above function pointers when encountering a {} with arguments inside the {}
int _my_print(char* output, int output_len, const char* fmt, ...)
{
    va_list args; va_start(args, fmt);

    int bytes_written = 0;
    char c, is_escape = 0;
    while ((c = *(fmt++))) 
    {
        if (is_escape || (c != '{' && c != PRINTCCY_ESCAPE_CHARACTER))
        {
            is_escape = 0;
            if(output) *(output + bytes_written) = c;
            bytes_written++;
            continue;
        }

        if (c == PRINTCCY_ESCAPE_CHARACTER) 
        {
            is_escape = 1;
            continue;
        }

        const char* start = fmt;
        while(*fmt && *fmt != '}') {fmt++;}
        bytes_written += (*_my_funcs_ptr++)(output ? output + bytes_written : output, output_len, &args, start, fmt++ - start);
    }

    va_end(args);
    return bytes_written;
}

#define _MY_N_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define _MY_N_ARGS(...)  _MY_N_ARGS_HELPER( __VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0 )

#define _MY_MATCH_ARG_TYPE_BASE int: my_print_int, float: my_print_float, double: my_print_double, long long: my_print_long_long, char*: my_print_char_ptr

// define it youreself as: 
// #define MY_MATCH_ARG_TYPE_CUSTOM my_type: print_my_type, my_type2: ...
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

#define _MY_FILL_FPTR1(fmt) (void)0
#define _MY_FILL_FPTR2(fmt, _0) _MY_FILL_FPTR(_0)
#define _MY_FILL_FPTR3(fmt, _0, _1) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1)
#define _MY_FILL_FPTR4(fmt, _0, _1, _2) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2)
#define _MY_FILL_FPTR5(fmt, _0, _1, _2, _3) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3)
#define _MY_FILL_FPTR6(fmt, _0, _1, _2, _3, _4) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3), _MY_FILL_FPTR(_4)
#define _MY_FILL_FPTR7(fmt, _0, _1, _2, _3, _4, _5) _MY_FILL_FPTR(_0), _MY_FILL_FPTR(_1), _MY_FILL_FPTR(_2), _MY_FILL_FPTR(_3), _MY_FILL_FPTR(_4), _MY_FILL_FPTR(_5)

#define _MY_CONCAT_HELPER(a, b) a##b 
#define _MY_CONCAT(a, b) _MY_CONCAT_HELPER(a, b)

_Thread_local int _my_print_sizes[100] = {0};
_Thread_local int* _my_print_size = 0;

#define _MY_INIT (void)(!_my_funcs_ptr ? _my_funcs_ptr = _my_print_funcs : 0 ), (void)(!_my_print_size ? _my_print_size = _my_print_sizes : 0 )
#define _MY_SETUP_PRINT(...) ( _MY_INIT,\
                               _MY_CONCAT(_MY_FILL_FPTR, _MY_N_ARGS(__VA_ARGS__))( __VA_ARGS__ ),\
                               _my_funcs_ptr -= _MY_N_ARGS(__VA_ARGS__) - 1)
#define _MY_CLEAN_UP_PRINT(...) (_my_funcs_ptr -= _MY_N_ARGS(__VA_ARGS__) - 1, *(_my_print_size))

// third argument should be the format string
#define print(output_buffer, output_len, ...) ( _MY_SETUP_PRINT(__VA_ARGS__),\
                                                *(_my_print_size) = _my_print(output_buffer, output_len, __VA_ARGS__),\
                                                _MY_CLEAN_UP_PRINT(__VA_ARGS__) )

#define _MY_BUFFER_SIZE (2<<16)
_Thread_local char _my_buffer[_MY_BUFFER_SIZE];
_Thread_local char* _my_buffer_ptr = 0;

_Thread_local int _my_print_lens[100];
_Thread_local int* _my_print_len;
_Thread_local int _my_current_print_len;

// second argument should be the format string
#define printfb(fb, ...) (!_my_buffer_ptr ? _my_buffer_ptr = _my_buffer : 0,!_my_print_len ? _my_print_len = _my_print_lens : 0, \
                           *_my_print_len = print(0, 0, __VA_ARGS__),\
                           _my_buffer_ptr += *_my_print_len,\
                           _my_current_print_len = print(_my_buffer_ptr - *_my_print_len++, (size_t)_MY_BUFFER_SIZE - (size_t)(_my_buffer_ptr - _my_buffer), __VA_ARGS__),\
                           _my_buffer_ptr -= *(--_my_print_len),\
                           fwrite(_my_buffer_ptr, sizeof(_my_buffer[0]), _my_current_print_len, fb),\
                           _my_current_print_len)

// first argument should be the format string
#define printout(...) printfb(stdout, __VA_ARGS__)

#endif // _PRINTCCY_H