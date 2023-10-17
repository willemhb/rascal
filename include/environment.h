#ifndef rascal_environment_h
#define rascal_environment_h

#include "table.h"

/* functions, types, and API for dealing with names, scopes, and environments. */

// C types
// user types
typedef enum {
  INTERNED=0x001,    // saved in symbol table
  LITERAL =0x002,    // always treat `s` like `'s`
} SymFl;

struct Symbol {
  Obj       obj;
  char*     name;
  uintptr_t idno;    // unique identifier
  CompileFn special; // special form associated with this symbol
};

typedef enum {
  GLOBAL_SCOPE, PRIVATE_SCOPE, FUNCTION_SCOPE,
} ScopeType;

struct Environment {
  Obj          obj;
  Environment* parent;
  NameSpace*   globals;
  NameSpace*   private;
  NameSpace*   locals;
  NameSpace*   upvals;
};

typedef enum {
  CONSTANT   = 0x001,
  LOCAL_UPVAL= 0x002,
} BindingFl;

struct Binding {
  Obj          obj;
  Binding*     captured; // the binding this reference captured (if any)
  Symbol*      name;     // plain name of the binding (unqualified)
  NameSpace*   ns;       // namespace to which this binding belongs
  size_t       offset;
  Value        value;
};

// globals
extern struct Type SymbolType, EnvironmentType, BindingType;

// external API
void    initEnvt(Vm* vm);
void    freeEnvt(Vm* vm);

#define getAnnot(x, k)    generic2(getAnnot, x, x, k)
#define setAnnot(x, k, v) generic2(setAnnot, x, k, v)

Value  getAnotVal(Value x, Value key);
Value  getAnnotObj(void* p, Value key);
Value  setAnnotVal(Value x, Value key, Value value);
Value  setAnnotObj(void* p, Value key, Value value);

// accessors
ScopeType getScopeType(Environment* envt);

// constructors
Symbol*      newSymbol(char* name, flags_t fl);
Environment* newEnvironment(Environment* parent, ScopeType type);
Binding*     newBinding(Binding* parent, Symbol* name, NameSpace* ns, size_t offset, NsType type, Value val);

// convenience constructors
Symbol* symbol(char* token);
Symbol* gensym(char* name);

// utilities
Binding*  define(Environment* envt, void* p, Value init, bool internal);
Binding*  capture(Environment* envt, Symbol* name);
Binding*  lookup(Environment* envt, Symbol* name);

#endif
