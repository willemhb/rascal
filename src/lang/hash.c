#include <string.h>

#include "object.h"
#include "hash.h"

/* globals */
#define BOUNDED_HASH_BOUND 16384

uhash ObjTypeHash[NUM_OBJ_TYPES];

/* API */
// local helpers --------------------------------------------------------------
static uhash hash_sym_obj(Val x);
static uhash hash_port_obj(Val x);
static uhash hash_native_fn_obj(Val x);
static uhash hash_str_obj(Val x);
static uhash hash_bin_obj(Val x);
static uhash hash_pair_obj(Val x, int bound, uhash *accum);
static uhash hash_list_obj(Val x, int bound, uhash *accum);
static uhash hash_vec_obj(Val x, int bound, uhash *accum);
static uhash hash_table_obj(Val x, int bound, uhash *accum);

static uhash bounded_hash(Val x, int bound, uhash *accum);
static uhash pointer_hash(Val x);

// toplevel hashing dispatch --------------------------------------------------
uhash hash_val(Val x, bool deep_hash) {
  if (deep_hash) {
    uhash accum = 0;

    return bounded_hash(x, BOUNDED_HASH_BOUND, &accum);
  }

  return pointer_hash(x);
}

// leaf type hashes -----------------------------------------------------------
static uhash hash_sym_obj(Val x) {
  return as_sym(x)->hash;
}

static uhash hash_port_obj(Val x) {
  return mix_hashes(ObjTypeHash[PORT_OBJ], hash_ptr(as_port(x)->ios));
}

static uhash hash_native_fn_obj(Val x) {
  return mix_hashes(ObjTypeHash[NATIVE_FN_OBJ], as_native_fn(x)->name->hash);
}

static uhash hash_str_obj(Val x) {
  return as_str(x)->hash;
}

static uhash hash_bin_obj(Val x) {
  Bin* xb = as_bin(x);

  return mix_hashes(ObjTypeHash[BIN_OBJ], hash_mem(xb->array, xb->count));
}

/* initialization */
void hash_init(void) {
  for (int ot=SYM_OBJ; ot<=STR_OBJ; ot++)
    ObjTypeHash[ot] = hash_uint(ot);
}
