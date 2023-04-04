#ifndef sexpr_h
#define sexpr_h

#include "object.h"

/* fundamental interpreter types (symbol, list, pair) */

// C types --------------------------------------------------------------------
typedef enum {
  INTERNED = 0x01
} symbol_fl_t;

struct symbol_t {
  HEADER;
  symbol_t *left, *right;
  char*       name;
  uword       idno;
  variable_t* toplevel; // toplevel binding
};

struct pair_t {
  HEADER;
  value_t first;
  value_t second;
};

struct list_t {
  HEADER;
  value_t head;
  list_t* tail;
  uint64  arity;
};

// globals --------------------------------------------------------------------
extern list_t EmptyList;
extern data_type_t SymbolType, ListType, PairType;

// APIs -----------------------------------------------------------------------
// symbol API -----------------------------------------------------------------
#define     is_symbol(x) ISA(x, SymbolType)
#define     as_symbol(x) ASP(x, symbol_t)

symbol_t*   symbol(char* name, bool intern);

// pair API -------------------------------------------------------------------
#define     is_pair(x) ISA(x, PairType)
#define     as_pair(x) ASP(x, pair_t)

pair_t*     pair(value_t first, value_t second);

// list API -------------------------------------------------------------------
#define     is_list(x) ISA(x, ListType)
#define     as_list(x) ASP(x, list_t)

list_t*     list(usize n, value_t* args);
list_t*     cons(value_t head, list_t* tail);

#endif
