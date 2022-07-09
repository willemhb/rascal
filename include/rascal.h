#ifndef rascal_h
#define rascal_h

#include <stdio.h>

#include "common.h"
#include "types.h"
#include "globals.h"

typedef union {
  value_t word;

  struct {
    value_t type  :  8;
    value_t data  : 48;
    value_t flags :  8;
  };
} imparts_t;

// macros for pseudo-objects --------------------------------------------------
#define rnull   ((value_t)tag_cons)
#define rnone   ((value_t)((0xffffffff00ul)|type_none))
#define rtrue   ((value_t)((0x0000000100ul)|type_boolean))
#define rfalse  ((value_t)((0x0000000000ul)|type_boolean))
#define rfptr   ((value_t)tag_symbol)
#define rempty  ((value_t)tag_table)

// utility macros -------------------------------------------------------------
#define asptr(x)    ((void*)(((value_t)(x))&~7ul))
#define asint(x)    ((int)((x)>>8))

#define vtag(x)     ((x)&7)
#define wtag(x)     ((x)&255)

#define tagp(x, t)    (((value_t)(x))|(t))
#define tagi(i, t)    ((((value_t)(x))<<8)|(t))

#define imflags(x)    (((imparts_t)(x)).flags)
#define imdata(x)     (((imparts_t)(x)).data)
#define imtype(x)     (((imparts_t)(x)).type)

#define flagp(x, fl)  (((x)&(fl))==(fl))

#define nullp(x)  ((x)==rnull)
#define nonep(x)  ((x)==rnone)
#define truep(x)  ((x)==rtrue)
#define falsep(x) ((x)==rfalse)
#define fptrp(x)  ((x)==rfptr)
#define emptyp(x) ((x)==rempty)

#define obdata(x) (((void**)asptr(x))[3])

#endif
