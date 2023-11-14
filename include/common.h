#ifndef rl_common_h
#define rl_common_h

#include <uchar.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define DEBUG_RASCAL
#define DEBUG_READER

#define MAX_ARITY    0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2     0x0000800000000000UL
#define ARITY_WIDTH  48
#define QNAN         0x7ff8000000000000UL
#define SIGN         0x8000000000000000UL
#define UINT8_COUNT  (UINT8_MAX+1)
#define UINT16_COUNT (UINT16_MAX+1)

/* C types */
// utility types
typedef uintptr_t  word_t;
typedef uint64_t   hash_t;
typedef uint8_t    byte_t;
typedef void     (*funcptr_t)(void);
typedef uint32_t   flags_t;
typedef uint64_t   wflags_t;
typedef uint16_t   ushort_t;
typedef uint32_t   uint_t;
typedef uint64_t   idno_t;

// global context object type -- all global state stored here
typedef struct RlCtx RlCtx;

// basic rascal Vm types
typedef uintptr_t  Value; // tagged value
typedef struct Obj Obj;   // boxed object (includes type information)

// internal function pointer types
typedef hash_t (*HashFn)(Value x);
typedef bool   (*EgalFn)(Value x, Value y);
typedef int    (*RankFn)(Value x, Value y);
typedef int    (*OrdFn)(Value x, Value y);
typedef size_t (*SizeFn)(void* obj);
typedef void   (*TraceFn)(void* obj);
typedef void   (*FinalizeFn)(void* obj);
typedef void*  (*CloneFn)(void* obj);

/* globals */
extern struct RlCtx Ctx;

/* utility macros */
#define generic _Generic
#define unreachable __builtin_unreachable

#define cleanup(f) __attribute__ ((__cleanup__(f)))

#define generic2(method, dispatch, args...)     \
  generic((dispatch),                           \
          Value:method##_##val,                 \
          default:method##_##obj)(args)

#endif
