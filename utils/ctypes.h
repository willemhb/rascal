#ifndef ctypes_h
#define ctypes_h

#include <stddef.h>
#include <stdbool.h>

typedef enum {
  C_sint8,
  C_uint8,
  C_sint16,
  C_uint16,
  C_sint32,
  C_uint32,
  C_sint64,
  C_float64
} Ctype_t;

// exports --------------------------------------------------------------------
size_t Ctype_size(Ctype_t xct);

bool   Ctype_sintp(Ctype_t xct);
bool   Ctype_uintp(Ctype_t xct);
bool   Ctype_floatp(Ctype_t xct);
bool   Ctype_floatp(Ctype_t xct);

Ctype_t Ctype_common( Ctype_t xct, Ctype_t yct );

#endif
