#ifndef object_h
#define object_h

#include "value.h"

/* Object lifetime methods, eval types (types with syntactic relevance), and alist/table. */
// C types --------------------------------------------------------------------
struct symbol_t {
  HEADER;
  symbol_t *left, *right;
  char*       name;
  uword       idno;
  variable_t* toplevel; // toplevel binding
};

struct list_t {
  HEADER;
  usize   len;
  value_t head;
  list_t* tail;
};

struct tuple_t {
  HEADER;
  usize   len;
  value_t slots[];
};

struct table_t {
  HEADER;
  usize count, cap;
  value_t *table;
};

struct alist_t {
  HEADER;
  usize len, cap;
  value_t *array;
};

typedef struct {
  object_type_t* type;
  uhash          hash;
  uint16         flags;
  uint16         frozen;
  uint16         hashed;
  uint8          data;
  uint8          safe;
} object_init_t;

// globals --------------------------------------------------------------------
extern struct list_t  EmptyList;
extern struct tuple_t EmptyTuple;

// API ------------------------------------------------------------------------
// lifetime API ---------------------------------------------------------------
int  init_object(object_t* self, object_init_t* ini);
void mark_object(object_t* self);
void free_object(object_t* self);

// symbol API -----------------------------------------------------------------
value_t     symbol(char* name, bool intern);
variable_t* define(value_t name, namespace_t* ns);

// list API -------------------------------------------------------------------
value_t     list(usize n, value_t* args);
value_t     cons(value_t head, list_t* tail);

value_t     head(value_t xs);
list_t*     tail(value_t xs);

value_t     nth_head(list_t* xs, usize n);
list_t*     nth_tail(list_t* xs, usize n);

// tuple API ------------------------------------------------------------------
value_t     tuple(usize n, value_t* args);

value_t     nth_slot(value_t xs, usize n);

#endif
