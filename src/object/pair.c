#include <assert.h>
#include <string.h>

#include "object/pair.h"
#include "object.h"
#include "runtime/stack.h"
#include "runtime/memory.h"
#include "describe/safety.h"
#include "memutils.h"

static const size_t psize  = sizeof(pair_t);
static const size_t pnword = 4;

inline bool_t pairp( value_t x ) { return isap( x, type_pair ); }
inline bool_t listp( value_t x ) { return isap( x, type_list ); }
inline bool_t asscp( value_t x ) { return isap( x, type_assc ); }

// internal methods -----------------------------------------------------------
value_t   trace_pair( value_t xp ) {
  value_t out = xp, node = out;
  
  while ( pairp( node ) ) {
    value_t ca = car( node );
    
    ca           = trace( ca );
    car( node )  = ca;
    node         = cdr( node );
  }

  return out;
}

void relocate_pair( value_t *val, uchar_t **space, uchar_t **map, size_t *used ) {
  size_t n  = plen( *val );

  uchar_t *oldspace, *oldmap,
          *newspace = *space,
          *newmap   = *map;

  *space += n * psize;
  *map   += n;
  *used  += n;

  value_t curr = *val;

  while ( pairp( curr ) ) {
    oldspace = ptr( curr ),
      oldmap = findmap( ptr( curr ) );
    
    memcpy( newspace, oldspace, psize );
    memcpy( newmap, oldmap, pnword );

    // set new cdr to point at next word
    obhead( curr ) &= ~1ul;

    // set the forwarding pointer
    car( curr ) = (value_t)newspace;
    
    curr        = cdr( curr );

    // clean out the old memory
    memset( oldmap, 0, pnword );
    memset( &cdr( oldspace ), 0, psize / 2);

    // point newspace and newmap 
    newspace += psize;
    newmap   += pnword;
  }
  
}

void      untrace_pair( value_t xv );
void      print_pair( FILE *ios, value_t xv );

value_t   mk_entry( value_t key, value_t bind, ulong_t hash );
value_t   mk_cons(value_t ca, value_t cd );
value_t   mk_list( value_t *base, size_t n );

// accessors ------------------------------------------------------------------
value_t pair_xar( value_t xp, value_t xc );
value_t pair_xdr( value_t xp, value_t xc );

// sequence methods -----------------------------------------------------------
value_t pair_ref( value_t xp, value_t key );
value_t pair_xef( value_t xp, value_t key, value_t bind );
size_t  pair_len( value_t xp );
bool_t  pair_hasp( value_t xp );

// initialization -------------------------------------------------------------
void    pair_init( void ) {
  // abstract pair types
  TypeNames[type_list]                = "list";
  TypeMembers[type_list][type_pair]   = true;
  TypeMembers[type_list][type_null]   = true;

  TypeNames[type_null]                = "null";
  Print[type_null]                    = print_pair;
  
  TypeNames[type_pair]                = "pair";
  TypeSizes[type_pair]                = sizeof(pair_t);
  TypeFlags[type_pair]                = 0;
  Trace[type_pair]                    = trace_pair;
  Relocate[type_pair]                 = relocate_pair;
  Untrace[type_pair]                  = untrace_pair;
  Print[type_pair]                    = print_pair;
  Finalize[type_pair]                 = NULL;
}
