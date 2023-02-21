#ifndef type_h
#define type_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef uint64 uhash;

typedef enum {
  NONE=1,
  ANY,
  UNIT,
  REAL,
  GLYPH,
  SYM,
  FUNC,
  BIN,
  LIST,
  TABLE
} Type;

// type information type ------------------------------------------------------
typedef struct {
  // metaobject information ---------------------------------------------------
  char*  name;
  Type   type;
  uhash  type_hash;

  // size and layout information ----------------------------------------------
  usize  size;

  // lifetime and runtime methods ---------------------------------------------
  void  (*trace)(void* self);
  usize (*destruct)(void* self);

  // interface methods --------------------------------------------------------
  void  (*print)(Val x, void* state);
  uhash (*hash)(Val x, void* state);
  bool  (*equal)(Val x, Val y, void* state);
  int   (*compare)(Val x, Val y, void* state);
} Mtable;

// globals --------------------------------------------------------------------
#define NUM_TYPES (TABLE+1)

extern Mtable MetaTables[NUM_TYPES];

// API -------------------------------------------------------------------------
Type val_type_of(Val x);
Type obj_type_of(Obj* o);

bool val_has_type(Val x, Type t);
bool obj_has_type(Obj* o, Type t);

Mtable* val_mtable(Val x);
Mtable* obj_mtable(Obj* o);

void*   construct(Type type, usize n, usize extra, void* data);

#define type_of(x)    generic((x), Val:val_type_of, Obj*:obj_has_type)(x)
#define has_type(x,t) generic((x), Val:val_has_type, Obj*:obj_has_type)(x, t)
#define mtable(x)     generic((x), Val:val_mtable,  Obj*:obj_mtable, default:obj_mtable)(x)

#endif
