#ifndef type_h
#define type_h

#include "base/value.h"

#include "util/generic.h"

/* C types */
typedef enum Type Type;
typedef enum Kind Kind;

// state objects
typedef struct Mtable Mtable;

typedef uint64 uhash;

enum Type {
  NONE_TYPE,
  ANY_TYPE,
  UNIT_TYPE,
  REAL_TYPE,
  INT_TYPE,
  BOOL_TYPE,
  GLYPH_TYPE,
  SYM_TYPE,
  STREAM_TYPE,
  FUNC_TYPE,
  TUPLE_TYPE,
  BIN_TYPE,
  LIST_TYPE,
  VEC_TYPE,
  TABLE_TYPE
};

enum Kind {
  BOTTOM,
  UNIT,
  DATA,
  UNION,
  TOP
};

// type information type ------------------------------------------------------
struct Mtable {
  char*  name;
  Type   type;
  Kind   kind;
  usize  size;

  // interpreter methods
  void  (*trace)(void* self);
  usize (*destruct)(void* self);
  void  (*print)(Val x, void* state);
  uhash (*hash)(Val x, void* state);
  bool  (*equal)(Val x, Val y, void* state);
  int   (*compare)(Val x, Val y, void* state);
};

/* globals */
#define NUM_TYPES (TABLE_TYPE+1)

extern Mtable MetaTables[NUM_TYPES];

/* API */
Type    val_type_of(Val v);
Type    obj_type_of(Obj* o);
bool    val_has_type(Val v, Type t);
bool    obj_has_type(Obj* o, Type t);

// generics
#define type_of(x)      GENERIC_2(type_of, x)
#define has_type(x, t)  GENERIC_2(has_type, x, t)
#define mtable(x)      (MetaTables+type_of(x))

#endif
