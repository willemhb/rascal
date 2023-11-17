#ifndef rl_val_ptr_h
#define rl_val_ptr_h

#include "val/value.h"

/* Support for low-level types. */

/* Globals */
extern Type PointerType, FuncPtrType;

/* External API */
/* Pointer type */
#define is_ptr(x)  has_type(x, &PointerType)
#define as_ptr(x)  as(Pointer, untag48, x)

#define is_fptr(x) has_type(x, &FuncPtrType)
#define as_fptr(x) as(FuncPtr, untag48, x)

/* Initialization */

#endif
