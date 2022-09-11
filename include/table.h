#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"
#include "list.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    table_fl_symbol_table  = 1,
    table_fl_cycle_detector= 2,
  } table_fl_t;

typedef struct table_t
{
  HEADER;

  object_t **entries;
  uint cnt, cap;
} table_t;

typedef struct // a pair with a hash hidden after the cdr
{
  cons_t base;
  ulong  hash;
} entry_t;

// globals --------------------------------------------------------------------
extern table_t *Symbols;

// forward declarations -------------------------------------------------------
value_t table(value_t *args, size_t n_args);

// statics and macros ---------------------------------------------------------
#define as_table(x)      asa(table_t*, x, pval)
#define as_entry(x)      asa(entry_t*, x, pval)

#define table_entries(x) getf(table_t*, x, entries)
#define table_cnt(x)     getf(table_t*, x, cnt)
#define table_cap(x)     getf(table_t*, x, cap)
#define entry_hash(x)    getf(entry_t*, x, hash)

static inline tag_p(table, TABLE)


#endif
