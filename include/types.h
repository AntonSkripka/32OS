#ifndef TYPES_H
#define TYPES_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

#ifdef __x86_64__
    typedef uint64_t uintptr_t;
    typedef uint64_t size_t;
#else
    typedef uint32_t uintptr_t;
    typedef uint32_t size_t;
#endif

typedef enum {
    false = 0,
    true = 1
} bool;

#endif