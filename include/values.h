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
typedef void*       Ptr;
typedef char32_t    Glyph;
typedef double      Real;

// user object types
typedef struct Port   Port;
typedef struct Func   Func;
typedef struct Sym    Sym;
typedef struct Bin    Bin;
typedef struct Pair   Pair;
typedef struct List   List;
typedef struct Vec    Vec;
typedef struct Map    Map;

// aliases for user object types
typedef struct Bin    Str;
typedef struct Vec    Struct;
typedef struct Bin    Buffer;
typedef struct Map    Record;
typedef struct Vec    Alist;
typedef struct Map    Table;

// internal object types
typedef struct UpVal  UpVal;  // upvalue
typedef struct VNode  VNode;  // vector node
typedef struct MNode  MNode;  // map node

// internal structure types (not accessible from Rascal)

#endif
