#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "runtime.h"
#include "object.h"
#include "lispio.h"

// utilities ------------------------------------------------------------------
// error signaling ------------------------------------------------------------
void error( value_t a, const char *n, const char *f, ...)
{
  va_list va;
  va_start(va, f);
  prin_error( a, n, f, va );
  va_end(va);
  longjmp( Toplevel, 1 );
}

void verror( value_t a, const char *n, const char *f, va_list va)
{
  prin_error( a, n, f, va );
  va_end( va );
  longjmp( Toplevel, 1 );
}

void prin_error( value_t a, const char *n, const char *f, va_list va)
{
  fprintf( stderr, "error: %s: ", n );
  vfprintf( stderr, f, va );
  if (a != NONE)
    lisp_prin( STDERR, a );

  fprintf( stderr, ".\n" );
}

// validation -----------------------------------------------------------------
void require( bool_t t, value_t a, const char *n, const char *f, ...)
{
  va_list va;
  va_start(va, f);

  if (!t)
    {
      prin_error( a, n, f, va );
      va_end( va );
      longjmp( Toplevel, 1 );
    }

  va_end(va);
}

size_t argco( size_t e, size_t g, bool_t v, const char *n )
{
  static const char *fmtf = "expected %zu arguments, got %zu";
  static const char *fmtv = "expected at least %zu arguments, got %zu";

  if (v)
    require( g >= e, NONE, n, fmtv, e, g );

  else
    require( e == g, NONE, n, fmtf, e, g );

  return g;
}

size_t argcos( size_t g, const char *n, size_t e, ...)
{
  static const char *fmt2 = "expected %zu or %zu arguments, got ";

  assert( e > 0);
  va_list va;
  va_start(va, e);

  if (e == 1)
    {
      size_t ex = va_arg(va, size_t);
      bool_t vx = va_arg(va, sint_t);

      va_end(va);
      return argco( ex, g, vx, n );
    }

  else if (e == 2)
    {
      size_t ex = va_arg(va, type_t);
      bool_t vx = va_arg(va, sint_t);
      size_t ey = va_arg(va, size_t);
      bool_t vy = va_arg(va, sint_t);
      va_end(va);

      bool_t valid = (vx ? g >= ex : g == ex)
	|| (vy ? g >= ey : g == ey);
      
      require( valid, integer(g), n, fmt2, ex, ey );
      return g;
    }

  else
    {
      FILE *tmp = tmpfile();
      assert( tmp );
      size_t nf = fprintf( tmp, "expected " );
      bool_t valid = false;

      for (size_t i=0; i<e; i++)
	{
	  size_t ex = va_arg(va, size_t);
	  bool_t vx = va_arg(va, sint_t);
	  valid = valid || (vx ? g >= ex : g == ex);

	  if (i == 1)
	    nf += fprintf( tmp, " or %zu", ex );

	  else
	    nf += fprintf( tmp, " %zu,", ex );
	}

      va_end(va);

      if (valid)
	  fclose(tmp);
      
      else
	{
	  nf += fprintf( tmp, "arguments, got " );
	  char fmtbuf[nf+1];
	  fgets( fmtbuf, nf+1, tmp );
	  fclose( tmp );
	  error( integer(g), n, fmtbuf );
	}

      return g;
    }
}

type_t argtype( value_t a, const char *n, type_t e)
{
  static const char *fmt = "expected a value of type %s, got ";
  type_t g = lisp_type(a);
  require( g == e, a, n, fmt, BuiltinNames[e] );
  return g;
}

type_t argtypes( value_t a, const char *n, size_t e, ...)
{
  static const char *fmt2 = "expected a value of type %s or %s, got ";

  assert( e > 0 );
  va_list va;
  va_start(va, e);

  if (e == 1)
    {
      type_t t = va_arg(va, type_t);
      va_end(va);
      return argtype(a, n, t );
    }

  else if (e == 2)
    {
      type_t tx = va_arg(va, type_t);
      type_t ty = va_arg(va, type_t);
      type_t ta = lisp_type( a );
      va_end(va);
      require( ta == tx || ta == ty, a, n, fmt2, BuiltinNames[tx], BuiltinNames[ty] );
      return ta;
    }

  else
    {
      FILE *tmp = tmpfile();
      assert( tmp );
      size_t nf = fprintf( tmp, "expected a value of type" );
      bool_t valid = false;
      type_t ta = lisp_type(a);

      for (size_t i=0; i<e; i++)
	{
	  type_t t = va_arg(va, type_t);
	  valid = valid || ta == t;

	  if (i == 1)
	    nf += fprintf( tmp, " or %s", BuiltinNames[t]);

	  else
	    nf += fprintf( tmp, " %s,", BuiltinNames[t] );
	}

      va_end(va);

      if (valid)
	  fclose(tmp);
      
      else
	{
	  nf += fprintf( tmp, "got " );
	  char fmtbuf[nf+1];
	  fgets( fmtbuf, nf+1, tmp );
	  fclose( tmp );
	  error( a, n, fmtbuf );
	}

      return ta;
    }
}

// predicates -----------------------------------------------------------------
// type predicates ------------------------------------------------------------
wt_pred(type, TYPE)
const_pred(none, false)
const_pred(any, true)
val_pred(nil, NIL)

wt_pred(boolean, BOOLEAN)
wt_pred(character, CHARACTER)

bool_t is_flonum( value_t x )
{
  return (x&QNAN) != QNAN;
}

wt_pred(integer, INTEGER)


bool_t is_builtin( value_t x )
{
  return is_function(x) && !!(x&7);
}


bool_t is_closure( value_t x )
{
  return is_function(x) && !(x&7);
}

ht_pred(pair, PAIR)
ht_pred(cons, CONS)

type_pred(vector, VECTOR)
type_pred(string, STRING)
type_pred(binary, BINARY)

ht_pred(symbol, SYMBOL)

wt_pred(port, PORT)
type_pred(environment, ENVIRONMENT)
ht_pred(pointer, POINTER)

// other tag predicates -------------------------------------------------------
ht_pred(immediate, IMMEDIATE)
ht_pred(function, FUNCTION)
ht_pred(object, OBJECT)

// union type predicates ------------------------------------------------------
bool_t is_list( value_t x )
{
  return is_nil(x) || is_cons(x);
}

bool_t is_cell( value_t x )
{
  return is_cons(x) || is_pair(x);
}

// symbol predicates ----------------------------------------------------------
flag_pred(gensym, symbol_t*, symbol, pval, gensym, true)
flag_pred(keyword, symbol_t*, symbol, pval, keyword, true)
flag_pred(bound, symbol_t*, symbol, pval, bound, true)
flag_pred(constant, symbol_t*, symbol, pval, constant, true)

// function predicates --------------------------------------------------------
bool_t is_macro(value_t x)
{
  return is_closure(x)
    && clo_head(x) == sym_mac;
}

flag_pred(vargs, closure_t*, closure, pval, vargs, true)
flag_pred(oargs, closure_t*, closure, pval, oargs, true)
flag_pred(compiled, closure_t*, closure, pval, compiled, true)
flag_pred(constructor, closure_t*, closure, pval, constructor, true)

// environment predicates -----------------------------------------------------


// misc predicates ------------------------------------------------------------
val_pred(true, TRUE)
val_pred(false, FALSE)
