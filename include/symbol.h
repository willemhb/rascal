#ifndef rascal_symbol_h
#define rascal_symbol_h

#include "common.h"

// C types --------------------------------------------------------------------
struct symbol_t {
  uint_t   length;
  uchar_t  gensym;
  uchar_t  keyword;
  ushort_t tag;

  char name[0];
};

// utilities ------------------------------------------------------------------

// implementation -------------------------------------------------------------
size_t  symbol_size(value_t x);
hash_t  symbol_hash(value_t x);
sint_t  symbol_order(value_t x, value_t y);
value_t symbol_value(value_t s, value_t e);

// native functions -----------------------------------------------------------
value_t native_symbol(value_t *args, size_t n_args);
value_t native_symbol_p(value_t *args, size_t n_args);
value_t native_gensym_p(value_t *args, size_t n_args);
value_t native_keyword_p(value_t *args, size_t n_args);
value_t native_bound_p(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void symbol_init(void);

#endif
