#ifndef _PRINTCCY_H
#define _PRINTCCY_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

// should return the number of characters printed
typedef int32_t(_printccy_print_func)(char*, size_t, va_list*, const char*, size_t);

#ifndef PRINTCCY_ESCAPE_CHARACTER
#define PRINTCCY_ESCAPE_CHARACTER '%'
#endif // PRINTCCT_ESCAPE_CAHRACTER

#ifndef PRINTCCY_TEMP_BUFFER_SIZE
#define PRINTCCY_TEMP_BUFFER_SIZE (2<<12)
#endif // PRINTCCY_TEMP_BUFFER_SIZE

static _Thread_local struct {
    // print
    _printccy_print_func* funcs[128]; // holds the function pointers the printing functions
    size_t funcs_i;

    int32_t printed_size;

    // printfb
    char buffer[PRINTCCY_TEMP_BUFFER_SIZE];
    size_t buffer_i;

    int32_t lens[100];
    int32_t lens_i;
    int32_t written_len;
} _printccy;

#if __STDC_VERSION__ >= 202311L
#elif __STDC_VERSION__ >= 201710L
#elif __STDC_VERSION__ >= 201112L
#else
#error Printccy cannot be used with this compiler version
#endif

#if __STDC_VERSION__ >= 202311L
    #define _PRINTCCY_MAYBE_UNUSED [[maybe_unused]]
    #define _PRINTCCY_BOOL bool
#else
    #define _PRINTCCY_MAYBE_UNUSED
    #define _PRINTCCY_BOOL _Bool
#endif

// define it youtself as:
// #define PRINTCCY_CUSTOM_TYPES PRINTCCY_BASE_TYPES, my_type: print_my_type, my_type2: ...
#define PRINTCCY_BASE_TYPES \
    int32_t: printccy_print_int,\
    uint32_t: printccy_print_uint,\
    float: printccy_print_float,\
    double: printccy_print_double,\
    int64_t: printccy_print_long_long,\
    uint64_t: printccy_print_unsigned_long_long,\
    char: printccy_print_char,\
    const char*: printccy_print_char_ptr,\
    char*: printccy_print_char_ptr,\
    _PRINTCCY_BOOL: printccy_print_bool

#ifdef PRINTCCY_CUSTOM_TYPES
    #define PRINTCCY_TYPES PRINTCCY_BASE_TYPES, PRINTCCY_CUSTOM_TYPES
#else
    #define PRINTCCY_TYPES PRINTCCY_BASE_TYPES
#endif // PRINTCCY_CUSTOM_TYPES

#define _PRINTCCY_MATCH_ARG_TYPE(X) _Generic((X), PRINTCCY_TYPES, default: 0)

// copies to the output buffer from the fmt string and calls above function pointers when encountering a {} with arguments inside the {}
static inline int32_t _printccy_print(char* output, int32_t output_len, int32_t n_args, const char* fmt, ...)
{
    va_list args; va_start(args, fmt);

    int32_t bytes_written = 0, nth_arg = 0;
    char c, is_escape = 0;
    int32_t funcs_i = _printccy.funcs_i - n_args;
    while ((c = *(fmt++)))
    {
        if (is_escape || (c != '{' && c != PRINTCCY_ESCAPE_CHARACTER))
        {
            is_escape = 0;
            if(output && bytes_written < output_len - 1) *(output + bytes_written) = c;
            bytes_written++;
            continue;
        }

        if (c == PRINTCCY_ESCAPE_CHARACTER)
        {
            is_escape = 1;
            continue;
        }

        const char* start = fmt;
        while (*fmt && *fmt != '}') { fmt++; }
        if (++nth_arg <= n_args)
        {
            _printccy_print_func* fptr = _printccy.funcs[funcs_i++];
            const int32_t remaining = output_len - bytes_written;
            const int32_t n = fptr ? fptr(output && remaining > 0 ? output + bytes_written : 0, remaining > 0 ? (size_t)remaining : 0, &args, start, fmt - start) : 0;
            if (n > 0) bytes_written += n;
        }
        if (*fmt) fmt++;
    }
    if (output && output_len > 0) output[bytes_written < output_len ? bytes_written : output_len - 1] = '\0';

    va_end(args);
    return bytes_written;
}

#define _PRINTCCY_ASSERT(expr, msg) sizeof(struct { _Static_assert(expr != 0, msg); int32_t _dummy; })

#define _PRINTCCY_IS_INT(X) _Generic((X), int: 0, default: 1)
#define _PRINTCCY_GET_FPTR_AND_ASSERT(X) ((void)_PRINTCCY_ASSERT(_PRINTCCY_IS_INT(_PRINTCCY_MATCH_ARG_TYPE(X)),  "unsupported type of variable " #X), _PRINTCCY_MATCH_ARG_TYPE(X))

#define _PRINTCCY_FILL_FPTR(X) _printccy.funcs[_printccy.funcs_i++] = _PRINTCCY_GET_FPTR_AND_ASSERT(X)

