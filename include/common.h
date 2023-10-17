#ifndef rascal_common_h
#define rascal_common_h

#include <stdlib.h>
#include <stdio.h>
#include <uchar.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>

#define DEBUG_RASCAL
#define DEBUG_READER

#define MAX_ARITY    0x0000ffffffffffffUL // also maximum hash value
#define MAX_POW2     0x0000800000000000UL
#define ARITY_WIDTH  48
#define QNAN         0x7ff8000000000000UL
#define SIGN         0x8000000000000000UL
#define UINT8_COUNT  (UINT8_MAX+1)
#define UINT16_COUNT (UINT16_MAX+1)

// utility types
typedef uint64_t   hash_t;
typedef uint8_t    byte_t;
typedef void     (*funcptr_t)(void);
typedef uint32_t   flags_t;

/**
 *
 * Used for conversion from double -> uint64_t when working with NaN-boxed values.
 *
 * Notionally useful for extracting different parts of the floating point
 * representation as well.
 * 
 **/

typedef union {
  double    number;
  uintptr_t word;
  struct {
    uintptr_t frac : 52;
    uintptr_t expt : 11;
    uintptr_t sign :  1;
  } bits;
} IEEE_754_64;

static inline uintptr_t doubleToWord(double number) {
  return ((IEEE_754_64)number).word;
}

static inline double wordToDouble(uintptr_t word) {
  return ((IEEE_754_64)word).number;
}

// other C types
// VM state objects
typedef struct Context Context;   // saved execution state
typedef struct Frame   Frame;     // call frame
typedef struct Vtable  Vtable;    // stores layout information and core APIs for data types
typedef struct Vm      Vm;        // all essential execution state

// basic tagged types
// basic value types
typedef uintptr_t       Value;    // standard tagged value representation (NaN boxed)
typedef struct Obj      Obj;      // generic object

// core rascal types
// immediate types
typedef double          Float;    // ieee754-64 floating point number
typedef uintptr_t       Arity;    // 48-bit unsigned integer
typedef int             Small;    // 32-bit integer
typedef bool            Boolean;  // boolean
typedef char            Glyph;    // unicode codepoint
typedef void*           Pointer;  // arbitrary C pointer
typedef funcptr_t       FuncPtr;  // C function pointer

// object types
// miscellaneous
typedef struct Symbol   Symbol;   // first-class identifier type
typedef struct Type     Type;     // first-class representation of a rascal type
typedef struct Stream   Stream;   // IO stream
typedef struct Big      Big;      // arbitrary precision integer

// utility array types
typedef struct Buffer8   Buffer8;
typedef struct Buffer16  Buffer16;
typedef struct Buffer32  Buffer32;
typedef struct Binary8   Binary8;
typedef struct Binary16  Binary16;
typedef struct Binary32  Binary32;
typedef struct Alist     Alist;
typedef struct Objects   Objects;

// utility table types
typedef struct Table       Table;
typedef struct SymbolTable SymbolTable;
typedef struct NameSpace   NameSpace;

// immutable collection types
typedef struct Bits     Bits;     // unencoded binary data
typedef struct String   String;   // encoded binary data
typedef struct Tuple    Tuple;    // a simple, fixed-size imutable collection
typedef struct List     List;     // classic linked list
typedef struct Vector   Vector;   // clojure-like vector
typedef struct VecNode  VecNode;
typedef struct VecLeaf  VecLeaf;
typedef struct Map      Map;      // clojure-like hashmap (with reader support for sets)
typedef struct MapNode  MapNode;
typedef struct MapLeaf  MapLeaf;

// function & method types
typedef struct Function    Function;    // generic function object
typedef struct MethodTable MethodTable; // core of multimethod implementation
typedef struct MethodMap   MethodMap;   // root for single method tree
typedef struct MethodNode  MethodNode;  // internal method table node
typedef struct Method      Method;      // a single method table entry
typedef struct Native      Native;      // native function
typedef struct Closure     Closure;     // packages a chunk/namespace with names

// execution and control
typedef struct Chunk       Chunk;
typedef struct Control     Control;     // reified continuation

// names & environments
typedef struct Binding     Binding;
typedef struct Scope       Scope;       // complete naming context, including globals and file-scoped variables
typedef struct Environment Environment; // a reified naming context, including values
typedef struct UpValue     UpValue;     // a captured local binding

// enum & flag types
typedef enum {
  READER_READY, READER_EXPRESSION, READER_DONE, READER_ERROR,
} ReadState;

typedef enum {
  BINARY, ASCII, LATIN1, UTF8, UTF16, UTF32,
} Encoding;

// function pointer types
typedef Value    (*NativeFn)(size_t n, Value* a);
typedef size_t   (*CompileFn)(Vm* vm, List* form);
typedef void     (*ReadFn)(Vm* vm, int dispatch);
typedef void     (*TraceFn)(void* p);
typedef void     (*FreeFn)(void* p);
typedef hash_t   (*HashFn)(Value p);
typedef size_t   (*SizeFn)(void* p);
typedef bool     (*EgalFn)(Value x, Value y);
typedef int      (*OrdFn)(Value x, Value y);

// miscellaneous helper macros
#define generic2(method, dispatch, args...)     \
  _Generic((dispatch),                          \
           Value:method##Val,                   \
           default:method##Obj)(args)

#endif
