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
// internal enum types --------------------------------------------------------
// returned by API functions to indicate success or failure -------------------
typedef enum {
  NO_ERROR, /* okay */
  USER_ERROR, /* user generated error (recoverable) */   
  RUNTIME_ERROR, /* runtime generated error (recoverable) */
  SYSTEM_ERROR /* system generated error (fatal) */
} Status;

#define NUM_ERRORS (SYSTEM_ERROR+1)

// utility typedefs -----------------------------------------------------------
typedef unsigned short ushort_t;
typedef unsigned char byte_t;
typedef unsigned int flags_t;
typedef unsigned short instr_t;
typedef uintptr_t hash_t;
typedef uintptr_t idno_t;
typedef unsigned int count_t;
typedef unsigned long lcount_t;
typedef void (*funcptr_t)(void);
typedef uintptr_t uptr_t;

// rascal typedefs ------------------------------------------------------------
// Expression types -----------------------------------------------------------
typedef uptr_t Expr;
typedef nullptr_t Nul;
typedef double Num;
typedef struct Obj Obj;

// Object types ---------------------------------------------------------------
// Vm types -------------------------------------------------------------------
typedef struct Chunk Chunk;
typedef struct UpVal UpVal;
typedef struct Ref Ref;
typedef struct Cntl Cntl;

// Language types -------------------------------------------------------------
typedef struct Sym Sym;
typedef struct Fun Fun;
typedef struct Type Type;

// immutable aggregate types --------------------------------------------------
typedef struct List List;
typedef struct Tuple Tuple;
typedef struct Map Map;

// mutable aggregate types ----------------------------------------------------
typedef struct Stack Stack;
typedef struct Table Table;
typedef struct Buffer Buffer;

// VM types -------------------------------------------------------------------
// struct types ---------------------------------------------------------------
typedef struct SymbolTable SymbolTable;
typedef struct CallState CallState;
typedef struct GlobalState GlobalState;
typedef struct RascalState RascalState;
typedef struct ObjAPI ObjAPI;
typedef struct IStack IStack; /* internal stack */
typedef struct ITable ITable; /* internal table */
typedef struct IBuffer IBuffer; /* internal buffer */

// function pointer types -----------------------------------------------------
typedef Status (*RascalIFn)(RascalState* s);
typedef size_t (*SizeFn)(void* ob);
typedef void*  (*AllocFn)(flags_t flags, size_t n);
typedef void   (*CloneFn)(void* ob); // called to clone object's owned pointers
typedef void   (*TraceFn)(void* ob);
typedef void   (*FreeFn)(void* ob);

// scalar types ---------------------------------------------------------------
typedef Expr* StackRef;
typedef CallState* CSRef;

typedef union {
  Expr expr;
  Num num;
  Obj* obj;
  Nul nul;
} Val;

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

#endif