#define _PRINTCCY_FILL_FPTR1(fmt) (void)0
#define _PRINTCCY_FILL_FPTR2(fmt, _0) _PRINTCCY_FILL_FPTR(_0)
#define _PRINTCCY_FILL_FPTR3(fmt, _0, _1) _PRINTCCY_FILL_FPTR2(fmt, _0), _PRINTCCY_FILL_FPTR(_1)
#define _PRINTCCY_FILL_FPTR4(fmt, _0, _1, _2) _PRINTCCY_FILL_FPTR3(fmt, _0, _1), _PRINTCCY_FILL_FPTR(_2)
#define _PRINTCCY_FILL_FPTR5(fmt, _0, _1, _2, _3) _PRINTCCY_FILL_FPTR4(fmt, _0, _1, _2), _PRINTCCY_FILL_FPTR(_3)
#define _PRINTCCY_FILL_FPTR6(fmt, _0, _1, _2, _3, _4) _PRINTCCY_FILL_FPTR5(fmt, _0, _1, _2, _3), _PRINTCCY_FILL_FPTR(_4)
#define _PRINTCCY_FILL_FPTR7(fmt, _0, _1, _2, _3, _4, _5) _PRINTCCY_FILL_FPTR6(fmt, _0, _1, _2, _3, _4), _PRINTCCY_FILL_FPTR(_5)
#define _PRINTCCY_FILL_FPTR8(fmt, _0, _1, _2, _3, _4, _5, _6) _PRINTCCY_FILL_FPTR7(fmt, _0, _1, _2, _3, _4, _5), _PRINTCCY_FILL_FPTR(_6)
#define _PRINTCCY_FILL_FPTR9(fmt, _0, _1, _2, _3, _4, _5, _6, _7) _PRINTCCY_FILL_FPTR8(fmt, _0, _1, _2, _3, _4, _5, _6), _PRINTCCY_FILL_FPTR(_7)
#define _PRINTCCY_FILL_FPTR10(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8) _PRINTCCY_FILL_FPTR9(fmt, _0, _1, _2, _3, _4, _5, _6, _7), _PRINTCCY_FILL_FPTR(_8)
#define _PRINTCCY_FILL_FPTR11(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9) _PRINTCCY_FILL_FPTR10(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8), _PRINTCCY_FILL_FPTR(_9)
#define _PRINTCCY_FILL_FPTR12(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) _PRINTCCY_FILL_FPTR11(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9), _PRINTCCY_FILL_FPTR(_10)
#define _PRINTCCY_FILL_FPTR13(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) _PRINTCCY_FILL_FPTR12(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10), _PRINTCCY_FILL_FPTR(_11)
#define _PRINTCCY_FILL_FPTR14(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) _PRINTCCY_FILL_FPTR13(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11), _PRINTCCY_FILL_FPTR(_12)
#define _PRINTCCY_FILL_FPTR15(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) _PRINTCCY_FILL_FPTR14(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12), _PRINTCCY_FILL_FPTR(_13)
#define _PRINTCCY_FILL_FPTR16(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) _PRINTCCY_FILL_FPTR15(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13), _PRINTCCY_FILL_FPTR(_14)
#define _PRINTCCY_FILL_FPTR17(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) _PRINTCCY_FILL_FPTR16(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14), _PRINTCCY_FILL_FPTR(_15)
#define _PRINTCCY_FILL_FPTR18(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) _PRINTCCY_FILL_FPTR17(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15), _PRINTCCY_FILL_FPTR(_16)
#define _PRINTCCY_FILL_FPTR19(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17) _PRINTCCY_FILL_FPTR18(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16), _PRINTCCY_FILL_FPTR(_17)
#define _PRINTCCY_FILL_FPTR20(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18) _PRINTCCY_FILL_FPTR19(fmt, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17), _PRINTCCY_FILL_FPTR(_18)

#define _PRINTCCY_CONCAT_HELPER(a, b) a##b
#define _PRINTCCY_CONCAT(a, b) _PRINTCCY_CONCAT_HELPER(a, b)

#define _PRINTCCY_N_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define _PRINTCCY_N_ARGS(...)  _PRINTCCY_N_ARGS_HELPER( __VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 )

#define _PRINTCCY_SETUP_PRINT(...) ( _PRINTCCY_CONCAT(_PRINTCCY_FILL_FPTR, _PRINTCCY_N_ARGS(__VA_ARGS__))( __VA_ARGS__ ) )
#define _PRINTCCY_CLEAN_UP_PRINT(...) (_printccy.funcs_i -= _PRINTCCY_N_ARGS(__VA_ARGS__) - 1)

// avoids the "right-hand operand of comma expression has no effect" warning
static inline _PRINTCCY_MAYBE_UNUSED int32_t _printccy_forward_int(int32_t value) { return value; }

