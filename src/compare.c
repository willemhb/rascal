#include <string.h>
#include <stdio.h>

#include "compare.h"
#include "number.h"
#include "hashing.h"

// forward declarations -------------------------------------------------------
bool  equal_tuples(value_t x, value_t y);
bool  equal_lists(value_t x, value_t y);
bool  equal_vectors(value_t x, value_t y);
bool  equal_binaries(value_t x, value_t y);
bool  equal_stencils(value_t x, value_t y);

int   compare_bools(value_t x, value_t y);
int   compare_reals(value_t x, value_t y);
int   compare_fixnums(value_t x, value_t y);
int   compare_symbols(value_t x, value_t y);
int   compare_tuples(value_t x, value_t y);
int   compare_lists(value_t x, value_t y);
int   compare_vectors(value_t x, value_t y);
int   compare_binaries(value_t x, value_t y);
int   compare_stencils(value_t x, value_t y);

uhash hash_unit(value_t x);
uhash hash_bool(value_t x);
uhash hash_object(void* self);
uhash hash_symbol(void* self);
uhash hash_tuple(void* self);
uhash hash_list(void* self);
uhash hash_vector(void* self);
uhash hash_binary(void* self);
uhash hash_stencil(void* self);

// globals --------------------------------------------------------------------
uhash NulHashVal = 0, TrueHashVal = 0, FalseHashVal = 0;

uhash TypeHash[NUM_TYPES];

uhash (*HashVal[NUM_TYPES])(value_t x) = {
  [UNIT]   = hash_unit,   [BOOL]    = hash_bool
};

uhash (*HashObj[NUM_TYPES])(void* ptr) = {
  [SYMBOL] = hash_symbol, [TUPLE]   = hash_tuple,
  [LIST]   = hash_list,   [VECTOR]  = hash_vector,
  [BINARY] = hash_binary, [STENCIL] = hash_stencil

};

bool (*Equal[NUM_TYPES])(value_t x, value_t y) = {
  [TUPLE]   = equal_tuples,   [LIST]   = equal_lists,
  [VECTOR]  = equal_vectors,  [BINARY] = equal_binaries,
  [STENCIL] = equal_stencils
};

int (*Compare[NUM_TYPES])(value_t x, value_t y) = {
  [BOOL]    = compare_bools,    [REAL]   = compare_reals,
  [FIXNUM]  = compare_fixnums,  [SYMBOL] = compare_symbols,
  [TUPLE]   = compare_tuples,   [LIST]   = compare_lists,
  [VECTOR]  = compare_vectors,  [BINARY] = compare_binaries,
  [STENCIL] = compare_stencils
};

// API ------------------------------------------------------------------------
bool same(value_t x, value_t y) {
  return x == y;
}

bool equal(value_t x, value_t y) {
  if (x == y)
    return true;

  type_t xt = type_of(x), yt = type_of(y);

  if (xt != yt)
    return false;

  if (Equal[xt])
    return Equal[xt](x, y);

  return false;
}

int compare(value_t x, value_t y) {
  if (x == y)
    return 0;

  type_t xt = type_of(x), yt = type_of(y);

  if (xt != yt)
    return CMP(xt, yt);

  if (Compare[xt])
    return Compare[xt](x, y);

  return CMP(as_word(x), as_word(y));
}

uhash hash(value_t x) {
  type_t xt = type_of(x);

  if (is_object_type(xt)) {
    if (HashObj[xt])
      return hash_object(as_object(x));
  }

  return HashVal[xt] ? HashVal[xt](x) : hash_uword(x) & VAL_MASK; // compress to 48 bits
}

// helpers --------------------------------------------------------------------
uhash hash_object(void* ptr) {
  object_t* obj = ptr;

  if (!has_flag(ptr, HASHED)) {
    uhash base  = TypeHash[obj->type];
    uhash vals  = HashObj[obj->type](ptr);
    obj->flags |= HASHED;
    obj->hash   = mix_2_hashes(base, vals) & VAL_MASK;
  }

  return obj->hash;
}

