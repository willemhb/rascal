#ifndef object_h
#define object_h

#include "base/value.h"

/* C types */
// flags ----------------------------------------------------------------------
typedef enum ObjFl   ObjFl;

enum ObjFl {
  FROZEN =0b10000000000,
  HASHED =0b01000000000,
  STATIC =0b00100000000,
  BLACK  =0b00010000000,
  GRAY   =0b00001000000
};

struct Obj {
  Obj *next;
  uint64 hash;
  union {
    struct {
      uint64            : 48;
      uint64 flags      : 11;
      uint64 type       :  5;
    };

    struct {
      uint64 arity : 48;
    };

    struct {
      uint64 count : 24;
      uint64 cap   : 24;
    };

    struct {
      uint64 nargs   : 24;
      uint64 nlocals : 24;
    };
  };

  ubyte space[0];
};

/* API */
bool    is_obj(Val x);
Obj*    as_obj(Val x);
Val     obj_tag(Obj* o);
flags32 obj_flags(Obj* o);

#endif
