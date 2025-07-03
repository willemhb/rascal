/* DESCRIPTION */
// headers --------------------------------------------------------------------
#include "data/obj.h"
#include "data/array.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
void trace_exprs(Exprs* xs) {
  for ( int i=0; i < xs->count; i++ )
    mark_exp(xs->vals[i]);
}

void trace_objs(Objs* os) {
  for ( int i=0; i < os->count; i++ )
    mark_obj(os->vals[i]);
}

// expression APIs

// object API
void* as_obj(Expr x) {
  return (void*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ_T;
}

void* mk_obj(ExpType type, flags_t flags) {
  Obj* out = allocate(true, Types[type].obsize);

  out->type     = type;
  out->bfields  = flags | FL_GRAY;
  out->heap     = Heap;
  Heap          = out;

  return out;
}

void* clone_obj(void* ptr) {
  assert(ptr != NULL);

  Obj* obj = ptr;
  ExpTypeInfo* info = &Types[obj->type];
  Obj* out = duplicate(true, info->obsize, obj);

  if ( info->clone_fn )
    info->clone_fn(out);

  return out;
}

void mark_obj(void* ptr) {
  Obj* obj = ptr;

  if ( obj != NULL ) {
    if ( obj->black == false ) {
      obj->black = true;

      ExpTypeInfo* info = &Types[obj->type];

      if ( info->trace_fn )
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
    ExpTypeInfo* info = &Types[obj->type];

    if ( info->free_fn )
      info->free_fn(obj);

    release(obj, info->obsize);
  }
}

// initialization -------------------------------------------------------------
