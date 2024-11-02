#ifndef rl_values_h
#define rl_values_h

#include "common.h"

/* Initial declarations for Rascal value types and basic APIs */
// general types
typedef word_t      Val;  // tagged word. May be immediate or an object
typedef struct Obj  Obj;  // Compound or large values are stored in a boxed object

// immediate types
typedef nullptr_t   Nul;
typedef bool        Bool;
typedef enum Label  Label;
typedef void*       Ptr;
typedef char32_t    Glyph;
typedef double      Real;

// user object types
typedef struct Port   Port;
typedef struct Func   Func;
typedef struct Cntl   Cntl;
typedef struct Sym    Sym;
typedef struct Bin    Bin;
typedef struct Pair   Pair;
typedef struct List   List;
typedef struct Vec    Vec;

typedef struct Map    Map;

// aliases for user object types
typedef struct Bin    Str;

// internal object types
typedef struct Buffer Buffer; // binary or text buffer (mostly for IO)
typedef struct Alist  Alist;  // generic alist
typedef struct Table  Table;  // mutable mapping
typedef struct Env    Env;    // environment
typedef struct Ref    Ref;    // environment mapping
typedef struct UpVal  UpVal;  // upvalue
typedef struct VNode  VNode;  // vector node
typedef struct MNode  MNode;  // map node
typedef struct State  State;  // global state object

// internal structure types (not accessible from Rascal)
typedef struct MTable MTable; // metatable (holds layout information and lifetime & comparison methods)
typedef struct TNode  TNode;  // table node

#endif
