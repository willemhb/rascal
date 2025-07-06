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
#define MAX_INTERN     512        // interned string length limit
#define MAX_ARITY      0x40000000 // largest (signed?) 32-bit power of 2

// magic numbers
#define FRAME_SIZE 3

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

// C types --------------------------------------------------------------------
// utility typedefs -----------------------------------------------------------
typedef unsigned short ushort_t;
typedef unsigned char  byte_t;
typedef unsigned int   flags_t;
typedef unsigned short instr_t;
typedef uintptr_t      hash_t;
typedef void (*funcptr_t)(void);
typedef uintptr_t      uptr_t;

// rascal typedefs ------------------------------------------------------------
// Expression types -----------------------------------------------------------
typedef uptr_t     Expr;
typedef nullptr_t  Nul;
typedef double     Num;
typedef bool       Bool;
typedef char32_t   Glyph;
typedef struct Obj Obj;

// Object types ---------------------------------------------------------------
// Vm types -------------------------------------------------------------------
typedef struct Chunk   Chunk;
typedef struct Ref     Ref;
typedef struct Env     Env;
typedef struct UpVal   UpVal;

// Language types -------------------------------------------------------------
typedef struct Sym  Sym;
typedef struct Fun  Fun;
typedef struct Port Port;

// Immutable aggregate types --------------------------------------------------
typedef struct Tuple Tuple;
typedef struct List  List;
typedef struct Map   Map;
typedef struct Str   Str;
typedef struct Bin   Bin;

// Mutable aggretate types ----------------------------------------------------
typedef struct MutTuple MutTuple;
typedef struct MutList  MutList;
typedef struct MutMap   MutMap;
typedef struct MutStr   MutStr;
typedef struct MutBin   MutBin;

// Internal types (used in map and list implementations) ----------------------
typedef struct ListNode ListNode;
typedef struct ListLeaf ListLeaf;
typedef struct MapNode  MapNode;
typedef struct MapLeaf  MapLeaf;

typedef union {
  Expr  expr;
  Num   num;
  Obj*  obj;
  Bool  boolean;
  Nul   nul;
  Glyph glyph;
} Val;

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

#endif
