#ifndef rl_environ_h
#define rl_environ_h

#include "object.h"

/* Types and APIs, and globals for the rascal namespace and environment system. */

/* C types */
typedef enum Scope {
  LOCAL_SCOPE,     // value is on stack
  UPVALUE_SCOPE,   // value is indirected through an upvalue
  NAMESPACE_SCOPE, // value is stored in the current namespace environ
  STRUCT_SCOPE,    // value is stored in struct object
  RECORD_SCOPE,    // value is stored in record object
} Scope;

// user identifier types
struct Symbol {
  HEADER;

  // bit fields
  word_t literal : 1;

  // identifier info
  String* nmspc;
  String* name;
  word_t  idno;      // non-zero for gensyms
};

struct Environ {
  HEADER;

  // bit fields
  word_t scope    : 3;
  word_t bound    : 1;
  word_t captured : 1;

  // data fields
  Symbol*    name;      // Name for this Environ object (may be a namespace, function, or type)
  Environ*   parent;    // the environment within which this environment was defined
  Environ*   template;  // the unbound environment a bound environment was cloned from
  EnvMap*    locals;
  EnvMap*    nonlocals;

  union {
    Alist* upvals;
    MutVec*  values;
  };
};

struct Binding {
  HEADER;

  // bit fields
  word_t scope         : 3;
  word_t final         : 1;
  word_t inited        : 1;
  word_t specializable : 1; // can have methods added
  word_t macro         : 1; // macro name

  // data fields
  Binding* captures;   // the binding captured by this binding (if any)
  Environ* environ;    // the environment in which the binding was *originally* created
  Symbol*  name;       // name under which this binding was created in *original* environment
  size_t   offset;     // location (may be on stack, in upvalues, or directly in environment)
  Type*    constraint; // type constraint for this binding
  Value    initval;    // default initval (only used for object scopes)
};

struct UpValue {
  HEADER;

  // bit fields
  word_t closed : 1;

  // data fields
  UpValue* next_upv;

  union {
    Value* location;
    Value  value;
  };
};

/* Globals */
// types
extern Type SymbolType, EnvironType, BindingType, UpValueType;

// global environment
extern Environ Globals;

/* APIs */
/* Symbol API */
static inline bool is_gs(Symbol* s) {
  return s->idno > 0;
}

#define qualify(s, ns)                          \
  generic((ns),                                 \
          char*:cstr_qualify,                   \
          String*:str_qualify,                  \
          Symbol*:sym_qualify)(s, ns)

Symbol* mk_sym(String* ns, String* n, bool gs);

// qualify methods
Symbol* cstr_qualify(Symbol* s, char* cstr);
Symbol* str_qualify(Symbol* s, String* str);
Symbol* sym_qualify(Symbol* s, Symbol* ns);


/* Upvalue API */
static inline Value* deref_upval(UpValue* upv) {
  if ( upv->closed )
    return &upv->value;

  return upv->location;
}

// utilities for working with symbols and environments
#define define(n, v, e)                         \
  generic((n),                                  \
          char*:cstr_define,                    \
          Symbol*:sym_define)(n, v, e)

// define methods
size_t cstr_define(char* n, Value i, Environ* e);
size_t sym_define(Symbol* n, Value i, Environ* e);


#endif
