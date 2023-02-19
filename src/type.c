#include "types/type.h"
#include "types/object.h"
#include "types/text.h"
#include "types/table.h"

#include "runtime/memory.h"

#include "util/hash.h"

// globals --------------------------------------------------------------------
Mtable MetaTables[NUM_TYPES];

// API ------------------------------------------------------------------------
// type queries & predicates --------------------------------------------------
Type val_type_of(Val v) {
  switch (TAG_BITS(v)) {
    case SIGNAL_TAG:
    case CPTR_TAG:
    case CSTR_TAG:
    case FIXNUM_TAG: return FIXNUM_TYPE;
    case UNIT_TAG:   return UNIT_TYPE;
    case GLYPH_TAG:  return GLYPH_TYPE;
    case OBJ_TAG:    return obj_type_of(as_obj(v));
    default:         return REAL_TYPE;
  }
}

Type obj_type_of(Obj* o) {
  return o->type;
}

bool val_has_type(Val v, Type t) {
  switch (MetaTables[t].kind) {
    case BOTTOM_KIND: return false;
    case UNIT_KIND:   return v == NUL;
    case DATA_KIND:   return type_of(v) == t;
    case TOP_KIND:    return true;
  }

  unreachable();
}

bool obj_has_type(Obj* o, Type t) {
  switch(MetaTables[t].kind) {
    case BOTTOM_KIND: return false;
    case UNIT_KIND:   return false;
    case DATA_KIND:   return o->type == t;
    case TOP_KIND:    return true;
  }

  unreachable();
}

Mtable* val_mtable(Val x) {
  return MetaTables + type_of(x);
}

Mtable* obj_mtable(Obj* o) {
  return MetaTables + o->type;
}

// misc utilities -------------------------------------------------------------
void* construct(Type type, usize n, usize extra) {
  /* these are needed statically for initialization */ 
  static const usize ObjSizes[NUM_TYPES] = {
    [SYM_TYPE]  = sizeof(Sym),  [STREAM_TYPE] = sizeof(Stream),
    [FUNC_TYPE] = sizeof(Func), [BIN_TYPE]    = sizeof(Bin),
    [LIST_TYPE] = sizeof(List), [TABLE_TYPE]  = sizeof(Table)
  };
  
  usize total = ObjSizes[type] * n + extra;

  return allocate(total, 0);
}

// initialization -------------------------------------------------------------
void number_init(void) {
  // initialize top, bottom, and unit types -----------------------------------
  extern Val native_none(Val* pos, List* opt, Table* kw, List* va);

  MetaTables[NONE_TYPE] = (Mtable) {
    .name       =intern("none"),
    .constructor=native(0, 1, "none", &MetaTables[NONE_TYPE], native_none),
    .type       =NONE_TYPE,
    .kind       =BOTTOM_KIND,
    .type_hash  =hash_uint(NONE_TYPE)
  };

  extern Val  native_unit(Val* pos, List* opt, Table* kw, List* va);
  extern void print_unit(Val x, void* state);

  MetaTables[UNIT_TYPE] = (Mtable) {
    .name       =intern("unit"),
    .constructor=native(0, 1, "unit", &MetaTables[UNIT_TYPE], native_unit),
    .type       =UNIT_TYPE,
    .kind       =UNIT_KIND,
    .type_hash  =hash_uint(UNIT_TYPE),
    .size       =sizeof(NUL),
    .print      =print_unit
  };

  extern Val native_any(Val* pos, List* opt, Table* kw, List* va);
  
  MetaTables[ANY_TYPE] = (Mtable) {
    .name       =intern("any"),
    .constructor=native(0, 1, "any", &MetaTables[ANY_TYPE], native_any),
    .type       =ANY_TYPE,
    .kind       =TOP_KIND,
    .type_hash  =hash_uint(ANY_TYPE)
  };
}
