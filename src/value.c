#include <string.h>

#include "value.h"
#include "hashing.h"

// dispatchers ----------------------------------------------------------------
inline bool_t vimmediatep( value_t x   ) { return !x || tag(x)==tag_immediate; }
inline bool_t oimmediatep( object_t *o ) { return !o; }
inline bool_t timmediatep( type_t t ) { return !t || ((t&3) == tag_immediate); }

inline bool_t vobjectp( value_t   x ) { return x && tag(x) == tag_object; }
inline bool_t oobjectp( object_t *o ) { return o; }
inline bool_t tobjectp( type_t t )   { return t && ((t&3) == tag_object); }

inline bool_t vheaderp( value_t x ) { return objectp( x ) ? obtag( x ) == tag_header : tag( x ) == tag_header; }
inline bool_t oheaderp( object_t *o ) { return o ? false : obtag( o ) == tag_header; }

inline bool_t vmovedp( value_t x   ) { return obtag( x ) == tag_moved; }
inline bool_t omovedp( object_t *o ) { return obtag( o ) == tag_moved; }

inline type_t vtypeof( value_t   x ) { return objectp( x ) ? obtype( x ) : imtype( x ); }
inline type_t otypeof( object_t *o ) { return o ? obtype( o )  : type_null;  }

inline int_t rinit( type_t t, uint_t f,  size_t c, size_t s, value_t i, void *spc ) {
  return Init[t]( t, f, c, s, i, spc );
}

size_t vsizeof( value_t x ) {
  uint_t t = rtypeof( x );
  
  if ( Sizeof[t])
    return Sizeof[t]( x );
  
  else if ( immediatep( x ) )
    return TypeSizes[t];
  
  else
    return obsize( x );
}

hash_t rhash( value_t x ) {
  type_t t = rtypeof( x );

  if ( Hash[t] )
    return Hash[t](x);
  
  else if ( immediatep( t ) )
    return int64hash( x );

  else
    return pointerhash( ptr( x ) );
}

inline size_t vprint( FILE *ios, value_t x ) {
  type_t t = rtypeof( x );

  if ( Print[t] )
    return Print[t]( ios, x );
  
  char_t *fallback = rtypename( t );
  size_t  out      = strlen( fallback ) + 2;
  fprintf( ios, "<%s>", fallback );
  return out;
}

inline size_t oprint( FILE *ios, object_t *ob ) {
  return vprint( ios, (value_t)ob );
}

inline char_t *vtypename( value_t x )   { return TypeNames[rtypeof(x)]; }
inline char_t *otypename( object_t *o ) { return TypeNames[rtypeof(o)]; }
inline char_t *ttypename( type_t t )    { return TypeNames[t]; }

inline bool_t visap( value_t x,   type_t t ) { return TypeMembers[t][rtypeof(x)]; }
inline bool_t oisap( object_t *o, type_t t ) { return TypeMembers[t][rtypeof(o)]; }
inline bool_t tisap( type_t x,    type_t t ) { return TypeMembers[x][t]; }

inline bool_t pglobalp( uchar_t *p ) {
  return Collecting ?
    (p >= Reserve && p < (Reserve + ReserveSize)) :
    (p >= Heap && p < (Heap + HeapSize));
}

inline bool_t oglobalp( object_t *o ) { return o ? pglobalp( (uchar_t*)o ) : false; }

inline bool_t vglobalp( value_t x ) { return immediatep( x ) ? false : pglobalp( ptr( x ) ); }
