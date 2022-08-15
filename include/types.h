#ifndef rascal_types_h
#define rascal_types_h

#include "common.h"
// C types --------------------------------------------------------------------
struct enum_type_t {
  value_t  idno;

  char_t  *name;
  tuple_t *members;
};

struct union_type_t {
  value_t  idno;

  char_t  *name;
  tuple_t *members;
};

struct data_type_t {
  value_t   idno;

  char_t   *name;
  table_t  *slots;

  bool_t    is_builtin;
  Ctype_t   Ctype;
  size_t    base_size;
  uintptr_t value_tag;

  /* object model methods */
  size_t  (*size)(value_t x);
  size_t  (*print)(value_t x, port_t *ios);
  hash_t  (*hash)(value_t x);
  sint_t  (*order)(value_t x, value_t y);
  value_t (*relocate)(value_t x);
};

// utilities ------------------------------------------------------------------

// implementation -------------------------------------------------------------

// native functions -----------------------------------------------------------

// initialization -------------------------------------------------------------
void types_globals_init(void);
void types_types_init(void);
void types_natives_init(void);

#endif
