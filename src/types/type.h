#ifndef types_type_h
#define types_type_h

#include "types/value.h"

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
  FIXNUM_TYPE,
  GLYPH_TYPE,
  SYM_TYPE,
  STREAM_TYPE,
  FUNC_TYPE,
  BIN_TYPE,
  LIST_TYPE,
  TABLE_TYPE
};

enum Kind {
  BOTTOM_KIND,
  UNIT_KIND,
  DATA_KIND,
  TOP_KIND
};

// type information type ------------------------------------------------------
struct Mtable {
  Sym*   name;
  Type   type;
  Kind   kind;
  usize  type_hash;
  usize  size;

  // lifetime and runtime methods
  void  (*trace)(void* self);
  usize (*destruct)(void* self);
  int   (*write)(Val data, Type type, int size, void* buf);

  // interface methods
  void  (*print)(Val x, void* state);
  uhash (*hash)(Val x, void* state);
  bool  (*equal)(Val x, Val y, void* state);
  int   (*compare)(Val x, Val y, void* state);
};

/* globals */
#define NUM_TYPES (TABLE_TYPE+1)

extern Mtable MetaTables[NUM_TYPES];

/* API */
Type val_type_of(Val x);
Type obj_type_of(Obj* o);

bool val_has_type(Val x, Type t);
bool obj_has_type(Obj* o, Type t);

Mtable* val_mtable(Val x);
Mtable* obj_mtable(Obj* o);

void*   construct(Type type, usize n, usize extra);

#define type_of(x)    generic((x), Val:val_type_of, Obj*:obj_has_type)(x)
#define has_type(x,t) generic((x), Val:val_has_type, Obj*:obj_has_type)(x, t)
#define mtable(x)     generic((x), Val:val_mtable,  Obj*:obj_mtable)(x)

// initialization -------------------------------------------------------------
void type_init(void);

#endif
