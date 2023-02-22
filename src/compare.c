#include "compare.h"
#include "type.h"
#include "object.h"

#include "util/hash.h"
#include "util/number.h"

// C types --------------------------------------------------------------------
typedef struct {
  Table backrefs;
  int bound;
  bool bounded;
} EqualState;

typedef struct {
  Table backrefs;
  int bound;
  bool bounded;
} CompareState;

typedef struct {
  Table backrefs;
  uhash hash;
  int   bound;
  bool  deep;
} HashState;

// globals --------------------------------------------------------------------
uhash EmptyStringHash, EmptyListHash, EmptyTupleHash;

// internal helpers -----------------------------------------------------------
Val  trace_backref(Table* backrefs, Val x);
void save_backref(Table* backrefs, Val a, Val b, Val c, Val cb);

void init_equal_state(EqualState* state);
void free_equal_state(EqualState* state);
void init_compare_state(CompareState* state);
void free_compare_state(CompareState* state);
void init_hash_state(HashState* state, Type type, bool deep);
void free_hash_state(HashState* state);

uhash hash_sym(Val x, void* state);
uhash hash_func(Val x, void* state);
uhash hash_str(Val x, void* state);
uhash hash_bin(Val x, void* state);
uhash hash_list(Val x, void* state);
uhash hash_vec(Val x, void* state);
uhash hash_tuple(Val x, void* state);
uhash hash_table(Val x, void* state);

bool  equal_funcs(Val x, Val y, void* state);
bool  equal_strs(Val x, Val y, void* state);
bool  equal_bins(Val x, Val y, void* state);
bool  equal_lists(Val x, Val y, void* state);
bool  equal_vecs(Val x, Val y, void* state);
bool  equal_tuples(Val x, Val y, void* state);
bool  equal_tables(Val x, Val y, void* state);

int   compare_funcs(Val x, Val y, void* state);
int   compare_strings(Val x, Val y, void* state);
int   compare_bins(Val x, Val y, void* state);
int   compare_strs(Val x, Val y, void* state);
int   compare_lists(Val x, Val y, void* state);
int   compare_vecs(Val x, Val y, void* state);
int   compare_tuples(Val x, Val y, void* state);
int   compare_tables(Val x, Val y, void* state);

void  compare_init(void);

// external API ---------------------------------------------------------------
bool same(Val x, Val y) {
  return x == y;
}

bool equal(Val x, Val y) {
  if (x == y)
    return true;

  Type xt = type_of(x), yt = type_of(y);

  if (xt != yt)
    return false;

  if (MTABLE(xt).equal == NULL)
    return false;

  EqualState state;

  init_equal_state(&state);

  bool out = MetaTables[xt].equal(x, y, &state);

  free_equal_state(&state);

  return out;
}

int compare(Val x, Val y) {
  if (x == y)
    return 0;

  Type xt = type_of(x), yt = type_of(y);

  if (xt != yt)
    return cmp_ints(xt, yt);

  if (MTABLE(xt).compare == NULL)
    return cmp_words(x, y);

  CompareState state;

  init_compare_state(&state);

  int out = MTABLE(xt).compare(x, y, &state);

  free_compare_state(&state);

  return out;
}

uhash hash(Val x, bool deep) {
  Type xt = type_of(x);

  if (MTABLE(xt).hash == NULL)
    return hash_uint(x);

  HashState state;

  init_hash_state(&state, xt, deep);

  uhash out = MTABLE(xt).hash(x, &state);

  free_hash_state(&state);

  return out;
}

// internal API ---------------------------------------------------------------
Val trace_backref(Table* backrefs, Val x) {
  Val c = table_get(backrefs, x);

  if (c == NOTFOUND)
    return NUL;

  if (c == x)
    return c;

  return trace_backref(backrefs, c);
}

void save_backref(Table* backrefs, Val a, Val b, Val c, Val cb) {
  Val ca = (c == NUL ? a : c);

  if (cb != NUL)
    table_set(backrefs, cb, ca);

  table_set(backrefs, a, ca);
  table_set(backrefs, b, ca);
}

// state lifetime -------------------------------------------------------------
#define COMPARE_BOUND 128
#define HASH_BOUND    16384

void init_equal_state(EqualState* state) {
  init_table(&state->backrefs, CSTACK, 0, NULL);
  state->bound   = COMPARE_BOUND;
  state->bounded = true;
}

void free_equal_state(EqualState* state) {
  free_table(&state->backrefs);
}

void init_compare_state(CompareState* state) {
  init_table(&state->backrefs, CSTACK, 0, NULL);
  state->bound   = COMPARE_BOUND;
  state->bounded = true;
}

void free_compare_state(CompareState* state) {
  free_table(&state->backrefs);
}

void init_hash_state(HashState* state, Type type, bool deep) {
  if (deep)
    init_table(&state->backrefs, CSTACK, 0, NULL);

  state->deep  = deep;
  state->hash  = MTABLE(type).type_hash;
  state->bound = HASH_BOUND;
}

void free_hash_state(HashState* state) {
  if (state->deep)
    free_table(&state->backrefs);
}

// hash dispatch --------------------------------------------------------------
uhash hash_sym(Val x, void* state) {
  (void)state;
  Sym* sym = as_sym(x);

  if (!has_flag(sym, HASHED)) {
    uhash th  = MTABLE(SYM).type_hash;
    uhash ih  = hash_uint(sym->arity);
    uhash sh  = hash_string(sym->name);
    sym->hash = mix_hashes(3, sh, ih, th);

    set_flag(sym, HASHED);
  }

  return sym->hash;
}

uhash hash_func(Val x, void* state) {
  HashState* hstate = state;

  if (!hstate->deep)
    return hash_uint(x); 
}

uhash hash_str(Val x, void* state) {
  (void)state;
  Str* str = as_str(x);

  if (!has_flag(str, HASHED)) {
    if (str == &EmptyString) {
      str->hash = EmptyStringHash;
    } else {
      uhash th  = MTABLE(STR).type_hash;
      uhash sh  = hash_string(str->chars);
      str->hash = mix_hashes(2, sh, th);
    }

    set_flag(str, HASHED);
  }

  return str->hash;
}

uhash hash_bin(Val x, void* state) {
  Bin* bin = as_bin(x);

  
}
