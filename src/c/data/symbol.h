#ifndef data_symbol_h
#define data_symbol_h

#include "data/object.h"

// C types
struct symbol {
  HEADER;
  symbol_t*   left, * right;
  uint64      idno;
  char*       name;
  value_t     bind;
};

typedef enum {
  INTERNED = 0x01,
  LITERAL  = 0x02,
  CONSTANT = 0x04
} symfl_t;

// API & utilities
#define is_symbol(x) (rascal_type(x) == SYMBOL)
#define as_symbol(x) ((symbol_t*)(((value_t)(x)) & WVMASK))

symbol_t* intern_symbol(char* name);
symbol_t* make_symbol(char* name, flags fl);

#endif
