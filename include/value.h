#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"
#include "rtypes.h"
#include "types.h"
#include "instructions.h"
#include "globals.h"

// utility macros -------------------------------------------------------------

#define rnull   ((value_t)type_null)
#define rnone   (((value_t)0xffffffff00000000ul)|type_none)
#define rtrue   (((value_t)0x0000000100000000ul)|type_boolean)
#define rfalse  ((value_t)type_boolean)

#define tag(x)         ((x)&3)
#define wtag(x)        ((x)&255)
#define tagv(x,t)      (((value_t)(x))|(t))
#define ptr(x)         ((void_t*)(((value_t)(x))&~3ul))

#define asob(x)        ((object_t*)ptr(x))
#define obslot(x,n)    (((value_t*)ptr(x))[(n)])
#define obhead(x)      (*((value_t*)ptr(x)))
#define obtype(x)      (obhead(x)&255)
#define obtag(x)       (obhead(x)&3)
#define obsize(x)      ((obhead(x)>>8)&0x00fffffffffffful)
#define obflags(x)     (obhead(x)>>56)
#define imtype(x)      (wtag(x))

#define nullp(x)      ((x)==rnull)
#define nonep(x)      ((x)==rnone)
#define truep(x)      ((x)==rtrue)
#define falsep(x)     ((x)==rfalse)

// forward declarations -------------------------------------------------------
bool_t vimmediatep( value_t x );
bool_t oimmediatep( object_t *o );
bool_t timmediatep( type_t xt );

bool_t vobjectp( value_t   x );
bool_t oobjectp( object_t *o );
bool_t tobjectp( type_t xt );

bool_t vheaderp( value_t x );
bool_t oheaderp( object_t *o );

bool_t vmovedp( value_t x   );
bool_t omovedp( object_t *o );

type_t vtypeof( value_t   x );
type_t otypeof( object_t *o );

int_t rinit( type_t t, uint_t f,  size_t c, size_t s, value_t i, void *spc );

size_t vsizeof( value_t x );
size_t osizeof( object_t *ob );

hash_t rhash( value_t x );

size_t vprint( FILE *ios, value_t x    );
size_t oprint( FILE *ios, object_t *ob );

char_t *vtypename( value_t x );
char_t *otypename( object_t *o );
char_t *ttypename( type_t t );

bool_t visap( value_t x,   type_t t );
bool_t oisap( object_t *o, type_t t );
bool_t tisap( type_t x,    type_t t );

bool_t pglobalp( uchar_t *p );
bool_t oglobalp( object_t *o );
bool_t vglobalp( value_t x );

// utilities ------------------------------------------------------------------
static inline int_t ival(value_t x) {
  return ((int_t)(x>>32));
}

static inline ulong_t uval(value_t x) {
  return x>>8;
}

// dispatchers ----------------------------------------------------------------
#define immediatep( x ) \
  _Generic((x),							\
	   value_t:vimmediatep,					\
	   type_t:timmediatep,					\
	   object_t*:oimmediatep)( x )

#define objectp( x )						\
  _Generic((x),							\
	   value_t:vobjectp,					\
       	   type_t:tobjectp,					\
	   object_t*:oobjectp)( x )

#define headerp( x )    _Generic((x), value_t:vheaderp, object_t*:oheaderp)( x )
#define movedp( x )     _Generic((x), value_t:vmovedp, object_t*:omovedp)( x )
#define rtypeof(x)      _Generic((x), value_t:vtypeof, object_t*:otypeof)( x )
#define rsizeof( x )    _Generic((x), value_t:vsizeof, object_t*:osizeof)( x )
#define rprint( x )     _Generic((x), value_t:vprint, object_t*:oprint)( x )

#define rtypename( x )				\
  _Generic((x),					\
	   value_t:vtypename,			\
	   object_t*:otypename,			\
	   type_t:ttypename)(x)

#define isap(x, t)				\
  _Generic((x),					\
       	   value_t:visap,			\
           object_t*:oisap,			\
       	   type_t:tisap				\
	   )(x,t)

#define globalp(x)					\
  _Generic((x),						\
	   value_t:vglobalp,				\
	   object_t*:oglobalp,				\
	   default:pglobalp)(x)


#endif
