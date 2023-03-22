#ifndef atom_h
#define atom_h

#include "value.h"

/* primitive types that don't fit anywhere else */

// globals --------------------------------------------------------------------
extern data_type_t PonterType, BooleanType;

// API ------------------------------------------------------------------------
// pointer --------------------------------------------------------------------
#define is_pointer(x)   IST(x, PTRTAG, TAG_MASK)
#define as_pointer(x)   ASV(x, pointer_t)
#define pointer(p)      TAGV(p, PTRTAG)

// boolean --------------------------------------------------------------------
#define is_boolean(x) IST(x, BOOLTAG, WTAG_MASK)
#define as_boolean(x) ((x) == TRUE_VAL)
#define boolean(x)    ((x) ? TRUE_VAL : FALSE_VAL)

#endif
