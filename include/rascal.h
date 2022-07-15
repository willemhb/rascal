#ifndef rascal_h
#define rascal_h

#include <stdio.h>

#include "common.h"
#include "types.h"
#include "globals.h"

// utility macros -------------------------------------------------------------
#define car( x ) (((cons_t*)ptrval(x))->car)
#define cdr( x ) (((cons_t*)ptrval(x))->cdr)

#define sbind( x )  (((symbol_t*)ptrval(x))->bind)
#define shash( x )  (((symbol_t*)ptrval(x))->hash)
#define sidno( x )  (((symbol_t*)ptrval(x))->idno)
#define sname( x )  (&(((symbol_t*)ptrval(x))->name[0]))
#define slen( x )   (((symbol_t*)ptrval(x))->len)
#define sentry( x ) ((symbols_t*)(ptrval(x)-sizeof(cons_t)))

#define alen( x )  (_alen( x )[0])
#define adata( x ) _adata( x )
#define sdata( x ) ((char*)adata( x ))
#define vdata( x ) ((value_t*)adata( x ))
#define asize( x ) (((uint*)adata( x ))[-1])

#define vhead( x ) ((x)[-1])
#define shead( x ) (((value_t*)(x))[-1])

#define clform( x ) ( vdata( x )[0] )
#define clname( x ) ( vdata( x )[1] )
#define clenvt( x ) ( vdata( x )[2] )
#define clvals( x ) ( vdata( x )[3] )
#define clcode( x ) ( vdata( x )[4] )

#define envarg( x ) ( vdata( x )[0] )
#define envloc( x ) ( vdata( x )[1] )
#define envclo( x ) ( vdata( x )[2] )
#define envupv( x ) ( vdata( x ) + 3 )

#endif
