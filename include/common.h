#ifndef rl_common_h
#define rl_common_h

#include <limits.h>
#include <errno.h>
#include <uchar.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>

// utility typedefs -----------------------------------------------------------
typedef unsigned short ushort_t;
typedef unsigned char byte_t;
typedef unsigned int flags_t;
typedef unsigned short instr_t;
typedef uintptr_t hash_t;
typedef void (*funcptr_t)(void);

// rascal typedefs ------------------------------------------------------------
// enum types -----------------------------------------------------------------
typedef enum {
  OKAY,
  USER_ERROR,
  EVAL_ERROR,
  RUNTIME_ERROR,
  SYSTEM_ERROR
} Status;

// Expression types -----------------------------------------------------------
typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double Num;
typedef bool Bool;
typedef char32_t Glyph;
typedef struct Obj Obj;

// Object types ---------------------------------------------------------------
typedef struct Type Type;
typedef struct Chunk Chunk;
typedef struct Alist Alist;
typedef struct Buf16 Buf16;
typedef struct Ref Ref;
typedef struct UpVal UpVal;
typedef struct Env Env;
typedef struct Port Port;
typedef struct Method Method;
typedef struct MethodTable MethodTable;
typedef struct Fun Fun;
typedef struct Str Str;
typedef struct Sym Sym;
typedef struct List List;

typedef union {
  Expr  expr;
  Num   num;
  Obj*  obj;
  Bool  boolean;
  Nul   nul;
  Glyph glyph;
} Val;

// Internal types
typedef struct Strings Strings;
typedef struct Objs Objs;
typedef struct Exprs Exprs;
typedef struct RlState RlState;

typedef void (*PrintFn)(Port* p, Expr x);
typedef hash_t (*HashFn)(Expr x);
typedef bool (*EgalFn)(Expr x, Expr y);
typedef void (*CloneFn)(RlState* rls, void* ob); // called to clone an object's owned pointers
typedef void (*TraceFn)(RlState* rls, void* ob);
typedef void (*FreeFn)(RlState* rls, void* ob);
typedef bool (*HasFn)(Type* tx, Type* ty);

// limits ---------------------------------------------------------------------
#define MAX_INTERN     512
#define MAX_ARITY      0x40000000 // largest 32-bit power of 2
#define MAX_SAVESTATES 512        // arbitrary, hard to imagine this many nested catch frames

// Prompt/messages
#define PROMPT  "rl>"
#define VERSION "%.2d.%.2d.%.2d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR   0
#define MINOR   9
#define PATCH   4
#define RELEASE "a"

// miscellaneous
#define RASCAL_DEBUG

// redefining annoyingly named builtins
#define clz         __builtin_clzl
#define popc        __builtin_popcountl
#define unreachable __builtin_unreachable

#endif
