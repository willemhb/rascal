#ifndef rl_val_environment_h
#define rl_val_environment_h

#include "object.h"
#include "symbol.h"

/* All of the types relevant to variable definition and resolution are defined here with their APIs. */

/* Scope types */
enum {
  LOCAL_SCOPE,
  UPVAL_SCOPE,
  MODULE_SCOPE
};

enum {
  FUNC_BIND  = 0x01, // name of a function (immutable)
  MACRO_BIND = 0x03, // name of a macro (immutable)
  VALUE_BIND = 0x04, // name of a constant (immutable)
  VAR_BIND   = 0x08, // name of a variable (mutable)
};

/* hold's information on a single binding. */
struct Binding {
  HEADER;
  Symbol*   name;          // the original name under which this binding was registered
  Value     value;         // the value associated with this binding
  flags_t   scope_type;    // self explanatory 
  flags_t   bind_type;     // semantics of the binding
  bool      initialized;   // has this module-level binding been initialized?
  bool      local_upval;   // does this upvalue refer to a local binding in the same scope?
  int       offset;        // order in which this local or upvalue was defined
  int       parent_offset; // order in which the parent of this upvalue was defined
};

struct Module {
  HEADER;
  
  Symbol*  name;        // common name of the module
  String*  path;        // absolute path to file where module is defined
  MutDict* imports;     // map from module names to import specs
  MutDict* exports;     // map from unqualified variable names to bindings
  MutDict* bindings;    // actual module-level bindings for this module
  List*    form;        // the contents of the (begin ...) form
  Closure* body;        // the result of compiling the (begin ...) form
  Value    result;      // the result of executing body (also indicates whether module has been initialized)
};

struct Dependency {
  HEADER;
  Module* imported;  // 
  Symbol* qualifier; // qualified name (if any)
  MutSet* only;      // include in import (exclude all others)
  MutSet* except;    // exclude from import
};

/* The Environment type contains all of the information required to resolve a binding in a given scope */
struct Environment {
  HEADER;
  Environment* parent;
  Module*      module;
  MutDict*     locals;
  MutDict*     upvals;
};



/* Globals. */
/* Type objects. */
extern Type EnvironmentType, ModuleType, DependencyType, BindingType;

/* External APIs */
#define is_bind(x)        has_type(x, &BindingType)
#define as_bind(x)        as(Binding*, untag48, x)

Binding* new_bind(Symbol* name, flags_t scope_type, flags_t bind_type);

#define is_envt(x) has_type(x, &EnvironmentType)
#define as_envt(x) as(Environment*, untag48, x)

Environment* new_envt(Environment* parent);

// define/resolve API
Binding* resolve(Symbol* name, Environment* envt, bool capture);
Binding* define(Symbol* name, Environment* envt, flags_t bind_type);

#endif
