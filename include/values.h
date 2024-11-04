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
typedef void*       Ptr;

// callable types
typedef struct Cntl     Cntl;
typedef struct NativeFn NativeFn;
typedef struct UserFn   UserFn;

// reference types
typedef struct Sym      Sym;

// IO types
typedef struct Stream   Stream;
typedef struct Bin      Bin;

// uxer collection types
typedef struct Cons   Cons;
typedef struct Vec    Vec;
typedef struct Map    Map;

#endif
