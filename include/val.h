#ifndef rascal_val_h
#define rascal_val_h

#include "types.h"

// mostly just forward declarations and utility macros
#define isa(val, type)				\
  (rtypeof(val)==(type))
#define is_nul(val)      ((val)==NUL)


#define as_val(val)      (((val_data_t)(val)).as_val)
#define as_ptr(val)					\
  _Generic((val),					\
	   val_t:((void*)(as_val((val_t)(val))&PMASK)),	\
	   default:(typeof(val))(val))
#define tag_ptr(val,tag) ((((val_t)(val))&PMASK)|(tag))

// forward declarations
void mark_val(val_t val);
void trace_vals(val_t *vals, size_t n);
void free_val(val_t val);

#endif