// equal methods --------------------------------------------------------------
bool  equal_tuples(value_t x, value_t y) {
  tuple_t* xt = as_tuple(x),* yt = as_tuple(y);

  return equal_value_arrays(xt->len, xt->slots, yt->len, yt->slots);
}

bool equal_vectors(value_t x, value_t y) {
  vector_t* vx = as_vector(x),* vy = as_vector(y);

  if (vx->len != vy->len)
    return false;

  return equal_stencils(tag_ptr(vx->vals, OBJTAG), tag_ptr(vy->vals, OBJTAG));
}

bool equal_binaries(value_t x, value_t y) {
  binary_t* bx = as_binary(x),*by = as_binary(y);

  if (bx->len != by->len)
    return false;

  if (bx->len)
    return memcmp(bx->array, by->array, bx->len);

  return true;
}

bool equal_stencils(value_t x, value_t y) {
  stencil_t* xst = as_stencil(x),* yst = as_stencil(y);

  return equal_value_arrays(stencil_len(xst), xst->array, stencil_len(yst), yst->array);
}

// compare methods ------------------------------------------------------------
int compare_bools(value_t x, value_t y) {
  return as_bool(x) - as_bool(y);
}

int compare_reals(value_t x, value_t y) {
  return CMP(as_real(x), as_real(y));
}

int compare_fixnums(value_t x, value_t y) {
  return CMP(as_fixnum(x), as_fixnum(y));
}

int compare_tuples(value_t x, value_t y) {
  tuple_t* tx  = as_tuple(x),* ty = as_tuple(y);

  return compare_value_arrays(tx->len, tx->slots, ty->len, ty->slots);
}

int compare_vectors(value_t x, value_t y) {
  vector_t* vx = as_vector(x),* vy = as_vector(y);
  int o = compare_stencils(object(vx->vals), object(vy->vals));

  if (o)
    return o;

  return 0 - (vx->len < vy->len) + (vx->len > vy->len);
}

int compare_binaries(value_t x, value_t y) {
  binary_t* bx = as_binary(x),* by = as_binary(y);
  usize maxcmp = MIN(bx->len, by->len);
  int o;

  if (maxcmp && (o=memcmp(bx->array, by->array, maxcmp)))
    return o;

  return 0 - (bx->len < by->len) + (bx->len > by->len);
}

int compare_stencils(value_t x, value_t y) {
  stencil_t* stx = as_stencil(x),* sty = as_stencil(y);

  return compare_value_arrays(stencil_len(stx), stx->array, stencil_len(sty), sty->array);
}

// hash methods ---------------------------------------------------------------
uhash hash_unit(value_t x) {
  (void)x;

  return NulHashVal;
}

uhash hash_bool(value_t x) {
  return x == TRUE_VAL ? TrueHashVal : FalseHashVal;
}

uhash hash_symbol(void* ptr) {
  symbol_t* sx = ptr;

  uhash ihash    = hash_uword(sx->idno);
  uhash nhash    = hash_str(sx->name);

  return mix_2_hashes(nhash, ihash);
}

uhash hash_tuple(void* ptr) {
  tuple_t* tx = ptr;

  return hash_value_array(tx->len, tx->slots);
}

uhash hash_vector(void* ptr) {
  vector_t* vx = ptr;

  return hash_object(vx->vals);
}

uhash hash_binary(void* ptr) {
  binary_t* bin = ptr;
  
  return hash_mem(bin->array, bin->len);
}

uhash hash_stencil(void* ptr) {
  stencil_t* stx = ptr;

  return hash_value_array(stencil_len(stx), stx->array);
}

// initialization -------------------------------------------------------------
void compare_init(void) {
  NulHashVal   = hash_uword(NUL);
  TrueHashVal  = hash_uword(TRUE_VAL);
  FalseHashVal = hash_uword(FALSE_VAL);

  // initialize type hashes ---------------------------------------------------
  for (type_t t = NONE; t < NUM_TYPES; t++) {
    TypeHash[t] = hash_uword(t);

#ifdef RASCAL_DEBUG
    // printf("hash of %s: %lu\n", type_name(t), TypeHash[t]);
#endif
  }
}
