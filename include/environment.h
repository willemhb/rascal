#ifndef rascal_environment_h
#define rascal_environment_h

#include "object.h"

/* functions, types, and API for dealing with names, scopes, and environments. */

// C types
// generics
#include "tpl/declare.h"

TABLE_OBJ_TYPE(SymbolTable, char*, Symbol*);


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
  TOPLEVEL_ENVIRONMENT, SCRIPT_ENVIRONMENT, WITH_ENVIRONMENT, FUNCTION_ENVIRONMENT,
} EnvironmentFl;

struct Environment {
  Obj    obj;
  Table* globals;
  Table* private;
  Table* locals;
  Table* upvals;
};

struct Binding {
  Obj          obj;
  Symbol*      name;    // plain name of the binding (unqualified)
  Environment* envt;    // 
  size_t       offset;
  Value        value;
};

struct UpValue {
  Obj      obj;
  UpValue* next;
  size_t   offset;
  Value    value;
};

// globals
extern struct Type SymbolType, EnvironmentType, BindingType, UpValueType;

// external API
void    initEnvt(Vm* vm);
void    freeEnvt(Vm* vm);

#define getAnnot(x, k)    generic2(getAnnot, x, x, k)
#define setAnnot(x, k, v) generic2(setAnnot, x, k, v)

Value  getAnotVal(Value x, Value key);
Value  getAnnotObj(void* p, Value key);
Value  setAnnotVal(Value x, Value key, Value value);
Value  setAnnotObj(void* p, Value key, Value value);

// constructors
Symbol*      newSymbol(char* name, int fl);
Environment* newEnvironment(int flags);
Binding*     newBinding(Symbol* name, Environment* environment, size_t offset, int fl);
UpValue*     newUpValue(size_t offset);

// convenience constructors
Symbol* symbol(char* token);
Symbol* gensym(char* name);

// utilities

// initialization
void initializeEnvironment(void);

#endif
