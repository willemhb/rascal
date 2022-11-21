#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

/* tags used by the vm */
#define QNAN     0x7ffc000000000000ul
#define IMMTAG   0x7ffc000000000000ul
#define FIXTAG   0x7ffd000000000000ul
#define PTRTAG   0x7ffe000000000000ul
#define OBJTAG   0x7fff000000000000ul

#define NULTAG   0x7ffc000000000000ul
#define ASCIITAG 0x7ffc000100000000ul

#define REALTAG  0x0000000000000000ul

#define TAGMASK  0xffff000000000000ul
#define WTAGMASK 0xffffffff00000000ul
#define PTRMASK  0x0000fffffffffffful

#define rlNul NULTAG
#define rlEOS (ASCIITAG|((Value)EOF))

/* basic types recognized by the rascal virtual machine */
typedef UWord   Value;
typedef ULong   Fixnum;
typedef Double  Real;
typedef Void   *Pointer;
typedef Char    Ascii;

typedef struct Object
{
  struct Object *dtype;

  Byte space[0];
} Object;

typedef union RascalData
{
  Value     asValue;
  Fixnum    asFixnum;
  Real      asReal;
  Object   *asObject;
  Ascii     asAscii;
  Pointer   asPointer;
} RascalData;

#define OBJECT Object obj;

#define asValue( x )     (((RascalData)(x)).asValue)
#define asReal( x )      (((RascalData)(x)).asReal)
#define asFixnum( x )    (asValue(x)&PTRMASK)
#define asPointer( x )   ((Void*)asFixnum(x))
#define asAscii( x )     (((RascalData)(x)).asAscii)

#define tagValue( x, t ) (asValue(x)|(t))

#define asPtr( x )					\
  ((Void*)_Generic((x),					\
		   Value: (((Value)(x))&PTRMASK),	\
		   default: ((typeof(x))(x))))

#define asObject( x ) ((Object*)asPtr(x))

/* tag predicates */
#define TAG_PREDICATE(TYPE, MASK, TAG)					\
  static inline Bool is##TYPE( Value x ) { return (x&MASK) == TAG; }

TAG_PREDICATE(Imm, TAGMASK, IMMTAG)
TAG_PREDICATE(Obj, TAGMASK, OBJTAG)
TAG_PREDICATE(Fix, TAGMASK, FIXTAG)
TAG_PREDICATE(Ptr, TAGMASK, PTRTAG)
TAG_PREDICATE(Ascii, WTAGMASK, ASCIITAG)

/* other primitive predicates */
static inline Bool isNul( Value x )  { return x == rlNul; }
static inline Bool isReal( Value x ) { return (x&QNAN) != QNAN; }

/* other helpers */
static inline Value tagPtr( Void *ptr, Value tag ) { return (Value)ptr | tag; }

// forward declarations
typedef struct Type Type;

Type *rlTypeof( Value x );

#define valueIsType( x, T ) (rlTypeof(x)==(T))

#endif
