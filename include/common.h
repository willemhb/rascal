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
typedef uintptr_t uptr_t;
typedef uintptr_t hash_t;
typedef void (*funcptr_t)(void);

// rascal typedefs ------------------------------------------------------------
// enum types -----------------------------------------------------------------
typedef enum {
  OKAY,
  USER_ERROR, // error originating from user (an eval error, but alerts handler that there's an error object to handle)
  EVAL_ERROR, // error originating in rascal code (eg type error, arity error, syntax error)
  RUNTIME_ERROR, // error originating in the runtime (eg stack overflow)
  SYSTEM_ERROR, // error originating in the OS (eg file not found, out of memory)
} Status;

#define NUM_ERRORS (SYSTEM_ERROR+1)

// Expression types -----------------------------------------------------------
typedef uptr_t Expr;
typedef nullptr_t Nul;
typedef double Num;
typedef bool Bool;
typedef char32_t Glyph;
typedef struct Obj Obj;
typedef struct Box Box; // transparent storage for boxed primitives

// Object types ---------------------------------------------------------------
// user types
typedef struct Type Type;
typedef struct Chunk Chunk;
typedef struct Ref Ref;
typedef struct UpVal UpVal;
typedef struct Env Env;
typedef struct Port Port;
typedef struct Ctl Ctl;
typedef struct Fun Fun;
typedef struct Method Method;
typedef struct MethodTable MethodTable;
typedef struct MTNode MTNode;
typedef struct Str Str;
typedef struct Sym Sym;
typedef struct List List;
typedef struct Tuple Tuple;
typedef struct Map Map;
typedef struct MapNode MapNode;
typedef struct LibHandle LibHandle;
typedef struct ForeignFn ForeignFn;

// Internal types
typedef struct Strings Strings;
typedef struct Table Table;
typedef struct Objs Objs;
typedef struct Exprs Exprs;
typedef struct CodeBuf CodeBuf;
typedef struct TextBuf TextBuf;
typedef struct BitVec BitVec;
typedef struct LineInfo LineInfo;
typedef struct RlState RlState;
typedef struct RlVm RlVm;
typedef struct Frame Frame;
typedef Expr* StackRef;
typedef Frame* FrameRef;

typedef void (*NativeFn)(RlState* rls, int offset, int argc);
typedef void (*PrintFn)(Port* p, Expr x);
typedef hash_t (*HashFn)(Expr x);
typedef bool (*EgalFn)(Expr x, Expr y);
typedef void (*CloneFn)(RlState* rls, void* ob); // called to clone an object's owned pointers
typedef void (*TraceFn)(RlState* rls, void* ob);
typedef void (*FreeFn)(RlState* rls, void* ob);
typedef bool (*HasFn)(Type* tx, Expr x);
typedef void (*BoxFn)(Box* box, Expr x);
typedef Expr (*UnboxFn)(Box* box);
typedef bool (*InitFn)(Box* src, void* spc);

// limits ---------------------------------------------------------------------
#define MAX_INTERN     512
#define MAX_ARITY      0x40000000 // largest 32-bit power of 2
#define MAX_FARGC      63         // largest number of fixed arguments supported
#define MAX_SAVESTATES 512        // arbitrary, hard to imagine this many nested catch frames

// Prompt/messages
#define PROMPT  "rl>"
#define VERSION "%.2d.%.2d.%.2d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR   0
#define MINOR   14
#define PATCH   1
#define RELEASE "a"

// miscellaneous
#define RASCAL_DEBUG

// redefining annoyingly named builtins
#define clz         __builtin_clzl
#define popc        __builtin_popcountl
#define unreachable __builtin_unreachable
#define c_generic   _Generic

#endif