// third argument should be the format string
#define print(output_buffer, output_len, ...) _printccy_forward_int((\
                                                _PRINTCCY_SETUP_PRINT(__VA_ARGS__),\
                                                _printccy.printed_size = _printccy_print(output_buffer, output_len, _PRINTCCY_N_ARGS(__VA_ARGS__) - 1, __VA_ARGS__),\
                                                _PRINTCCY_CLEAN_UP_PRINT(__VA_ARGS__),\
                                                _printccy.printed_size ))

#define _PRINTCCY_SATURATING_SUB(a, b) ((a) >= (b) ? (a) - (b) : 0)

// second argument should be the format string
// the complexity comes from handling recursive printfb calls, if i could declare a local VA buffer then i could get rid of everything here
#define printfb(fb, ...) _printccy_forward_int((\
                           _printccy.lens[_printccy.lens_i] = print(0, 0, __VA_ARGS__), /*figure out the needed buffer size*/\
                           _printccy.lens[_printccy.lens_i] = (size_t)_printccy.lens[_printccy.lens_i] < _PRINTCCY_SATURATING_SUB((size_t)PRINTCCY_TEMP_BUFFER_SIZE, _printccy.buffer_i) ? _printccy.lens[_printccy.lens_i] : (int32_t)_PRINTCCY_SATURATING_SUB((size_t)PRINTCCY_TEMP_BUFFER_SIZE, _printccy.buffer_i),\
                           _printccy.buffer_i += _printccy.lens[_printccy.lens_i++], /*offset the buffer for that amount*/\
                           _printccy.written_len = print(&_printccy.buffer[_printccy.buffer_i - _printccy.lens[_printccy.lens_i - 1]], _PRINTCCY_SATURATING_SUB((size_t)PRINTCCY_TEMP_BUFFER_SIZE, _printccy.buffer_i - _printccy.lens[_printccy.lens_i - 1]), __VA_ARGS__),\
                           _printccy.buffer_i -= _printccy.lens[--_printccy.lens_i],/*restore all the buffers*/\
                           _printccy.written_len = (size_t)_printccy.written_len < _PRINTCCY_SATURATING_SUB((size_t)PRINTCCY_TEMP_BUFFER_SIZE, _printccy.buffer_i + 1) ? _printccy.written_len : (int32_t)_PRINTCCY_SATURATING_SUB((size_t)PRINTCCY_TEMP_BUFFER_SIZE, _printccy.buffer_i + 1),\
                           fwrite(&_printccy.buffer[_printccy.buffer_i], sizeof(_printccy.buffer[0]), _printccy.written_len, fb),\
                           _printccy.written_len)) // return the written len


// first argument should be the format string
#define printout(...) printfb(stdout, __VA_ARGS__)

// filters out the * symbol
#define _PRINTCCY_COPY_ARGS(to, from, len) do { size_t j = 0; for (size_t i = 0; i < (len); i++) { char c = (from)[i]; if(c != '*') (to)[j++] = c; } } while(0)
#define _PRINTCCY_INIT_EMPTY_BUFFER(name, len) char name[len]; for (size_t i = 0; i < (len); i++) (name)[i] = 0;

#define PRINTCCY_PRINT_FUNC_DEF(name) int32_t name(char* output, size_t output_len, va_list* list, const char* args, size_t args_len)

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_int) {
    const int32_t val = va_arg(*list, int32_t);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 'd';
    return snprintf(output, output_len, buf, val);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_uint) {
    const uint32_t val = va_arg(*list, uint32_t);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 'u';
    return snprintf(output, output_len, buf, val);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_bool) {
    const int val = va_arg(*list, int);
    return print(output, output_len, val ? "true" : "false");
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_long_long) {
    const int64_t val = va_arg(*list, int64_t);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 3));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          _PRINTCCY_COPY_ARGS(buf + 1, "lld", 3);
    return snprintf(output, output_len, buf, (long long)val);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_unsigned_long_long) {
    const uint64_t val = va_arg(*list, uint64_t);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 3));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          _PRINTCCY_COPY_ARGS(buf + 1, "llu", 3);
    return snprintf(output, output_len, buf, (unsigned long long)val);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_double) {
    const double val = va_arg(*list, double);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 'f';
    return snprintf(output, output_len, buf, val);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_float) {
    return printccy_print_double(output, output_len, list, args, args_len);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_char) {
    int val = va_arg(*list, int);
    return snprintf(output, output_len, "%c", val);
}

static inline PRINTCCY_PRINT_FUNC_DEF(printccy_print_char_ptr) {
    const char* val = va_arg(*list, char*);
    _PRINTCCY_INIT_EMPTY_BUFFER(buf, 2 + (args_len ? args_len : 1));
    buf[0] = '%';
    if (args_len) _PRINTCCY_COPY_ARGS(buf + 1, args, args_len);
    else          buf[1] = 's';
    return snprintf(output, output_len, buf, val);
}

#endif // _PRINTCCY_H
