#ifndef rascal_record_h
#define rascal_record_h

#include "object.h"

typedef union
{
  value_t   value;
  object_t *object;
} slot_t;

struct record_t
{
  object_t  object;
  type_t   *type;
  
  value_t   slots[0];
};

// creating new record types
record_t *new_record( type_t *type, size_t n );
void      init_record( record_t *record, type_t *type, size_t n, value_t *ini );
void      mark_record( object_t *object );
void      free_record( object_t *object );
size_t    record_size( object_t *object );
hash_t    hash_record( object_t *object );
void      order_records( object_t *x, object_t *y );

value_t   record_getf_i( record_t *record, arity_t i );
value_t   record_getf_n( record_t *record, symbol_t *n );
value_t   record_getf_v( record_t *record, value_t v );

value_t   record_setf_i( record_t *record, arity_t i, value_t v );
value_t   record_setf_n( record_t *record, symbol_t *n, value_t v );
value_t   record_setf_v( record_t *record, value_t n, value_t v );

type_t   *new_record_type( symbol_t *name, size_t n, value_t *slots );
type_t   *prim_record_type( char *name, native_fn_t constructor, size_t n, ...);

// convenience
#define record_getf(record, x)				\
  _Generic((x),						\
	   arity_t:record_getf_i,			\
	   symbol_t*:record_getf_n,			\
	   value_t:record_getf_v)((record), (x))


#define record_setf(record, x, v)				\
  _Generic((x),							\
	   arity_t:record_setf_i,				\
	   symbol_t*:record_setf_n,				\
	   value_t:record_setf_v)((record), (x), (v))

#define as_record(val)    ((record_t*)as_ptr(val))

#define record_type(val)  (as_record(val)->type)
#define record_slots(val) (as_record(val)->slots)



#endif
