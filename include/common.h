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

// runtime & vm structure types
typedef struct GcFrame       GcFrame;
typedef struct ReaderFrame   ReaderFrame;
typedef struct CompilerFrame CompilerFrame;
typedef struct CallFrame     CallFrame;

typedef struct RlHeap        RlHeap;
typedef struct RlReader      RlReader;
typedef struct RlCompiler    RlCompiler;
typedef struct RlInterpreter RlInterpreter;

// basic rascal Vm types
typedef uintptr_t   Value; // tagged value
typedef struct Obj  Obj;   // boxed object (includes type information)
typedef struct Type Type;  // object representing a rascal type

/* internal function pointer types */
// called to compute the value part of an object's hash
typedef hash_t (*HashFn)(Value x);

// called for non-trival equality tests (does not attempt to detect cycles)
typedef bool   (*EgalFn)(Value x, Value y);

// called for deep comparison of two totally orderable values (does not attempt to detect cycles)
typedef int    (*OrdFn)(Value x, Value y);

// called for deep comparison of two partially orderable values (does not attempt to detect cycles)
typedef int    (*RankFn)(Value x, Value y);

// called to determine to the object size (only specialized if an object )
typedef size_t (*SizeFn)(void* obj);

// called to trace an object's owned pointers
typedef void   (*TraceFn)(void* obj);

// called to handle freeing of an object's owned pointers or other cleanup (closing files, etc)
typedef void   (*FinalizeFn)(void* obj);

// called to create a copy of an object (only specialized if an object has owned pointers that shouldn't be shared)
typedef void*  (*CloneFn)(void* obj);

// called to handle freeing of an object
typedef void   (*DeallocFn)(void* obj);

// called to create a new instance (specialized so that objects can maintain a free list or other optimized allocation scheme)
typedef void*  (*AllocFn)(Type* type, flags_t fl, size_t extra);

// builtin reader function - returns -1 on error, 1 when an expression has been read, and 0 when no expression has been read
typedef int    (*ReadFn)(int dispatch);

/* utility macros */
#define generic _Generic
#define unreachable __builtin_unreachable

#define cleanup(f) __attribute__ ((__cleanup__(f)))

#define generic2(method, dispatch, args...)     \
  generic((dispatch),                           \
          Value:method##_##val,                 \
          default:method##_##obj)(args)

#endif
