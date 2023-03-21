#ifndef atom_h
#define atom_h

#include "value.h"

/* primitive types that don't fit anywhere else */

// globals --------------------------------------------------------------------
extern data_type_t PonterType, BooleanType;

// API ------------------------------------------------------------------------
// pointer --------------------------------------------------------------------
#define is_pointer(x)   (((x) & TAG_MASK) == PTRTAG)
#define as_pointer(x)   ((pointer_t)(((uword)(o)) & VAL_MASK))
#define pointer(p)      ((((uword)(o)) & VAL_MASK) | PTRTAG)

// boolean --------------------------------------------------------------------
#define is_boolean(x) (((x) & TAG_MASK) == BOOLTAG)
#define as_boolean(x) ((x) == TRUE_VAL)
#define boolean(x)    ((x) ? TRUE_VAL : FALSE_VAL)



#endif
