#ifndef rl_values_h
#define rl_values_h

#include "common.h"

/* Initial declarations for Rascal value types and basic APIs */
// general types
typedef word_t      Val;    // tagged word. May be immediate or an object
typedef struct Obj  Obj;    // Compound or large values are stored in a boxed object
typedef struct Fn   Fn;     // Common function header type

// immediate types
typedef nullptr_t Nul;
typedef bool      Bool;
typedef char32_t  Glyph;
typedef double    Num;
typedef sint32    Small;
typedef void*     Ptr;

// callable types
typedef struct PrimFn PrimFn;
typedef struct UserFn UserFn;

// reference types
typedef struct Sym Sym;

// IO types
typedef struct Port Port;
typedef struct Str  Str;

// persistent collection types
typedef struct List List;
typedef struct Vec  Vec;

// mutable collection types
typedef struct Pair   Pair;
typedef struct Buffer Buffer;
typedef struct Alist  Alist;
typedef struct Table  Table;

// internal environment types
typedef struct Ns  Ns;
typedef struct Env Env;
typedef struct Ref Ref;
typedef struct Upv Upv;

// internal node types
typedef struct VNode VNode;

// lots generics with this particular pattern
#define generic2(method, dispatch, args...)                 \
  generic((dispatch),                                       \
          Val:val_##method,                                 \
          default:obj_##method)( args )

#endif
