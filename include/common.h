#ifndef rl_common_h
#define rl_common_h

#include <limits.h>
#include <uchar.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>

// utility typedefs
typedef unsigned short ushort_t;
typedef unsigned char  byte_t;
typedef unsigned int   flags_t;
typedef unsigned short instr_t;
typedef uintptr_t      hash_t;

// limits
#define MAX_INTERN 512
#define MAX_ARITY  0x40000000 // largest 32-bit power of 2

// magic numbers
#define FRAME_SIZE 4

// Prompt/messages
#define PROMPT  "rl>"
#define VERSION "%.2d.%.2d.%.2d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR   0
#define MINOR   8
#define PATCH   0
#define RELEASE "a"

// miscellaneous
// #define RASCAL_DEBUG

// redefining annoyingly named builtins
#define clz         __builtin_clzl
#define popc        __builtin_popcountl
#define unreachable __builtin_unreachable

#endif
