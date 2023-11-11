#ifndef rl_val_func_h
#define rl_val_func_h

#include "val/object.h"

/* types for implementing rascal's multimethods. */

/* C types */
typedef Value (*NativeFn)(size_t n, Value* a);
typedef struct MethTRoot MethTRoot;
typedef struct MethTNode MethTNode;
typedef struct MethTLeaf MethTLeaf;

struct Func {
  HEADER;

  Symbol* name;
  Obj*    func;
};

struct MethTable {
  HEADER;

  MutDict*   cache;
  MethTRoot* fixed;
  MethTRoot* variadic;
};

struct MethTNode {
  HEADER;
  size_t     offset;
  MethTLeaf* leaf;
  MutDict*   exact_methods;
  MutDict*   abstract_methods;
  Alist*     union_methods;
  MethTNode* any_method;
};

struct Native {
  HEADER;
  NativeFn func;
};

struct Closure {
  HEADER;
  Chunk*   code;
  Objects* upvals;
};

/* Globals */
extern Type FuncType, NativeType, ClosureType,
  MethTableType, MethTLeafType, MethTRootType,
  MethTNodeType;

/* External API */
Func new_func(Symbol* name);

#endif
