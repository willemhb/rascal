#ifndef type_h
#define type_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef uint64 uhash;
typedef uint64 uidno;

typedef enum {
  BOTTOM,
  TOP,
  UNARY,
  DATA,
  UNION
} Kind;

typedef enum {
  // fucked up types ----------------------------------------------------------
  NONE=1,
  ANY,
  UNIT,

  // data types ---------------------------------------------------------------
  REAL,
  FIXNUM,
  GLYPH,
  SYM,
  STREAM,
  FUNC,
  BIN,
  STR,
  LIST,
  VEC,
  TUPLE,
  DICT,
  SET,
  RECORD,

  // strictly internal ---------------------------------------------------------
  SYSPTR,   // raw C pointer ---------------------------------------------------
  CHUNK,    // compiled code ---------------------------------------------------
  UPVAL,    // closed-over local -----------------------------------------------
  DISPATCH, // method table ----------------------------------------------------
  METHOD,   // single method ---------------------------------------------------
  CNTL      // reified continuation --------------------------------------------
} Type;

// type information type ------------------------------------------------------
typedef struct {
  // metaobject information ---------------------------------------------------
  char*  name;
  Type   type;
  Kind   kind;
  
  uhash  hash;
  uidno  idno;

  // size and layout information ----------------------------------------------
  usize  size;

  // lifetime and runtime methods ---------------------------------------------
  struct {
    void  (*trace)(void* self);
    usize (*destruct)(void* self);

    // interface methods --------------------------------------------------------
    void  (*print)(Val x, void* state);
    uhash (*hash)(Val x, void* state);
    bool  (*equal)(Val x, Val y, void* state);
    int   (*compare)(Val x, Val y, void* state);
  } m;
} Mtable;

// globals --------------------------------------------------------------------
#define NUM_TYPES (CNTL+1)

extern Mtable MetaTables[NUM_TYPES];

#define MTABLE(type) (MetaTables[(type)])

// API -------------------------------------------------------------------------
Type    val_type_of(Val x);
Type    obj_type_of(Obj* o);

bool    val_has_type(Val x, Type t);
bool    obj_has_type(Obj* o, Type t);

Mtable* val_mtable(Val x);
Mtable* obj_mtable(Obj* o);

void*   construct(Type type, usize n, usize a, usize obsize, uintptr_t ini, bool isdata);

#define type_of(x)    generic((x), Val:val_type_of, Obj*:obj_has_type)(x)
#define has_type(x,t) generic((x), Val:val_has_type, Obj*:obj_has_type)(x, t)
#define mtable(x)     generic((x), Val:val_mtable,  Obj*:obj_mtable, default:obj_mtable)(x)

#endif
