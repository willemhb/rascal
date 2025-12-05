/* Core type information, globals, and toplevel expression APIs. */
// headers --------------------------------------------------------------------
#include <string.h>

#include "data/base.h"
#include "data/sym.h"

#include "sys/error.h"
#include "sys/memory.h"

#include "util/hashing.h"
#include "util/number.h"

// macros ---------------------------------------------------------------------

// alist implementation macro


// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
TypeInfo Types[NUM_TYPES];

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
// Expression APIs ------------------------------------------------------------
ExpType get_exp_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NUL_T   : t = EXP_NUL;       break;
    case BOOL_T  : t = EXP_BOOL;      break;
    case GLYPH_T : t = EXP_GLYPH;     break;
    case OBJ_T   : t = head(x)->type; break;
    default      : t = EXP_NUM;       break;
  }

  return t;
}

ExpType get_obj_type(void* p) {
  assert(p != NULL);

  return ((Obj*)p)->type;
}

ExpAPI* exp_api(Expr x) {
  TypeInfo* ti = type_info(x);

  return ti->exp_api;
}

ObjAPI* obj_api(void* ptr) {
  assert(ptr != NULL);

  Obj* obj = ptr;

  return Types[obj->type].obj_api;
}

void mark_exp(Expr x) {
  if ( exp_tag(x) == OBJ_T )
    mark_obj(as_obj(x));
}

// Object APIs ----------------------------------------------------------------
void trace_exprs(Exprs* xs) {
  trace_exp_array(xs->count, xs->vals);
}

void trace_objs(Objs* os) {
  trace_obj_array(os->count, os->vals);
}

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ_T;
}

void* mk_obj(ExpType type, flags_t flags, size_t n) {
  assert(type > EXP_GLYPH);

  ObjAPI* api = Types[type].obj_api;
  
  Obj* out;

  if ( api->alloc_fn )
    out = api->alloc_fn(type, flags, n);

  else
    out = allocate(true, api->obsize);

  out->type     = type;
  out->bfields  = flags | FL_GRAY;
  out->heap     = Heap;
  Heap          = out;

  return out;
}

size_t obj_size(void* ptr) {
  assert(ptr != NULL);

  Obj* obj    = ptr;
  ObjAPI* api = obj_api(obj);
  size_t size = api->size_fn ? api->size_fn(obj) : api->obsize;

  return size;
}

void* clone_obj(void* ptr) {
  assert(ptr != NULL);

  Obj* obj    = ptr;
  ObjAPI* api = obj_api(obj);
  Obj* out    = duplicate(true, obj_size(obj), obj);

  if ( api->clone_fn )
    api->clone_fn(out);

  return out;
}

void mark_obj(void* ptr) {
  Obj* obj = ptr;

  if ( obj != NULL ) {
    if ( obj->black == false ) {
      obj->black = true;

      ObjAPI* api = obj_api(obj);

      if ( api->trace_fn )
        gc_save(obj);

      else
        obj->gray = false;
    }
  }
}

// used mostly to manually unmark global objects so they're collected correctly
// on subsequent GC cycles
void unmark_obj(void* ptr) {
  Obj* obj   = ptr;
  obj->black = false;
  obj->gray  = true;
}

void free_obj(void* ptr) {
  Obj* obj = ptr;

  if ( obj ) {
    ObjAPI* api = obj_api(obj);

    if ( api->free_fn )
      api->free_fn(obj);

    release(obj, obj_size(obj));
  }
}

// HAMT helpers ---------------------------------------------------------------
size_t hamt_shift(void* ob) {
  return ((Obj*)ob)->flags & HAMT_MASK;
}

size_t hamt_asize(void* ob) {
  return ((Obj*)ob)->flags >> HAMT_SHIFT;
}

void init_hamt(void* ob, size_t shift, size_t asize) {
  Obj* obj    = ob;
  obj->flags |= shift | (asize << HAMT_SHIFT);
}

// initialization -------------------------------------------------------------
