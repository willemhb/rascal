#ifndef data_table_h
#define data_table_h

#include "data/object.h"

// C types
struct table {
  HEADER;
  ARRAY(value_t);
};

typedef enum {
  EQTABLE = 0x01, // compare with '==?'
  IDTABLE = 0x02, // compare with 'id?'
} table_fl_t;

// APIs & utilities
#define     as_table(x) ((table_t*)((x) & WVMASK))

table_t*    table(void);
void        reset_table(table_t* slf);
void        init_table(table_t* slf);
bool        resize_table(table_t* slf, usize n);
long        table_find(table_t* slf, value_t key);
value_t     table_get(table_t* slf, value_t key);
value_t     table_set(table_t* slf, value_t key, value_t val);
bool        table_add(table_t* slf, value_t key, value_t val);

#endif
