#ifndef rascal_types_h
#define rascal_types_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "Ctypes.h"

#define QNAN  0x7ffc000000000000ul
#define SIGN  0x8000000000000000ul

#define HTAG0 0x0000000000000000ul
#define HTAG1 0x0001000000000000ul
#define HTAG2 0x0002000000000000ul
#define HTAG3 0x0003000000000000ul

#define HTMASK (SIGN|QNAN|HTAG3)

#define PMASK (~HTMASK)
#define HMASK (PMASK&~15ul)
#define LMASK PMASK
#define IMASK UINT32_MAX
#define UMASK IMASK

// tags  ----------------------------------------------------------------------
#define NIL       (QNAN|HTAG0)
#define BOOLEAN   (QNAN|HTAG1)
#define CHARACTER (QNAN|HTAG2)
#define INTEGER   (QNAN|HTAG3)

#define OBJECT    (SIGN|QNAN|HTAG0)
#define IMMEDIATE (SIGN|QNAN|HTAG1)
#define ESCAPE    (SIGN|QNAN|HTAG2)
#define POINTER   (SIGN|QNAN|HTAG3)

// C types --------------------------------------------------------------------
typedef uintptr_t value_t;

typedef struct
{
  uint   arity;       // abstract length, no 
  ushort builtin;     // the builtin function that constructed this object
  ushort size   : 12; // object base size
  ushort gcbits :  2;
  ushort moved  :  1;
  ushort seen   :  1;
} object_t;

typedef union
{
  double     d;
  value_t    u;
  
  struct
  {
    long l : 48;
    long   : 16;
  };
} word_t;

// macros & utils -------------------------------------------------------------
#define HEADER object_t base

#define pval(x)						\
  (_Generic((x),					\
	   value_t:(void*)(((value_t)(x))&PMASK),	\
	   default:(void*)(x)))

#define tagp(x) (((value_t)(x))|OBJECT)

#define asob(x)  ((object_t*)pval(x))

#define ob_arity(x)      (asob(x)->arity)
#define ob_size(x)       (asob(x)->size)
#define ob_builtin(x)    (asob(x)->type)
#define ob_gcbits(x)     (asob(x)->gcbits)
#define ob_moved(x)      (asob(x)->moved)
#define ob_seen(x)       (asob(x)->seen)

// forward declarations -------------------------------------------------------
void init_object(object_t *o, uint arity, ushort builtin, ushort size);

#endif
