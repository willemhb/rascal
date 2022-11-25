#ifndef rl_util_table_h
#define rl_util_table_h

#include "vm/value.h"

#include "util/hashing.h"

/* commentary 

   general utilities for working with hash tables

     - helpers for resizing, rehashing, etc.
     - internal table types for use in cyclic comparison, hashing, printing, &c. 
 */

/* C types */
typedef enum entry_type_key_t
  {
    entry_type_none   = 0, // invalid
    entry_type_string = 1, // C string
    entry_type_pointer= 2, // raw pointer
    entry_type_integer= 3, // unsigned integer
    entry_type_value  = 4  // tagged value
  } entry_type_key_t;

typedef union entry_type_t
{
  char     *string;
  void     *pointer;
  fixnum_t  integer;
  value_t   value;
} entry_type_t;

typedef struct entry_t
{
  entry_type_key_t key_type, value_type;
  entry_type_t key, value;
  hash_t       hash;
} entry_t;

typedef struct cyc_entry_t
{
  void *key;

  union
  {
    hash_t  hash;
    void   *pointer;
  } value;
} cyc_entry_t;

typedef struct cyc_table_t
{
  cyc_entry_t **entries;
  size_t        capacity;
  size_t        count;
} cyc_table_t;

typedef ulong (*rehash_fn_t)(const void *k);

/* API */
size_t pad_table_size( size_t new_count, size_t old_cap );
void   rehash_table( void **old, void **new, size_t new_cap, rehash_fn_t get_hash );

cyc_table_t *make_cyc_table( void );
void         init_cyc_table( cyc_table_t *cyc_table, size_t count );
void         free_cyc_table( cyc_table_t *cyc_table );



#endif
