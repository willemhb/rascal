#ifndef rl_val_environment_h
#define rl_val_environment_h

#include "object.h"
#include "symbol.h"

/* All of the types relevant to variable definition and resolution are defined here with their APIs. */

typedef enum {
  LOCAL_SCOPE,
  UPVALUE_SCOPE,
  MODULE_SCOPE
} Scope;



/* The Binding type stores the metadata and the value or offset for a name (depending on the scope).

   The way bindings are imported depends on the type.

   If the binding is a function, its method table is merged into the function with the same name in the current module
   (if no such function exists, a new binding is created for it and namespaced to the current module).

   If the binding is a variable or constant, it is shared with the current module using the given qualifier (if any). */

struct Binding {
  HEADER;
  Symbol* name;         // the original name under which this binding was registered.
  Type*   type;         // the type that the value ought to conform to.
  Value   value;        // the value associated with this binding.
  Scope   scope;        // scope type of this binding.
  uint_t  offset;       // order in which this binding was defined.
  bool    macro;        // if this is a functional binding at module scope, indicates whether the binding is a macro.
  bool    initialized;  // if this is a binding at module scope, indicates whether it has been runtime initialized.
  bool    mutable;      // indicates whether this binding may be altered once initialized (typically false, must explicitly be declared mutable).
  bool    exported;     // indicates whether this binding is externally visible (false by default, only true if the binding name appears in the export list).
  bool    local_upval;  // if this is a binding with upvalue scope, indicates whether it's a local upvalue.
};

/* The Dependency type stores all of the information from a single import spec. */
struct Dependency {
  HEADER;
  Module* module;
  Symbol* qualifier;
  MutSet* only;
  MutSet* except;
};

/* The NameSpace type contains all of the information required to resolve a binding in a given scope. */
struct Environment {
  HEADER;
  Environment* parent;
  Module*      module;
  MutDict*     locals;
  MutDict*     upvals;
};

/* An UpValue stores an indirected reference to a stack value captured by a closure.

   If the function in which the value was defined has not returned, it is stored at upvalue->location.

   Otherwise, it is stored at upvalue->value.

   The next field is used to store an invasive linked list of open UpValues (those whose bindings haven't yet escaped the stack), ensuring that UpValues are shared correctly. */

struct UpValue {
  HEADER;
  UpValue* next;
  Value*   location; // NULL if this UpValue has been captured.
  Value    value;
};

/* The Module type represents the result of compiling a module form.

   Typically, a Module is compiled and executed just once and registered globally.

   The main use case of Modules is organizing code into libraries. */

struct Module {
  HEADER;

  Symbol*  name;        // the name under which the module is registered (first argument to the module form).
  String*  path;        // full path to the file where the module was defined.
  MutDict* depends;     // dependencies defined in an import form.
  MutDict* bindings;    // module-level bindings belonging to this module.
  List*    form;        // a copy of the module raw (uncompiled) module as read from the file.
  Chunk*   body;        // the result of compiling the module body (NULL if the module has not been initialized).
  Value    value;       // the result of executing the module body.
};

/* Globals. */
/* Type objects. */
extern Type ModuleType, NameSpaceType, BindingType, DependencyType, UpValueType;

/* Module caches. */
extern MutDict SymbolicModuleCache, PathedModuleCache;

/* External APIs */

#endif
