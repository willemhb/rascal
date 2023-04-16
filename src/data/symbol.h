#ifndef data_symbol_h
#define data_symbol_h

#include "data/object.h"

// C types
struct symbol {
  HEADER;
  symbol_t* left, * right;
  uint64 idno;
  char*  name;
};

typedef enum {
  INTERNED = 0x01
} symfl_t;

// API & utilities
#define is_symbol(x) rl_isa(x, SYMBOL)
#define as_symbol(x) rl_asa(x, WVMASK, symbol_t*)  

symbol_t* symbol(char* name, bool intern);

#endif
