#ifndef internal_h
#define internal_h

#include "object.h"

/* sundry internal object types */
// C types --------------------------------------------------------------------

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
extern data_type_t TableType, AlistType;

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
