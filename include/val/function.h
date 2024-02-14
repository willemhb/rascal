#ifndef rl_val_function_h
#define rl_val_function_h

#include "val/object.h"

/* types for implementing rascal's multimethods. */

/* C types */
typedef Value (*NativeFn)(size_t n, Value* a);
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
  MethTNode* fixed;
  MethTNode* variadic;
  MethTLeaf* thunk;
};

struct MethTNode {
  HEADER;
  size_t     offset;
  MethTLeaf* leaf;
  MutDict*   exact_methods;
  MethTNode* any_method;
};

struct MethTLeaf {
  HEADER;
  size_t arity;
  bool   variadic;
  List*  signature;
  Obj*   method;
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
  MethTableType, MethTNodeType, MethTLeafType;

/* External API */
Func* new_func(Symbol* name);

#endif
