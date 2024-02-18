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
typedef struct CallFrame     CallFrame;

typedef struct RlHeap        RlHeap;         // Memory state
typedef struct RlReader      RlReader;       // Reader state
typedef struct RlCompiler    RlCompiler;     // Compiler state
typedef struct RlContext     RlContext;      // Environment state
typedef struct RlInterpreter RlInterpreter;  // Execution state

// basic rascal Vm types
typedef uintptr_t   Value; // tagged value
typedef struct Obj  Obj;   // boxed object (includes type information)
typedef struct Type Type;  // object representing a rascal type

/* internal function pointer types */
typedef hash_t (*HashFn)(Value x);                               // compute an object's raw hash
typedef bool   (*EgalFn)(Value x, Value y);                      // non-trival equality tests (does not attempt to detect cycles)
typedef int    (*OrdFn)(Value x, Value y);                       // deep comparison of two totally orderable values (does not attempt to detect cycles)
typedef int    (*RankFn)(Value x, Value y);                      // deep comparison of two partially orderable values (does not attempt to detect cycles)
typedef size_t (*SizeFn)(void* obj);                             // determine an object's size
typedef void   (*TraceFn)(void* obj);                            // trace an object's owned pointers
typedef void   (*FinalizeFn)(void* obj);                         // do cleanup when an object is no longer reachable
typedef void*  (*CloneFn)(void* obj);                            // create a copy of an object (only specialized if an object has owned pointers that shouldn't be shared)
typedef void   (*DeallocFn)(void* obj);                          // manage heap when an object is no longer reachable
typedef void*  (*AllocFn)(Type* type, flags_t fl, size_t extra); // called to create a new instance
typedef int    (*ReadFn)(int dispatch);                          // builtin reader function
typedef Value  (*NativeFn)(size_t n, Value* args);               // signature for native function pointers



/* utility macros */
#define generic _Generic
#define unreachable __builtin_unreachable

#define cleanup(f) __attribute__ ((__cleanup__(f)))

#define generic2(method, dispatch, args...)     \
  generic((dispatch),                           \
          Value:method##_##val,                 \
          default:method##_##obj)(args)

#endif
