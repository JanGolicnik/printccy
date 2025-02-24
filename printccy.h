/*
MIT License

Copyright (c) 2025 Jan Goličnik

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _PRINTCCY_H
#define _PRINTCCY_H

#ifndef PRINTCCY_NO_STD
#include <stdio.h>
#include <stdarg.h>
#endif // PRINTCCY_NO_STD

// should return the number of characters printed
typedef int(_printccy_print_func)(char*, size_t, va_list*, const char*, size_t);

#ifndef PRINTCCY_ESCAPE_CHARACTER
#define PRINTCCY_ESCAPE_CHARACTER '%'
#endif // PRINTCCT_ESCAPE_CAHRACTER

#ifndef PRINTCCY_TEMP_BUFFER_SIZE
#define PRINTCCY_TEMP_BUFFER_SIZE (2<<12)
#endif // PRINTCCY_TEMP_BUFFER_SIZE

_Thread_local struct {
    // print
    _printccy_print_func* funcs[128]; // holds the function pointers the printing functions
    size_t funcs_i;

    int printed_size;

    // printfb
    char buffer[PRINTCCY_TEMP_BUFFER_SIZE];
    size_t buffer_i;

    int lens[100];
    int lens_i;
    int written_len;
} _printccy;

#if __STDC_VERSION__ >= 202311L
    #define _PRINTCCY_MAYBE_UNUSED [[maybe_unused]]
// #elif __STDC_VERSION__ >= 201710L
// #elif __STDC_VERSION__ >= 201112L
// #elif __STDC_VERSION__ >= 199901L
#else
    #define _PRINTCCY_MAYBE_UNUSED
#endif

#ifndef PRINTCCY_NO_STD

// filters out the * symbol
#define _PRINTCCY_COPY_ARGS(to, from, len) do { for (size_t i = 0; i < (len); i++) { char c = (from)[i]; if(c != '*') (to)[i] = c; } } while(0)
#define _PRINTCCY_INIT_EMPTY_BUFFER(name, len) char name[len]; for (size_t i = 0; i < (len); i++) (name)[i] = 0;

int printccy_print_int(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    int val = va_arg(*list, int);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 'd';
    return snprintf(output, output_len, buf, val);
}

int printccy_print_long_long(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    long long val = va_arg(*list, long long);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 3));
    buf[0] = '%'; 
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          _PRINTCCY_COPY_ARGS(buf + 1, "lld", 3);
    return snprintf(output, output_len, buf, val);
}

int printccy_print_double(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    double val = va_arg(*list, double);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 'f';
    return snprintf(output, output_len, buf, val);
}

int printccy_print_float(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    return printccy_print_double(output, output_len, list, args, args_len);
}

int printccy_print_char_ptr(char* output, size_t output_len, va_list* list, const char* args, size_t args_len) {
    const char* val = va_arg(*list, char*);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%'; 
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 's';
    return snprintf(output, output_len, buf, val);
}

#define _PRINTCCY_MATCH_ARG_TYPE_BASE int: printccy_print_int, float: printccy_print_float, double: printccy_print_double, long long: printccy_print_long_long, char*: printccy_print_char_ptr

#else // PRINTCCY_NO_STD

#define _PRINTCCY_MATCH_ARG_TYPE_BASE

#endif // PRINTCCY_NO_STD

// define it youtself as: 
// #define PRINTCCY_CUSTOM_TYPES printccy_type: print_printccy_type, printccy_type2: ...
#ifdef PRINTCCY_CUSTOM_TYPES
#define _PRINTCCY_MATCH_ARG_COMMA ,
#else
#define PRINTCCY_CUSTOM_TYPES
#define _PRINTCCY_MATCH_ARG_COMMA
#endif // PRINTCCY_CUSTOM_TYPES

#ifndef _PRINTCCY_MATCH_ARG_TYPE
#define _PRINTCCY_MATCH_ARG_TYPE(X) _Generic((X), _PRINTCCY_MATCH_ARG_TYPE_BASE _PRINTCCY_MATCH_ARG_COMMA PRINTCCY_CUSTOM_TYPES, default: NULL)
#endif // _PRINTCCY_MATCH_ARG_TYPE

// copies to the output buffer from the fmt string and calls above function pointers when encountering a {} with arguments inside the {}
int _printccy_print(char* output, int output_len, const char* fmt, ...)
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
        bytes_written += _printccy.funcs[_printccy.funcs_i++](output ? output + bytes_written : output, output_len, &args, start, fmt++ - start);
    }

    va_end(args);
    return bytes_written;
}

#define _PRINTCCY_ASSERT(expr, msg) sizeof(struct { _Static_assert(expr, msg); int _dummy; })
#define _PRINTCCY_ASSERT_NON_ZERO_OR_EVAL(expr, msg) ((void)_PRINTCCY_ASSERT(expr != 0, msg), expr)

#define _PRINTCCY_FILL_FPTR(X) _printccy.funcs[_printccy.funcs_i++] = _PRINTCCY_ASSERT_NON_ZERO_OR_EVAL(_PRINTCCY_MATCH_ARG_TYPE(X), "unsupported type of variable " #X)

#define _PRINTCCY_FILL_FPTR1(fmt) (void)0
#define _PRINTCCY_FILL_FPTR2(fmt, _0) _PRINTCCY_FILL_FPTR(_0)
#define _PRINTCCY_FILL_FPTR3(fmt, _0, _1) _PRINTCCY_FILL_FPTR(_0), _PRINTCCY_FILL_FPTR(_1)
#define _PRINTCCY_FILL_FPTR4(fmt, _0, _1, _2) _PRINTCCY_FILL_FPTR(_0), _PRINTCCY_FILL_FPTR(_1), _PRINTCCY_FILL_FPTR(_2)
#define _PRINTCCY_FILL_FPTR5(fmt, _0, _1, _2, _3) _PRINTCCY_FILL_FPTR(_0), _PRINTCCY_FILL_FPTR(_1), _PRINTCCY_FILL_FPTR(_2), _PRINTCCY_FILL_FPTR(_3)
#define _PRINTCCY_FILL_FPTR6(fmt, _0, _1, _2, _3, _4) _PRINTCCY_FILL_FPTR(_0), _PRINTCCY_FILL_FPTR(_1), _PRINTCCY_FILL_FPTR(_2), _PRINTCCY_FILL_FPTR(_3), _PRINTCCY_FILL_FPTR(_4)
#define _PRINTCCY_FILL_FPTR7(fmt, _0, _1, _2, _3, _4, _5) _PRINTCCY_FILL_FPTR(_0), _PRINTCCY_FILL_FPTR(_1), _PRINTCCY_FILL_FPTR(_2), _PRINTCCY_FILL_FPTR(_3), _PRINTCCY_FILL_FPTR(_4), _PRINTCCY_FILL_FPTR(_5)

#define _PRINTCCY_CONCAT_HELPER(a, b) a##b 
#define _PRINTCCY_CONCAT(a, b) _PRINTCCY_CONCAT_HELPER(a, b)

#define _PRINTCCY_N_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define _PRINTCCY_N_ARGS(...)  _PRINTCCY_N_ARGS_HELPER( __VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0 )

#define _PRINTCCY_SETUP_PRINT(...) ( _PRINTCCY_CONCAT(_PRINTCCY_FILL_FPTR, _PRINTCCY_N_ARGS(__VA_ARGS__))( __VA_ARGS__ ),\
                                     _printccy.funcs_i -= _PRINTCCY_N_ARGS(__VA_ARGS__) - 1)
#define _PRINTCCY_CLEAN_UP_PRINT(...) (_printccy.funcs_i -= _PRINTCCY_N_ARGS(__VA_ARGS__) - 1)

// avoids the "right-hand operand of comma expression has no effect" warning
_PRINTCCY_MAYBE_UNUSED int _printccy_forward_int(int value) { return value; }

// third argument should be the format string
#define print(output_buffer, output_len, ...) _printccy_forward_int((\
                                                _PRINTCCY_SETUP_PRINT(__VA_ARGS__),\
                                                _printccy.printed_size = _printccy_print(output_buffer, output_len, __VA_ARGS__),\
                                                _PRINTCCY_CLEAN_UP_PRINT(__VA_ARGS__),\
                                                _printccy.printed_size ))

#define _PRINTCCY_SATURATING_SUB(a, b) ((a) >= (b) ? (a) - (b) : 0)

// second argument should be the format string
// the complexity comes from handling recursive printfb calls, if i could declare a local VA buffer then i could get rid of everything here
#define printfb(fb, ...) _printccy_forward_int((\
                           _printccy.lens[_printccy.lens_i] = print(0, 0, __VA_ARGS__), /*figure out the needed buffer size*/\
                           _printccy.buffer_i += _printccy.lens[_printccy.lens_i], /*offset the buffer for that amount*/\
                           _printccy.written_len = print(&_printccy.buffer[_printccy.buffer_i - _printccy.lens[_printccy.lens_i++]], _PRINTCCY_SATURATING_SUB((size_t)PRINTCCY_TEMP_BUFFER_SIZE, _printccy.buffer_i), __VA_ARGS__),\
                           _printccy.buffer_i -= _printccy.lens[--_printccy.lens_i],/*restore all the buffers*/\
                           fwrite(&_printccy.buffer[_printccy.buffer_i], sizeof(_printccy.buffer[0]), _printccy.written_len, fb),\
                           _printccy.written_len)) // return the written len

// first argument should be the format string
#define printout(...) printfb(stdout, __VA_ARGS__)

#endif // _PRINTCCY_H