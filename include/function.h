#ifndef rascal_native_h
#define rascal_native_h

#include "object.h"

/* functions and methods */
// user types
typedef enum {
  FINAL    =0x001,    // don't allow specialization
  MACRO    =0x002,    // syntactic extension
  SINGLETON=0x004,
} FnFl;

struct Function {
  Obj          obj;
  Symbol*      name;       // the name of this function (or `fun` if anonymous).

  union {
    MethodTable* methods;
    Method*      singleton;
  };
};

struct MethodTable {
  Obj         obj;
  MethodCache cache;
  MethodMap*  faMethods;
  MethodMap*  vaMethods;
};

struct MethodMap {
  Obj         obj;
  MethodNode* root;
  size_t      maxA;
  bool        va;
};

struct MethodNode {
  Obj          obj;
  size_t       offset;   // offset of the argument corresponding to this table entry
  Method*      leaf;     // candidate method if offset == arity
  TypeMap      dtmap;    // methods with a datatype annotation at `sig[offset]`
  TypeMap      atmap;    // methods with an abstract type annotation at `sig[offset]`
  Objects      utmap;    // methods with a union annotation at `sig[offset]` (ordered by specificity)
  MethodNode*  any;      // methods with no annotation at `sig[offset]`
  bool         va;       // leaf is a variadic function
  bool         exact;    // leaf has an exact signature
};

struct Method {
  Obj     obj;
  Tuple*  sig;    // declared method signature
  Tuple*  sig_s;  // sorted method signature (for comparing specificity)
  Obj*    fn;     // function to call if method matches
  bool    va;     // signature for a variadic method
  bool    exact;  // all annotations refer to concrete datatypes
};

struct Native {
  Obj      obj;
  NativeFn callBack;
};

struct Closure {
  Obj          obj;
  Chunk*       code;
  Environment* envt;
};

// external API
// constructors
// user types
Function* newFunction(Symbol* name, Obj* ini, int flags);

// internal types
// function and dispatch types
MethodTable* newMethodTable(void);
MethodMap*   newMethodMap(bool va);
MethodNode*  newMethodNode(size_t offset, bool va, bool exact);
Method*      newMethod(Obj* fn, Tuple* sig, bool va);

Method*      getMethod(Function* g, Tuple* s);
void         addMethod(Function* g, Tuple* s, Obj* m, bool va);

// initialization
void         initializeNativeFunctions(void);

#endif
