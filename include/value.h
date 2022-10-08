#ifndef rascal_val_h
#define rascal_val_h

#include "types.h"

// mostly just forward declarations and utility macros
#define isa(val, type)	 (rtypeof(val)==(type))
#define is_nul(val)      ((val)==NUL)

#define as_val(val)      (((val_data_t)(val)).as_val)
#define as_ptr(val)					\
  _Generic((val),					\
	   value_t:((void*)(as_val((value_t)(val))&PMASK)),	\
	   default:(typeof(val))(val))
#define tag_ptr(val,tag) ((((value_t)(val))&PMASK)|(tag))

// forward declarations
void mark_val( value_t value );
void mark_vals( value_t *vals, size_t n );
void free_val( value_t value );

#endif
