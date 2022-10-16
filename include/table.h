#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"


enum
  {
    mapEntryType   =0,
    setEntryType   =1,
    varEntryType   =2,

    primCmpType    =0,
    nativeCmpType  =1,
    userCmpType    =2,
  };

struct table_t
{
  object_t   object;

  arity_t    len;
  arity_t    eCap;
  object_t **entries;

  flags_t    minCap : 16;
  flags_t    oSize  :  8;
  flags_t    cType  :  2;
  flags_t    eType  :  2;
  flags_t    isWeak :  4;

  arity_t    oCap;
  
  union
  {
    int8_t  *ord8;
    int16_t *ord16;
    int32_t *ord32;
  };

  invoke_t   compare;
  type_t    *keyType, *valType;
};

struct entry_t
{
  object_t object;
  
  hash_t   hash;
  value_t  key;
};

struct assoc_t
{
  entry_t  entry;

  value_t  bind;
};

struct var_t
{
  assoc_t  assoc;
  type_t  *type;
  cons_t  *props;
};

// table methods
table_t *new_table(void);
void     init_table(table_t *table, bool isStatic, arity_t minCap, flags_t eType, flags_t cType, bool isWeak, invoke_t compare, type_t *keyType, type_t *valType );
void     mark_table( object_t *object );
void     free_table( object_t *object );
size_t   table_size( object_t *object );
hash_t   hash_table( object_t *object );
ord_t    order_tables( object_t *x, object_t *y );

void     resize_table( table_t *table, size_t newl );
bool     table_get( table_t *table, value_t key, entry_t **buf );
bool     table_put( table_t *table, value_t key, entry_t **buf );
bool     table_pop( table_t *table, value_t key, entry_t **buf );

table_t *mk_map( bool isWeak, invoke_t compare, type_t *keyType, type_t *valType );

table_t *mk_reader_cache( void );
table_t *mk_printer_cache( void );
table_t *mk_hasher_cache( void );
table_t *mk_order_cache( void );

// entry methods
entry_t *new_entry( void );
void     init_entry( entry_t *entry, value_t key, hash_t hash );
void     mark_entry( object_t *object );
void     hash_entry( object_t *object );
ord_t    order_entries( object_t *x, object_t *y );

// natives


// convenience
#define as_table(val) ((table_t*)as_ptr(val))
#define as_entry(val) ((entry_t*)as_ptr(val))
#define as_assoc(val) ((assoc_t*)as_ptr(val))
#define as_var(val)   ((var_t*)as_ptr(val))


#endif
