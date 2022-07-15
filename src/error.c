#include <setjmp.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "error.h"

// internals ------------------------------------------------------------------
static void verror(const char *fname, const char *fmt, va_list va) {
  fprintf( stderr, "%s: error: ", fname );
  vfprintf( stderr, fmt, va );
  fputs( ".\n", stderr );
}

// exports --------------------------------------------------------------------
char *r_typename( type_t t ) {
  static char* names[type_pad] = {
    [type_cons]    = "cons",    [type_symbol]      = "symbol",
    [type_closure] = "closure", [type_environment] = "environment",
    [type_bytes]   = "bytes",   [type_vector]      = "vector",
    [type_integer] = "integer", [type_null]        = "null",
    [type_type]    = "type",    [type_boolean]     = "boolean"
  };

  return names[t];
}

char *r_fname( void ) {
  if ( Fp == 0 )
    return "<toplevel>";

  value_t fun = Stack[Sp-Argc];
  value_t sym = clname( fun );
  return sname( sym );
}

void r_error( const char *fname, const char *fmt, ... ) {
  va_list va;
  va_start(va, fmt);
  verror( fname, fmt, va );
  va_end(va);
  longjmp(Toplevel, 1);
}

void r_require( const char *fname, bool test, const char *fmt, ...) {
  if (!test) {
    va_list va;
    va_start(va, fmt);
    verror( fname, fmt, va );
    va_end(va);
    longjmp(Toplevel, 1);
  }
}

int r_argc( const char *fn, int got, int expect ) {
  static const char *fmt = "expected %d arguments to #, got %d";

  r_require( fn, got == expect, fmt, expect, got );
  return got;
}

int r_vargc( const char *fn, int got, int expect ) {
  static const char *fmt = "expected at least %d arguments to #, got %d";

  r_require( fn, got >= expect, fmt, expect, got );
  return got;
}

int r_oargc( const char *fn, int got, int n, ...) {
  va_list va;
  va_start(va, n);

  if (n == 1) {
    int expect = va_arg(va, int);
    va_end(va);
    return r_argc( fn, got, expect );
    
  } else if (n == 2) {
    int expect1 = va_arg(va, int);
    int expect2 = va_arg(va, int);
    va_end(va);

    char buf[512];
    sprintf( buf, "expected %d or %d arguments to #, got %d", expect1, expect2, got );
    r_require( fn, got == expect1 || got == expect2, buf );
    return got;
  }
  
  FILE *cbuf = tmpfile();

  assert( cbuf );

  int okayp = false, cbufsize = fprintf( cbuf, "expected" );

  for(int i=0; i<n; i++) {
    int oargc = va_arg(va, int);

    if ((okayp = (got == oargc)))
      goto cleanup;

    else if ( i+1 < n )
      cbufsize += fprintf( cbuf, " %d,", oargc );
      
    else
      cbufsize += fprintf( cbuf, " or %d arguments to #, got %d", oargc, got );
  }

 cleanup:{
    char msgbuf[cbufsize+1];
    va_end(va);
    fgets(msgbuf, cbufsize+1, cbuf );
    fclose( cbuf );
    r_require( fn, okayp, msgbuf );
    return got;
  }
}


type_t r_argt( const char *fn, value_t got, type_t expect ) {
  static const char *fmt = "# expected %s(), got %s()";

  r_require( fn,
	     tag( got ) == expect,
	     fmt,
	     r_typename( expect ),
	     r_typename( tag( got ) ) );

  return tag( got );
}

type_t r_oargt( const char *fn, value_t got, int n, ...) {
  va_list va;
  va_start(va, n);

  if (n == 1) {
    type_t expect = va_arg(va, type_t);
    va_end(va);
    return r_argt( fn, got, expect );
    
  } else if (n == 2) {
    type_t expect1 = va_arg(va, type_t);
    type_t expect2 = va_arg(va, type_t);
    type_t argt    = tag( got );
    
    va_end(va);

    char buf[512];
    sprintf( buf,
	     "# expected %s() or %s(), got %s()",
	     r_typename( expect1 ),
	     r_typename( expect2 ),
	     r_typename( argt ) );
    
    r_require( fn, got == expect1 || got == expect2, buf );
    return got;
  }

  type_t argt = tag( got );
  char *argtn = r_typename( argt );
  
  FILE *cbuf = tmpfile();

  assert( cbuf );

  int okayp = false, cbufsize = fprintf( cbuf, "# expected" );

  for(int i=0; i<n; i++) {
    type_t oargt = va_arg(va, type_t);

    if ((okayp = (got == oargt)))
      goto cleanup;

    else if ( i+1 < n )
      cbufsize += fprintf( cbuf, " %s(),", r_typename( oargt ) );
      
    else
      cbufsize += fprintf( cbuf, " or %s(), got %s()", r_typename( oargt ), argtn );
  }

 cleanup:{
    char msgbuf[cbufsize+1];
    va_end(va);
    fgets(msgbuf, cbufsize+1, cbuf );
    fclose( cbuf );
    r_require( fn, okayp, msgbuf );
    return argt;
  }
}
