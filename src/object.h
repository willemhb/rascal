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

struct table_t {
  HEADER;
  usize count, tcap, ocap;
  value_t *table;

  union {
    void*   ord;
    sint8*  ord8;
    sint16* ord16;
    sint32* ord32;
    sint64* ord64;
  };
};

struct alist_t {
  HEADER;
  usize count, cap;
  value_t *array;
};

// globals --------------------------------------------------------------------
extern list_t EmptyList;
extern data_type_t SymbolType, ListType, TableType, AlistType;

// API ------------------------------------------------------------------------
// lifetime API ---------------------------------------------------------------
#define is_object(x) IST(x, OBJTAG, TAG_MASK)
#define as_object(x) ASP(x, object_t)

#define header(o)    ((object_t*)(o))
#define object(o)    ((((uword)(o)) & VAL_MASK) | OBJTAG)
#define freeze(o)    (((object_t*)(o))->frozen = true)
#define unfreeze(o)  (((object_t*)(o))->frozen = false)

int  init_object(void* self, data_type_t* type);
void mark_object(void* self);
void free_object(void* self);

// traversal utilities --------------------------------------------------------
void mark_objects(usize n, object_t** objs);
void mark_values(usize n, value_t* vals);
void print_objects(usize n, object_t** objs, port_t* ios, bool paired);
void print_values(usize n, value_t* vals, port_t* ios, bool paired);
int  compare_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs);
int  compare_values(usize xn, value_t* xvals, usize yn, value_t* yvals);
bool equal_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs);
bool equal_values(usize xn, value_t* xvals, usize yn, value_t* yvals);
uhash hash_objects(usize n, object_t** objs);
uhash hash_values(usize n, value_t* vals);

// symbol API -----------------------------------------------------------------
#define     is_symbol(x) ISA(x, SymbolType)
#define     as_symbol(x) ASP(x, symbol_t)

symbol_t*   symbol(char* name, bool intern);
bool        is_defined(symbol_t* sym, namespace_t* ns);
bool        is_bound(symbol_t* sym, namespace_t* ns);
value_t     toplevel(symbol_t* sym);
variable_t* defvar(value_t name, namespace_t* ns, string_t* doc, type_t* type, value_t bind);
variable_t* defconst(value_t name, namespace_t* ns, string_t* doc, type_t* type, value_t bind);

// list API -------------------------------------------------------------------
#define     is_list(x) ISA(x, ListType)
#define     as_list(x) ASP(x, list_t)

list_t*     list(usize n, value_t* args);
list_t*     cons(value_t head, list_t* tail);

// table API ------------------------------------------------------------------
#define     is_table(x) ISA(x, TableType)
#define     as_table(x) ASP(x, table_t)

table_t*    table(void);
void        reset_table(table_t* self);
value_t     table_get(table_t* self, value_t key);
value_t     table_set(table_t* self, value_t key, value_t val);
value_t     table_del(table_t* self, value_t key);

// alist API ------------------------------------------------------------------
#define     is_alist(x) ISA(x, AlistType)
#define     as_alist(x) ASP(x, alist_t)

alist_t*    alist(void);
void        reset_alist(alist_t* self);
usize       alist_push(alist_t* self, value_t val);
value_t     alist_pop(alist_t* self);

#endif
