#ifndef rl_envt_h
#define rl_envt_h

#include "val/object.h"

/* functions, types, and API for dealing with names, scopes, and environments. */

/* C types */
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
  Symbol*  name;
  Value    value;
};

struct UpValue {
  HEADER;
  UpValue* next;
  Value*   location;
  Value    value;
};

/* Globals */
extern struct Type SymbolType, EnvtType, BindingType;

/* External API */
Envt*    new_envt(Envt* parent);

#endif
