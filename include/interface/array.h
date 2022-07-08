#ifndef rascal_interface_array_h
#define rascal_interface_array_h

#include "rascal.h"

enum arrfl_t {
  arrfl_shared  = 0x0080,
  arrfl_unboxed = 0x0040
};

#define ArrayType(_dtype)		       \
  {					       \
    object_t base;			       \
    uint_t len   : 24;			       \
    uint_t Ctype :  8;			       \
    uint_t cap   : 24;			       \
    uint_t flags :  8;			       \
    _dtype *data;			       \
  }

#define asarray( type, x ) ((type##_t*)ptr(x))
#define alen( type, x )    (asarray( type, x )->len)
#define actype( type, x )  (asarray( type, x )->Ctype)
#define acap( type, x )    (asarray( type, x )->cap)
#define aflags( type, x )  (asarray( type, x )->flags)
#define adata( type, x )   (asarray( type, x )->data)

struct vector_t   ArrayType(value_t);
struct tuple_t    ArrayType(value_t);
struct binary_t   ArrayType(uchar_t);
struct string_t   ArrayType(char_t);
struct bytecode_t ArrayType(ushort_t);

#define asvec( x )  asarray( vector, x )
#define vlen( x )   alen( vector, x )
#define vCtype( x ) actype( vector, x )
#define vcap( x )   acap( vector, x )
#define vflags( x ) aflags( vector, x )
#define vdata( x )  adata( vector, x )

#define asbin( x )  asarray( binary, x )
#define blen( x )   alen( binary, x )
#define bCtype( x ) actype( binary, x )
#define bcap( x )   acap( binary, x )
#define bflags( x ) aflags( binary, x )
#define bdata( x )  adata( binary, x )

#define astup( x )  asarray( tuple, x )
#define tlen( x )   alen( tuple, x )
#define tCtype( x ) actype( tuple, x )
#define tcap( x )   acap( tuple, x )
#define tflags( x ) aflags( tuple, x )
#define tdata( x )  adata( tuple, x )

#define asstr( x )  asarray( string, x )
#define slen( x )   alen( string, x )
#define sCtype( x ) actype( string, x )
#define scap( x )   acap( string, x )
#define sflags( x ) aflags( string, x )
#define sdata( x )  adata( string, x )

#define ascode( x ) asarray( bytecode, x )
#define clen( x )   alen( bytecode, x )
#define cCtype( x ) actype( bytecode, x )
#define ccap( x )   acap( bytecode, x )
#define cflags( x ) aflags( bytecode, x )
#define cdata( x )  adata( bytecode, x )

static void trace_array( value_t *vals, size_t limit ) {
  for (size_t i=0; i<limit; i++) {
    value_t val = vals[i];
    val         = relocate( val );
    vals[i]     = val;
  }
}

// utilities ------------------------------------------------------------------
object_t *construct_array( type_t t, flags_t f, size_t n );
int_t     init_array( type_t t, flags_t f, size_t n, value_t ini, void *spc );


size_t  array_size( value_t xa );

#endif
