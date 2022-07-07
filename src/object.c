#include "object.h"


inline bool_t vflagp( value_t ob, uint_t fl ) {
  return objectp( ob ) && !!(obflags(ob)&fl);
}

inline bool_t fflagp( uint_t f1, uint_t f2 ) {
  return !!(f1&f2);
}

inline bool_t oflagp( object_t *ob, uint_t fl ) {
  return ob && !!( obflags(ob)&fl );
}

inline bool_t mflagp( uchar_t *m, uint_t fl ) {
  return m && !!(*m & fl);
}

ObFlagP( print, obfl_printed, Auto )
ObFlagP( trace, obfl_traced, Auto )
ObFlagP( protect, obfl_protect, Auto )
ObFlagP( finalize, obfl_finalize, Auto )

inline object_t *construct( uint_t t, uint_t f, size_t c, size_t s ) {
  return Construct[t]( t, f, c, s );
}

void finalize( object_t *ob ) {
  if ( finalizep( ob ) )
    Finalize[obtype(ob)](ob);
}

void untrace( value_t val ) {
  if ( immediatep( val ) ) return;
  else if ( tracep( val ) ) {
    obflags( val ) &= ~obfl_traced;
    Trace[rtypeof(val)](val);
  }
}

value_t trace( value_t val ) {
  if ( immediatep( val ) )
    return val;

  else if ( movedp( val ) )
    return obslot( val, 1 );

  else if ( !globalp( val ) )
    Relocate[rtypeof(val)]( &val, &Free, &MapFree, &HeapUsed );
  
  else if ( tracep( val ) )
    return val;

  obflags( val ) |= obfl_traced;
  return Trace[rtypeof(val)]( val );
}
