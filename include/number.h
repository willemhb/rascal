#ifndef rascal_number_h
#define rascal_number_h

#include "val.h"

#define is_real(val)     ((val)&QNAN != QNAN)
#define is_chr(val)      (((val)&TMASK)==CHRTAG)
#define as_real(val)     (((val_data_t)(val)).as_real)
#define as_chr(val)      ((int32_t)((((val_data_t)(val)).as_val)&(value_t)UINT32_MAX))


// forward declarations for object apis
void      prin_real(stream_t *port, value_t value);
void      prin_chr(stream_t *port, value_t value);

// arithmetic functions
value_t native_add(value_t *args, arity_t n);
value_t native_sub(value_t *args, arity_t n);
value_t native_mul(value_t *args, arity_t n);
value_t native_div(value_t *args, arity_t n);
value_t native_rem(value_t *args, arity_t n);

#endif
