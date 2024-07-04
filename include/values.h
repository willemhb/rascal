#ifndef rl_values_h
#define rl_values_h

#include "common.h"

/* Typedefs for Rascal value types. */

/* Initial declarations for Rascal value types and basic APIs */
// general types
typedef word_t     Val;  // tagged word. May be immediate or an object
typedef struct Obj Obj; // Compound or large values are stored in a boxed object

// miscellaneous immediate types
typedef nullptr_t Nul;
typedef bool      Bool;
typedef void*     Ptr;
typedef funcptr_t FunPtr;

// metaobject types
typedef struct Type Type;

// function types
typedef struct Func   Func;      // common function header
typedef struct Proto  Proto;
typedef struct PrimFn PrimFn;
typedef struct GenFn  GenFn;
typedef struct MT     MT;
typedef struct Cntl   Cntl;

// identifier and environment types
typedef struct Sym   Sym;
typedef struct Env   Env;
typedef struct Ref   Ref;
typedef struct UpVal UpVal;

// text, binary, and IO types
typedef char32_t    Glyph;
typedef struct Port Port;
typedef struct Str  Str;
typedef struct Bin  Bin;
typedef struct MStr MStr;
typedef struct MBin MBin;
typedef struct RT   RT;

// numeric types
typedef word_t      Arity; // 48-bit unsigned integer (can hold any valid hash or size)
typedef int         Small;
typedef double      Real;

// list, pair, and sequence types
typedef struct Pair  Pair;
typedef struct List  List;
typedef struct MPair MPair;
typedef struct MList MList;

// array types
typedef struct Vec   Vec;
typedef struct Vec   Struct;
typedef struct VNode VNode;
typedef struct MVec  MVec;
typedef struct Alist Alist;

// table types
typedef struct Map    Map;
typedef struct MNode  MNode;
typedef struct Map    Record;
typedef struct Map    Set;
typedef struct MMap   MMap;
typedef struct MMap   MSet;
typedef struct SCache SCache;
typedef struct EMap   EMap;
typedef struct NSMap  NSMap;

// vm types
typedef struct State State; // global state object
typedef struct Proc  Proc;  // single Rascal process

#endif
