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
  Symbol*   name;        // the original name under which this binding was registered
  Value     value;       // the value associated with this binding
  flags_t   scope_type;  //  
  flags_t   bind_type;   // 
  bool      initialized; // 
  bool      local_upval; // 
};

typedef struct {
  Symbol*  name;
  Binding* bind;
} ScopeEntry;

struct Scope {
  HEADER;
  ScopeEntry* data;
  short*      map;
  flags_t     scope_type;
  uint_t      cnt;
  uint_t      mcap;
  uint_t      ecap;
};

/* The Environment type contains all of the information required to resolve a binding in a given scope */
struct Environment {
  HEADER;
  Environment* parent;
  Scope*       module;
  Scope*       locals;
  Scope*       upvals;
};

/* Globals. */
/* Type objects. */
extern Type EnvironmentType, BindingType, ScopeType;

/* External APIs */
#define is_bind(x)        has_type(x, &BindingType)
#define as_bind(x)        as(Binding*, untag48, x)

Binding* new_bind(Symbol* name, flags_t scope_type, flags_t bind_type);

#define is_scope(x) has_type(x, &ScopeType)
#define as_scope(x) as(Scope*, untag48, x)

Scope* new_scope(flags_t scope_type);

Binding* get_bind(Scope* scope, Symbol* name);
Binding* add_bind(Scope* scope, Symbol* name, flags_t bind_type);

#define is_envt(x) has_type(x, &EnvironmentType)
#define as_envt(x) as(Environment*, untag48, x)

Environment* new_envt(Environment* parent);

Binding* resolve(Symbol* name, Environment* envt, bool capture, flags_t* scope_type, size_t* offset);
Binding* define(Symbol* name, Environment* envt, flags_t bind_type, flags_t* scope_type, size_t* offset);

#endif
