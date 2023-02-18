#ifndef object_h
#define object_h

#include "base/value.h"
#include "base/type.h"

/* C types */
// flags ----------------------------------------------------------------------
typedef enum   ObjFl ObjFl;

enum ObjFl {
  FROZEN     =0b10000000000,
  HASHED     =0b01000000000,
  NOFREE     =0b00100000000,
  BLACK      =0b00010000000,
  GRAY       =0b00001000000
};

struct Obj {
  Obj *next;
  uint64 hash   : 48;
  uint64 flags  : 11;
  uint64 type   :  5;

  ubyte space[0];
};

/* globals */

/* API */
bool is_obj(Val x);
Obj* as_obj(Val x);
Val  mk_obj(Obj* o);

// flag getters/setters -------------------------------------------------------
bool has_flag(Obj* o, int fl);
bool set_flag(Obj* o, int fl);
bool clear_flag(Obj* o, int fl);
bool has_wflag(Obj* o, int fl, int m);
int  get_flags(Obj* o, int m);
int  clear_flags(Obj* o, int m);
int  set_flags(Obj* o, int fl, int m);

// common predicates ----------------------------------------------------------
bool is_frozen(Obj* o);

// lifetime & memory management dispatch --------------------------------------
void  init_obj(Obj* self, Type type, int fl);
void  mark_obj(Obj* self);
void  destruct_obj(Obj* self);

#endif
