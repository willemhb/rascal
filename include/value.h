#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

/* tags used by the vm */
#define QNAN     0x7ffc000000000000ul
#define IMMTAG   0x7ffc000000000000ul
#define FIXTAG   0x7ffd000000000000ul
#define PTRTAG   0x7ffe000000000000ul
#define OBJTAG   0x7fff000000000000ul
#define HDRTAG   0xfffc000000000000ul

#define NULTAG   0x7ffc000100000000ul
#define BOOLTAG  0x7ffc000200000000ul
#define CHRTAG   0x7ffc000300000000ul
#define INTTAG   0x7ffc000400000000ul

#define NULVAL   0x7ffc000100000000ul
#define FLSVAL   0x7ffc000200000000ul
#define TRUVAL   0x7ffc000200000001ul

#define TAGMASK  0xffff000000000000ul
#define WTAGMASK 0xffffffff00000000ul

/* basic types recognized by the rascal virtual machine */
typedef UWord   Value;
typedef Void   *Pointer;
typedef ULong   Fixnum;
typedef Double  Real;
typedef Bool    Boolean;
typedef Char    Character;
typedef Int     Integer;

typedef struct Object
{
  union
  {
    struct Object *dtype;

    struct
    {
      Value       : 48;
      Value black :  1;
      Value gray  :  1;
      Value       : 14;
    };
  };

  Byte space[0];
} Object;

#define OBJECT Object obj;

/* tag predicates */
static inline Bool isReal( Value x ) { return (x&QNAN) != QNAN; }
static inline Bool isImm( Value x )  { return (x&TAGMASK) == IMMTAG; }
static inline Bool isPtr( Value x )  { return (x&TAGMASK) == PTRTAG; }
static inline Bool isFix( Value x )  { return (x&TAGMASK) == FIXTAG; }
static inline Bool isObj( Value x )  { return (x&TAGMASK) == OBJTAG; }
static inline Bool isChr( Value x )  { return (x&WTAGMASK) == CHRTAG; }
static inline Bool isInt( Value x )  { return (x&WTAGMASK) == INTTAG; }
static inline Bool isBool( Value x ) { return (x&WTAGMASK) == BOOLTAG; }
static inline Bool IsNul( Value x )  { return x == NULVAL; }

#endif
