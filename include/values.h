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
typedef char32_t    Glyph;
typedef double      Num;
typedef sint32      Small;
typedef void*       Ptr;

// callable types
typedef struct NativeFn NativeFn;
typedef struct UserFn   UserFn;

// reference types
typedef struct Sym      Sym;

// IO types
typedef struct Port     Port;
typedef struct Str      Str;

// user collection types
typedef struct Pair   Pair;
typedef struct Buffer Buffer;
typedef struct Alist  Alist;
typedef struct Table  Table;

// miscellaneous internal types
typedef struct Env      Env;
typedef struct Ref      Ref;

#endif
