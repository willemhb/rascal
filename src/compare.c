#include <string.h>

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
uhash hash_symbol(value_t x);
uhash hash_tuple(value_t x);
uhash hash_list(value_t x);
uhash hash_vector(value_t x);
uhash hash_binary(value_t x);
uhash hash_stencil(value_t x);

// globals --------------------------------------------------------------------
uhash NulHash = 0, TrueHash = 0, FalseHash = 0;

uhash TypeHashes[NUM_TYPES];

uhash (*Hash[NUM_TYPES])(value_t x) = {
  [UNIT]   = hash_unit,   [BOOL]    = hash_bool,
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

  uhash out = Hash[xt] ? Hash[xt](x) : hash_uword(x);

  return out & VAL_MASK; // compress into 48 bits
}

// equal methods --------------------------------------------------------------
bool  equal_tuples(value_t x, value_t y) {
  tuple_t* tx = as_tuple(x),* ty = as_tuple(y);

  if (tx->len != ty->len)
    return false;

  for (usize i=0; i<tx->len; i++) {
    if (!equal(tx->slots[i], ty->slots[i]))
      return false;
  }

  return true;
}

bool equal_lists(value_t x, value_t y) {
  list_t* lx = as_list(x),* ly = as_list(y);

  if (lx->len != ly->len)
    return false;

  while (lx->len) {
    if (!equal(lx->head, ly->head))
      return false;

    lx = lx->tail, ly = ly->tail;
  }

  return true;
}

bool  equal_binaries(value_t x, value_t y) {
  binary_t* bx = as_binary(x),*by = as_binary(y);

  if (bx->len != by->len)
    return false;

  if (bx->len)
    return memcmp(bx->array, by->array, bx->len);

  return true;
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

int compare_symbols(value_t x, value_t y) {
  symbol_t* sx = as_symbol(x),* sy = as_symbol(y);

  return strcmp(sx->name, sy->name) ? : CMP(sx->idno, sy->idno);
}

int compare_tuples(value_t x, value_t y) {
  tuple_t* tx  = as_tuple(x),* ty = as_tuple(y);
  usize maxcmp = MIN(tx->len, ty->len);
  int o;

  for (usize i=0; i<maxcmp; i++) {
    if ((o=compare(tx->slots[i], ty->slots[i])))
      return o;
  }

  return 0 - (tx->len < ty->len) + (tx->len > ty->len);
}

int compare_lists(value_t x, value_t y) {
  list_t* lx   = as_list(x),* ly = as_list(y);
  usize maxcmp = MIN(lx->len, ly->len);
  int o;

  for (usize i=0; i<maxcmp; i++) {
    if ((o=compare(lx->head, ly->head)))
      return o;
  }

  return 0 - !!lx->len - !!ly->len;
}

int compare_binaries(value_t x, value_t y) {
  binary_t* bx = as_binary(x),* by = as_binary(y);
  usize maxcmp = MIN(bx->len, by->len);
  int o;

  if (maxcmp && (o=memcmp(bx->array, by->array, maxcmp)))
    return o;

  return 0 - (bx->len < by->len) + (bx->len > by->len);
}

// hash methods ---------------------------------------------------------------
uhash hash_unit(value_t x) {
  (void)x;

  return NulHash;
}

uhash hash_bool(value_t x) {
  return x == TRUE_VAL ? TrueHash : FalseHash;
}

uhash hash_symbol(value_t x) {
  symbol_t* sx = as_symbol(x);

  if (!has_flag(sx, HASHED)) {
    uhash base     = TypeHashes[SYMBOL];
    uhash ihash    = hash_uword(sx->idno);
    uhash nhash    = hash_str(sx->name);
    sx->obj.hash   = mix_3_hashes(base, nhash, ihash);
    sx->obj.flags |= HASHED;
  }

  return sx->obj.hash;
}

uhash hash_tuple(value_t x) {
  tuple_t* tx = as_tuple(x);

  if (!has_flag(tx, HASHED)) {
    uhash h = TypeHashes[TUPLE];
    
    for (usize i=0; i<tx->len; i++)
      h = mix_2_hashes(h, hash(tx->slots[i]));
    
    tx->obj.hash   = h;
    tx->obj.flags |= HASHED;
  }

  return tx->obj.hash;
}

uhash hash_list(value_t x) {
  list_t* lx = as_list(x);

  // TODO: convert to loop
  if (!has_flag(lx, HASHED)) {
    uhash base    = TypeHashes[LIST];
    uhash hdh     = hash(lx->head);

    if (lx->len)
      lx->obj.hash = mix_3_hashes(base, hdh, hash_list(tag_ptr(lx->tail, OBJTAG)));

    else
      lx->obj.hash = mix_2_hashes(base, hdh);

    lx->obj.flags |= HASHED;
  }

  return lx->obj.hash;
}

uhash hash_binary(value_t x) {
  binary_t* bin = as_binary(x);

  if (!has_flag(bin, HASHED)) {
    uhash base      = TypeHashes[BINARY];
    uhash mhash     = hash_mem(bin->array, bin->len);

    bin->obj.hash   = mix_2_hashes(base, mhash);
    bin->obj.flags |= HASHED;
  }

  return bin->obj.hash;
}

// initialization -------------------------------------------------------------
void compare_init(void) {
  NulHash   = hash_uword(NUL);
  TrueHash  = hash_uword(TRUE_VAL);
  FalseHash = hash_uword(FALSE_VAL);

  // initialize type hashes ---------------------------------------------------
  for (type_t t = UNIT; t < NUM_TYPES; t++)
    TypeHashes[t] = hash_uword(t);
}
