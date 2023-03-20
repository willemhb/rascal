#ifndef object_h
#define object_h

#include "value.h"

/* Object lifetime methods, eval types (types with syntactic relevance), and alist/table. */
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
extern struct object_type_t SymbolType, ListType, TupleType, TableType, AlistType;

// API ------------------------------------------------------------------------
// lifetime API ---------------------------------------------------------------
#define is_object(x) (((x) & TAG_MASK) == OBJTAG)
#define as_object(x) ((object_t*)((x) & VAL_MASK))
#define object(o)    ((((uword)(o)) & VAL_MASK) | OBJTAG)
#define freeze(o)    (((object_t*)(o))->frozen = true)

int  init_object(void* self, void* ini);
void mark_object(void* self);
void free_object(void* self);

// traversal utilities --------------------------------------------------------
void mark_objects(usize n, object_t** objs);
void mark_values(usize n, value_t* vals);
int  compare_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs);
int  compare_values(usize xn, value_t* xvals, usize yn, value_t* yvals);
bool equal_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs);
bool equal_values(usize xn, value_t* xvals, usize yn, value_t* yvals);
uhash hash_objects(usize n, object_t** objs);
uhash hash_values(usize n, value_t* vals);

// symbol API -----------------------------------------------------------------
#define     is_symbol(x) has_type(x, &SymbolType.type)
#define     as_symbol(x) ((symbol_t*)((x) & VAL_MASK))

value_t     symbol(char* name, bool intern);
bool        is_defind(symbol_t* sym, namespace_t* ns);
bool        is_bound(symbol_t* sym, namespace_t* ns);
value_t     toplevel(symbol_t* sym);
variable_t* defvar(value_t name, namespace_t* ns, string_t* doc, rl_type_t* type, value_t bind);
variable_t* defconst(value_t name, namespace_t* ns, string_t* doc, rl_type_t* type, value_t bind);

// list API -------------------------------------------------------------------
#define     is_list(x) has_type(x, &ListType.type)
#define     as_list(x) ((list_t*)((x) & VAL_MASK))

value_t     list(usize n, value_t* args);
value_t     cons(value_t head, list_t* tail);

// tuple API ------------------------------------------------------------------
#define     is_tuple(x) has_type(x, &TupleType)
#define     as_tuple(x) ((tuple_t*)((x) & VAL_MASK))

value_t     tuple(usize n, value_t* args);

// table API ------------------------------------------------------------------
#define     is_table(x) has_type(x, &TableType.type)
#define     as_table(x) ((table_t*)((x) & VAL_MASK))

table_t*    table(void);
value_t     table_get(table_t* self, value_t key);
value_t     table_set(table_t* self, value_t key, value_t val);
value_t     table_del(table_t* self, value_t key, value_t val);

// alist API ------------------------------------------------------------------
#define     is_alist(x) has_type(x, &AlistType.type)
#define     as_alist(x) ((alist_t*)((x) & VAL_MASK))

alist_t*    alist(void);
usize       alist_push(alist_t* self, value_t val);
value_t     alist_pop(alist_t* self);

#endif
