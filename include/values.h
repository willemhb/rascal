#ifndef rl_values_h
#define rl_values_h

#include "common.h"

/* Initial declarations for Rascal value types and basic APIs */
// general types
typedef word_t      Val;    // tagged word. May be immediate or an object
typedef struct Obj  Obj;    // Compound or large values are stored in a boxed object
typedef struct Box  Box;    // type to represent boxed immediates

// immediate types
typedef nullptr_t Nul;
typedef bool      Bool;
typedef char32_t  Glyph;
typedef double    Num;
typedef sint32    Small;
typedef void*     Ptr;

// callable types
typedef struct Func   Func;   // generic function object
typedef struct Chunk  Chunk;  // compiled user code
typedef struct Prim   Prim;   // container for primitive C function
typedef struct MTRoot MTRoot; // method table root element
typedef struct MTNode MTNode; // method table internal element
typedef struct MTLeaf MTLeaf; // method table leaf element (contains actual thing to be called)

// miscellaneous user types
typedef struct Sym  Sym;
typedef struct Type Type;
typedef struct Port Port;

// persistent collection types
typedef struct Str  Str;
typedef struct Bin  Bin;
typedef struct List List;
typedef struct Vec  Vec;
typedef struct Map  Map;

// mutable collection types
typedef struct MStr   MStr;
typedef struct MBin   MBin;
typedef struct MVec   MVec;
typedef struct MMap   MMap;

// internal node types
typedef struct VNode   VNode;
typedef struct VLeaf   VLeaf;
typedef struct MNode   MNode;
typedef struct MLeaf   MLeaf;
typedef struct MLeafs  MLeafs;

// miscellaneous internal types
typedef struct Rt   Rt;
typedef struct Cntl Cntl;

// internal environment types
typedef struct Ns  Ns;
typedef struct Env Env;
typedef struct Ref Ref;
typedef struct Upv Upv;

// lots generics with this particular pattern
#define generic2(method, dispatch, args...)                 \
  generic((dispatch),                                       \
          Val:val_##method,                                 \
          default:obj_##method)( args )

#endif
