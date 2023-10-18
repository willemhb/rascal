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

typedef enum {
  VARIADIC =0x001,    // method allows multiple 
  EXACT    =0x002,    // 
} MethodFl;

struct Function {
  Obj          obj;
  Symbol*      name;     // the name of this function (or `fun` if anonymous).
  MethodTable* metht;  // method table
  Method*      leaf;   // singleton 
};

struct MethodTable {
  Obj         obj;
  Table*      cache;      // cache of exact signatures
  MethodMap*  fa_methods; // fixed arity methods
  MethodMap*  va_methods; // variadic methods
};

struct MethodMap {
  Obj         obj;
  MethodNode* root;
  size_t      max_a;
};

struct MethodNode {
  Obj          obj;
  size_t       offset;   // offset of the argument corresponding to this table entry
  Method*      leaf;     // candidate method if offset == arity
  Table*       dtmap;    // 
  Table*       atmap;    // 
  Objects*     utmap;    // methods with a union annotation at `sig[offset]` (ordered by specificity)
  MethodNode*  any;      // methods with no annotation at `sig[offset]`
};

struct Method {
  Obj     obj;
  Tuple*  sig;    // declared method signature
  Tuple*  sig_s;  // sorted method signature (for comparing specificity)
  Obj*    fn;     // function to call if method matches
};

struct Native {
  Obj      obj;
  NativeFn fn;
};

struct Closure {
  Obj      obj;
  Chunk*   code;
  Objects* upvals;
};

// globals
extern struct Type FunctionType, MethodTableType,
  MethodMapType, MethodNodeType, MethodType,
  ClosureType, NativeType;

// external API
// accessors
size_t get_sig_max(Function* f);

// constructors
Function* new_func(Symbol* name, flags_t fl);

// internal types
// function and dispatch types
MethodTable* new_metht(void);
MethodMap*   new_methm(bool va);
MethodNode*  new_methn(size_t offset, int fl);
Method*      new_method(Obj* fn, Tuple* sig, bool va);

Tuple*  get_sig(size_t sig_max, size_t n, Value* vals);
Tuple*  get_macro_sig(size_t sig_max, List* args);
Method* get_method(Function* g, Tuple* s);
Method* add_method(Function* g, Tuple* s, Obj* m, bool va);

// initialization
void init_builtin_functions(void);

#endif
