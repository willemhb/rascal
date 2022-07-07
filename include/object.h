#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "rtypes.h"
#include "types.h"
#include "instructions.h"
#include "value.h"
#include "describe/safety.h"

#undef obtype
#undef obsize
#undef obflags

struct object_t {
  value_t type      :  8;
  value_t size      : 48;
  value_t flags     :  8;

  uchar_t space[0];
};

#define obtype(x)  (asob(x)->type)
#define obsize(x)  (asob(x)->size)
#define obflags(x) (asob(x)->flags)

enum obfl_t {
  obfl_printed  = 0x001,
  obfl_traced   = 0x002,
  obfl_protect  = 0x004,
  obfl_finalize = 0x008,

  // used only during allocation and initialization
  obfl_global   = 0x0100,
  obfl_unboxed  = 0x0200,
  obfl_sequence = 0x0400,
  obfl_array    = 0x0800,
  obfl_shared   = 0x1000
};

#define initob(x, t, sz, fl) (obhead(x) = (((value_t)(fl))<<56)|((size_t)sz<<8)|(t))

bool_t vflagp( value_t ob, uint_t fl);
bool_t fflagp( uint_t f1, uint_t f2 );
bool_t oflagp( object_t *ob, uint_t fl );
bool_t mflagp( uchar_t *m, uint_t fl );

#define flagp( x, f )							\
  _Generic((x),								\
	   value_t:vflagp,						\
	   uint_t:fflagp,						\
       	   uchar_t*:mflagp,						\
	   object_t*:oflagp)( x, f )

DeclareObFlagP( print );
DeclareObFlagP( trace );
DeclareObFlagP( protect );
DeclareObFlagP( finalize );

#define printp(x)    GenericObFlagP( x, print )
#define tracep(x)    GenericObFlagP( x, trace )
#define protectp(x)  GenericObFlagP( x, protect )
#define finalizep(x) GenericObFlagP( x, finalize )

object_t *construct( uint_t t, uint_t f, size_t c, size_t s );
void      untrace( value_t val );
void      finalize( object_t *ob );
value_t   trace( value_t val );

#endif
