#ifndef rl_common_h
#define rl_common_h

/**
 *
 * Common system headers and typedefs.
 *
 **/

// headers --------------------------------------------------------------------
#include <limits.h>
#include <errno.h>
#include <uchar.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>

// macros ---------------------------------------------------------------------
// limits
#define MAX_INTERN 512 // interned string length limit
#define MAX_ARITY 0x40000000 // largest (signed?) 32-bit power of 2

// Prompt/messages
#define PROMPT  "rascal>"
#define VERSION "%.2d.%.2d.%.2d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR 0
#define MINOR 1
#define PATCH 0
#define RELEASE "a"

// miscellaneous
// #define RASCAL_DEBUG

// redefining annoyingly named builtins
#define clz         __builtin_clzl
#define popc        __builtin_popcountl
#define unreachable __builtin_unreachable

#define generic _Generic

// C types --------------------------------------------------------------------
// utility typedefs -----------------------------------------------------------
typedef unsigned short ushort_t;
typedef unsigned char byte_t;
typedef unsigned int flags_t;
typedef unsigned short instr_t;
typedef uintptr_t hash_t;
typedef void (*funcptr_t)(void);
typedef uintptr_t uptr_t;

// rascal typedefs ------------------------------------------------------------
// Expression types -----------------------------------------------------------
typedef uptr_t Expr;
typedef nullptr_t Nul;
typedef double Num;
typedef bool Bool;
typedef char32_t Glyph;
typedef struct Obj Obj;

// Object types ---------------------------------------------------------------
// Vm types -------------------------------------------------------------------
typedef struct Chunk Chunk;
typedef struct Env   Env;
typedef struct Ref   Ref;
typedef struct UpVal UpVal;
typedef struct Cntl  Cntl;

// Language types -------------------------------------------------------------
typedef struct Sym Sym;
typedef struct Fun Fun;
typedef struct Port Port;

// Immutable aggregate types --------------------------------------------------
typedef struct List List;
typedef struct Tuple Tuple;
typedef struct Map Map;
typedef struct Bin Bin;

// Mutable aggretate types ----------------------------------------------------
typedef struct MutTuple MutTuple;
typedef struct MutList MutList;
typedef struct MutMap MutMap;
typedef struct MutBin MutBin;

// Internal types (used in map implementation) --------------------------------
typedef struct MapNode MapNode;
typedef struct MapLeaf MapLeaf;

// VM types -------------------------------------------------------------------
// struct types ---------------------------------------------------------------
typedef struct Objects Objects;
typedef struct CallState CallState;
typedef struct GlobalState GlobalState;
typedef struct RascalState RascalState;

// function pointer types -----------------------------------------------------
typedef int (*RascalIFn)(RascalState* s);

// scalar types ---------------------------------------------------------------
typedef Expr* StackRef;
typedef CallState* CSRef;

typedef union {
  Expr expr;
  Num num;
  Obj* obj;
  Bool boolean;
  Nul nul;
  Glyph glyph;
} Val;

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

#endif
