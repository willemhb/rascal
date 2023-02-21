#include <string.h>

#include "type.h"
#include "object.h"

#include "runtime.h"

#include "util/hash.h"

// globals --------------------------------------------------------------------
// runtime methods ------------------------------------------------------------
// trace ----------------------------------------------------------------------
extern void trace_sym(void* self);
extern void trace_func(void* self);
extern void trace_list(void* self);
extern void trace_table(void* self);

// destruct -------------------------------------------------------------------
extern usize destruct_sym(void* self);
extern usize destruct_func(void* self);
extern usize destruct_bin(void* self);
extern usize destruct_table(void* self);

// print ----------------------------------------------------------------------
extern void print_unit(Val x, void* state);
extern void print_real(Val x, void* state);
extern void print_glyph(Val x, void* state);
extern void print_sym(Val x, void* state);
extern void print_func(Val x, void* state);
extern void print_bin(Val x, void* state);
extern void print_list(Val x, void* state);
extern void print_table(Val x, void* state);

// hash -----------------------------------------------------------------------
extern uhash hash_sym(Val x, void* state);
extern uhash hash_func(Val x, void* state);
extern uhash hash_bin(Val x, void* state);
extern uhash hash_list(Val x, void* state);
extern uhash hash_table(Val x, void* state);

// equal ----------------------------------------------------------------------
extern bool equal_funcs(Val x, Val y, void* state);
extern bool equal_lists(Val x, Val y, void* state);
extern bool equal_bins(Val x, Val y, void* state);
extern bool equal_tables(Val x, Val y, void* state);

// compare --------------------------------------------------------------------
extern int compare_reals(Val x, Val y, void* state);
extern int compare_glyphs(Val x, Val y, void* state);
extern int compare_syms(Val x, Val y, void* state);
extern int compare_funcs(Val x, Val y, void* state);
extern int compare_lists(Val x, Val y, void* state);
extern int compare_bins(Val x, Val y, void* state);
extern int compare_tables(Val x, Val y, void* state);

Mtable MetaTables[NUM_TYPES] = {
  [NONE] = {
    .name="none"
  },

  [ANY] = {
    .name="any"
  },

  [UNIT] = {
    .name="unit",
    .size=sizeof(Val),
    .print=print_unit
  },

  [REAL] = {
    .name="real",
    .size=sizeof(Real),
    .print=print_real,
    .compare=compare_reals
  },

  [GLYPH] = {
    .name="glyph",
    .size=sizeof(Glyph),
    .print=print_glyph,
    .compare=compare_glyphs
  },

  [SYM] = {
    .name="sym",
    .size=sizeof(Sym),
    .trace=trace_sym,
    .destruct=destruct_sym,
    .print=print_sym,
    .hash=hash_sym,
    .compare=compare_syms
  },

  [FUNC] = {
    .name="func",
    .size=sizeof(Glyph),
    .trace=trace_func,
    .destruct=destruct_func,
    .print=print_func,
    .hash=hash_func,
    .equal=equal_funcs,
    .compare=compare_funcs
  },

  [LIST] = {
    .name="list",
    .size=sizeof(List),
    .trace=trace_list,
    .print=print_list,
    .hash=hash_list,
    .equal=equal_lists,
    .compare=compare_lists
  },

  [BIN] = {
    .name="bin",
    .size=sizeof(Bin),
    .destruct=destruct_bin,
    .print=print_bin,
    .hash=hash_bin,
    .equal=equal_bins,
    .compare=compare_bins
  },

  [TABLE] = {
    .name="table",
    .size=sizeof(Table),
    .trace=trace_table,
    .destruct=destruct_table,
    .print=print_table,
    .hash=hash_table,
    .equal=equal_tables,
    .compare=compare_tables
  }
};

// API ------------------------------------------------------------------------
// type queries & predicates --------------------------------------------------
Type val_type_of(Val v) {
  switch (TAG_BITS(v)) {
    case UNIT_TAG:   return UNIT;
    case GLYPH_TAG:  return GLYPH;
    case OBJ_TAG:    return obj_type_of(as_obj(v));
    default:         return REAL;
  }
}

Type obj_type_of(Obj* o) {
  return o->type;
}

bool val_has_type(Val v, Type t) {
  switch (t) {
    case NONE: return false;
    case UNIT: return v == NUL;
    case ANY:  return true;
    default:   return type_of(v) == t;
  }
}

bool obj_has_type(Obj* o, Type t) {
  switch(t) {
    case NONE: return false;
    case UNIT: return false;
    case ANY:  return true;
    default:   return o->type == t;
  }
}

Mtable* val_mtable(Val x) {
  return MetaTables + type_of(x);
}

Mtable* obj_mtable(Obj* o) {
  return MetaTables + o->type;
}

// misc utilities -------------------------------------------------------------
void* construct(Type type, usize n, usize extra, void* data) {
  /* these are needed statically for initialization */
  usize base  = MetaTables[type].size;
  usize objs  = base * n;
  usize total = base * n + extra;

  void* out = allocate(total, 1, 0);

  if (data && extra)
    memcpy(out+objs, data, extra);

  return out;
}

// initialization -------------------------------------------------------------
void type_init(void) {
  // finish metatable initialization (called before anything else) ------------
  for (uint i=NONE; i<NUM_TYPES; i++) {
    MetaTables[i].type      = i;
    MetaTables[i].type_hash = hash_uint(i);
  }
}
