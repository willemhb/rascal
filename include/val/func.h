#ifndef rl_val_func_h
#define rl_val_func_h

#include "runtime.h"

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */
struct Func {
  HEADER;

  union {
    char* _name; // makes initialization easier
    Sym*  name;
  };

  // arity limits
  size32 min_ac, max_ac;

  MTRoot* fmethods;
  MTRoot* vmethods;
};

struct Chunk {
  HEADER;

  size16 nargs, nvars; // number of positional arguments and local variables respectively
  bool   va;           // whether the function accepts variadic arguments
  Sym*   name;         // copy of the name under which the chunk was compiled
  MBin*  code;         // instruction sequence
  MVec*  vals;         // compile time constants
  Env*   vars;         // local environment
  Ns*    ns;           // toplevel namespace context
  MVec*  upvs;         // upvalues (NULL if the given chunk is a prototype)
};

struct Prim {
  HEADER;

  size32 nargs, nvars;
  bool   va;

  union {
    char* _name;
    Sym*  name;
  };

  Opcode  label;     // if the call can be resolved at compile time
  CPrimFn cfn;
};

struct MTRoot {
  HEADER;

  MTLeaf* thunk;
  MMap*   cache;
  MTNode* methods;
};

struct MTNode {
  HEADER;

  size32  offset;   // offset of the argument to match
  MTLeaf* leaf;     // method to call if out of arguments
  MMap*   children; // next level
  MTNode* any;      // fallback node (in case no children are a match)
};

struct MTLeaf {
  HEADER;

  VType ftype;

  union {
    Chunk* ufn;
    Prim*  pfn;
  };
};

/* APIs */
#define as_func(x) ((Func*)as_obj(x))
#define is_func(x) has_vtype(x, T_FUNC)

// toplevel function dispatch
MTLeaf* get_method(Func* gf, Val* vs, size32 n);
MTLeaf* add_method(Func* gf, void* m, bool va, size32 n, ...);

#endif
