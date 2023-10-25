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
  GLOBAL_SCOPE, FUNCTION_SCOPE,
} ScopeType;

struct Environment {
  Obj          obj;
  Environment* parent;
  NameSpace*   globals;
  NameSpace*   locals;
  NameSpace*   upvals;
};

typedef enum {
  FINAL      = 0x001,
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

// common options
extern Value FinalOpt, DocOpt, NameOpt, SignatureOpt, MacroOpt, TypeOpt,
  EnvtOpt, VaOpt, FileOpt;

// common implicit variable names
extern Value FormSym, EnvtSym;

// other common names
extern Value ReplSym;

// external API
#define get_annot(x, k)    generic2(get_annot, x, x, k)
#define set_annot(x, k, v) generic2(set_annot, x, x, k, v)
#define get_annots(x)      generic2(get_annots, x, x)

// annotations
void   merge_annot(void* dst, void* src);
void   copy_annot(void* dst, void* src, Value key, Value fallback);
Map*   get_annots_val(Value x);
Map*   get_annots_obj(void* p);
Value  get_annot_val(Value x, Value key);
Value  get_annot_obj(void* p, Value key);
Value  set_annot_val(Value x, Value key, Value value);
Value  set_annot_obj(void* p, Value key, Value value);

// accessors
ScopeType get_scope_type(Environment* envt);
bool      is_local_upval(Binding* bind);

// constructors
Symbol*      new_symbol(char* name, flags_t fl);
Environment* new_envt(Environment* parent, ScopeType type);
Binding*     new_bind(Binding* parent, Symbol* name, NameSpace* ns, size_t offset, NsType type, Value val);

// convenience constructors
Symbol* symbol(char* token);
Symbol* keyword(char* token);
Symbol* gensym(char* name);
Symbol* genkey(char* name);

// utilities
bool      is_bound(Environment* envt, Symbol* name);
bool      is_unbound(Environment* envt, Symbol* name);

Binding*  define(Environment* envt, Symbol* name, Value init, flags_t fl);
Binding*  defun(Environment* envt, Symbol* name, flags_t fl);
Binding*  defmac(Environment* envt, Symbol* name, flags_t fl);
Binding*  capture(Environment* envt, Symbol* name);
Binding*  lookup(Environment* envt, Symbol* name, bool capture);
Function* lookup_syntax(Environment* envt, Symbol* name);

// global initialization
void init_options(void);

#endif
