#ifndef rascal_interface_primitive_h
#define rascal_interface_primitive_h

#include "common.h"
#include "ctypes.h"
#include "rtypes.h"
#include "types.h"

// api ------------------------------------------------------------------------
bool_t numberp( value_t x );
bool_t characterp( value_t x );
bool_t Ctypep( value_t x, Ctype_t t );

int_t  prim_init( type_t t, flags_t fl, size_t n, value_t ini, void *spc );
int_t  prim_order( value_t x, value_t y );
hash_t prim_hash( value_t x );

// initialization -------------------------------------------------------------
void init_primitive( void );

#endif
