#ifndef rascal_table_h
#define rascal_table_h

#include "obj.h"

typedef struct table_t table_t;

typedef enum
  {
    table_fl_symt=1,
    table_fl_envt=2,
  } table_fl_t;

typedef obj_t *(*intern_fn_t)(table_t *table, val_t key, hash_t hash);

typedef struct table_t
{
  OBJ_HEAD;

  objs_t   entries;
  int     *locations;

  ord_fn_t    ord;
  hash_fn_t   hash;
  intern_fn_t intern;
} table_t;

// globals
extern table_t *empty_table;

// api declaration
table_t *new_table(arity_t n, flags_t fl);
void     init_table(table_t *table, arity_t n, flags_t fl);
void     finalize_table(table_t *table);
void     trace_table(obj_t *obj);

bool     table_get(table_t *table, val_t key, obj_t **buf);
bool     table_put(table_t *table, val_t key, obj_t **buf);
bool     table_pop(table_t *table, val_t key, obj_t **buf);

// convenience
#define as_table(val) ((table_t*)as_obj(val))
#define is_table(val) (is_obj_type(val, table_type))

#define table_len(obj)     (((table_t*)(obj))->entries.len)
#define table_cap(obj)     (((table_t*)(obj))->entries.cap)
#define table_entries(obj) (((table_t*)(obj))->entries.data)
#define kv_key(obj)        (((kv_t*)(obj))->cons.hd)
#define kv_val(obj)        (((kv_t*)(obj))->cons.tl)
  

#endif
