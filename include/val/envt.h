#ifndef rl_envt_h
#define rl_envt_h

#include "val/object.h"

/* functions, types, and API for dealing with names, scopes, and environments. */

/* C types */
struct Envt {
  HEADER;
  Envt  parent;
  Table globals;
  Table private;
  Table locals;
  Table upvals;
};

struct Binding {
  HEADER;
  Binding captured; // the binding this reference captured (if any)
  Symbol  name;
  Value   value;
};

struct UpValue {
  HEADER;
  UpValue next;
  Value*  location;
  Value   value;
};

// globals
extern struct Type SymbolType, EnvtType, BindingType;


#endif
