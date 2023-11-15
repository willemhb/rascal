#ifndef rl_envt_h
#define rl_envt_h

#include "val/object.h"

/* functions, types, and API for dealing with names, scopes, and environments. */

/* C types */
typedef enum {
  GLOBAL, PRIVATE, LOCAL, UPVALUE,
} BindType;

struct Envt {
  HEADER;
  Envt*    parent;
  MutDict* globals;
  MutDict* private;
  MutDict* locals;
  MutDict* upvals;
};

struct Binding {
  HEADER;
  Binding* captured; // the binding this reference captured (if any)
  Symbol* name; // plain name for the binding
  Value value; // value for the binding (if value is stored directly)
  long offset;
};

struct UpValue {
  HEADER;
  UpValue* next;
  Value*   location;
  Value    value;
};

/* Globals */
extern struct Type EnvtType, BindingType, UpValType;

/* External API */
/* Envt API */
Envt* new_envt(Envt* parent);

/* binding API */

/* UpValue API */
UpValue* mk_upval(Value* location, UpValue* next);

#endif
