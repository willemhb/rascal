#include <stdlib.h>

#include "native.h"
#include "interp.h"
#include "runtime.h"
#include "lang.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t AsciiKeyword, BytesKeyword;

// internal api +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static value_t c_to_rl_bool( bool b ) {
  return b ? True : False;
}

static bool rl_to_c_bool( value_t x ) {
  return x != NIL && x != False;
}

// native function declarations +++++++++++++++++++++++++++++++++++++++++++++++
#define NATIVE_FN( name ) value_t native_##name( usize n, value_t* args )

// comparison -----------------------------------------------------------------
NATIVE_FN(idp);
NATIVE_FN(eqp);
NATIVE_FN(compare);
NATIVE_FN(hash);

// arithmetic -----------------------------------------------------------------
NATIVE_FN(add);
NATIVE_FN(sub);
NATIVE_FN(mul);
NATIVE_FN(div);
NATIVE_FN(mod);
NATIVE_FN(eql);
NATIVE_FN(ltp);

// type predicates ------------------------------------------------------------
NATIVE_FN(numberp);
NATIVE_FN(glyphp);
NATIVE_FN(unitp);
NATIVE_FN(portp);
NATIVE_FN(nativep);
NATIVE_FN(pointerp);
NATIVE_FN(symbolp);
NATIVE_FN(listp);
NATIVE_FN(alistp);
NATIVE_FN(tablep);
NATIVE_FN(bufferp);
NATIVE_FN(chunkp);
NATIVE_FN(closurep);
NATIVE_FN(controlp);
NATIVE_FN(integerp);
NATIVE_FN(consp);
NATIVE_FN(stringp);

// constructors ---------------------------------------------------------------
NATIVE_FN(number);
NATIVE_FN(glyph);
NATIVE_FN(unit);
NATIVE_FN(list);
NATIVE_FN(cons);
NATIVE_FN(symbol);
NATIVE_FN(gensym);
NATIVE_FN(alist);
NATIVE_FN(table);
NATIVE_FN(buffer);
NATIVE_FN(string);
NATIVE_FN(any);
NATIVE_FN(none);

// collection generics --------------------------------------------------------
NATIVE_FN(len);
NATIVE_FN(emptyp);
NATIVE_FN(get);
NATIVE_FN(set);
NATIVE_FN(add);
NATIVE_FN(pop);
NATIVE_FN(hasp);
NATIVE_FN(map);
NATIVE_FN(filter);
NATIVE_FN(reduce);
NATIVE_FN(foreach);

// accessors/mutators ---------------------------------------------------------
NATIVE_FN(head);
NATIVE_FN(tail);

// interpreter ----------------------------------------------------------------
NATIVE_FN(eval);
NATIVE_FN(apply);
NATIVE_FN(compile);
NATIVE_FN(exec);
NATIVE_FN(read);
NATIVE_FN(print);

// runtime --------------------------------------------------------------------
NATIVE_FN(exit);
NATIVE_FN(error);

// misc -----------------------------------------------------------------------
NATIVE_FN(not);

// native function implementations ++++++++++++++++++++++++++++++++++++++++++++
// comparison -----------------------------------------------------------------
value_t native_idp( usize n, value_t* args ) {
  argco("id?", false, n, 2);
  return args[0] == args[1] ? True : NIL;
}

value_t native_eqp( usize n, value_t* args ) {
  argco("eq?", false, n, 2);
  return equal(args[0], args[1]) ? True : NIL;
}

value_t native_compare( usize n, value_t* args ) {
  argco("compare", false, n, 2);
  return number(compare(args[0], args[1]));
}

value_t native_hash( usize n, value_t* args ) {
  argco("hash", false, n, 1);
  return number(hash(args[0]));
}

// arithmetic -----------------------------------------------------------------
value_t native_add( usize n, value_t* args ) {
  argco("+", true, n, 1);
  value_t out;
  
  if ( n == 1 ) {
    argtype("+", args[0], NUMBER);
    out = args[0];
  } else {
    number_t accum = to_number("+", args[0]);

    for ( usize i=1; i<n; i++ )
      accum += to_number("+", args[i]);

    out = number(accum);
  }

  return out;
}

value_t native_sub( usize n, value_t* args ) {
  argco("-", true, n, 1);
  value_t out;

  if ( n == 1 ) {
    argtype("-", args[0], NUMBER);
    out = number(-as_number(args[0]));
  } else {
    number_t accum = to_number("-", args[0]);

    for ( usize i=1; i<n; i++ )
      accum -= to_number("-", args[i]);

    out = number(accum);
  }

  return out;
}

value_t native_mul( usize n, value_t* args ) {
  argco("*", true, n, 2);

  number_t accum = to_number("*", args[0]);

  for ( usize i=1; i<n && accum != 0; i++ )
    accum *= to_number("*", args[i]);

  return number(accum);
}

value_t native_div( usize n, value_t* args ) {
  argco("/", true, n, 1);

  value_t out;

  if ( n == 1 ) {
    number_t denom = to_number("/", args[0]);
    require("/", denom != 0, args[0], "division by zero");
    out = number(1/denom);
  } else {
    number_t accum = to_number("/", args[0]);
    
    for ( usize i=1; i<n && accum != 0; i++ ) {
      number_t divisor = to_number("/", args[i]);
      require("/", divisor != 0, args[i], "division by zero");
      accum /= divisor;
    }
    out = number(accum);
  }

  return out;
}

value_t native_mod( usize n, value_t* args ) {
  argco("mod", false, n, 2);
  long numer = to_integer("mod", args[0]);
  long denom = to_integer("mod", args[1]);
  require("mod", denom != 0, args[1], "division by zero");
  return number(numer % denom);
}

value_t native_eql( usize n, value_t* args ) {
  argco("=", true, n, 2);
  argtype("=", args[0], NUMBER);

  value_t out = True;

  for ( usize i=1; out != False && i<n; i++ ) {
    argtype("=", args[i], NUMBER);

    if ( args[i] != args[i-1] )
      out = False;
  }

  return out;
}

value_t native_ltp( usize n, value_t* args ) {
  argco("<", true, n, 2);
  argtype("<", args[0], NUMBER);
  value_t out = True;

  for ( usize i=1; out != False && i<n; i++ ) {
    argtype("<", args[i], NUMBER);

    if ( as_number(args[i-1]) >= as_number(args[i]) )
      out = False;
  }

  return out;
}

// type predicates ------------------------------------------------------------
#define NATIVE_TYPEP( type )                            \
  value_t native_##type##p( usize n, value_t* args )    \
  {                                                     \
    argco(#type"?", false, n, 1);                       \
    return c_to_rl_bool(is_##type(args[0]));            \
  }

NATIVE_TYPEP(number);
NATIVE_TYPEP(glyph);
NATIVE_TYPEP(unit);
NATIVE_TYPEP(port);
NATIVE_TYPEP(native);
NATIVE_TYPEP(pointer);
NATIVE_TYPEP(symbol);
NATIVE_TYPEP(list);
NATIVE_TYPEP(alist);
NATIVE_TYPEP(table);
NATIVE_TYPEP(buffer);
NATIVE_TYPEP(chunk);
NATIVE_TYPEP(closure);
NATIVE_TYPEP(control);
NATIVE_TYPEP(integer);
NATIVE_TYPEP(cons);
NATIVE_TYPEP(string);

#undef NATIVE_TYPEP

value_t native_keywordp( usize n, value_t* args ) {
  argco("keyword?", false, n, 1);
  return c_to_rl_bool(is_symbol(args[0]) && hasfl(args[0], LITERAL));
}

// constructors -------------------------------------------------------------
value_t native_list( usize n, value_t* args ) {
  list_t* out = mk_list(n, args);
  return object(out);
}

value_t native_cons( usize n, value_t* args ) {
  argco("cons", false, 2, n);
  argtype("cons", args[1], LIST);
  list_t* out = list(args[0], as_list(args[1]));
  return object(out);
}

value_t native_symbol( usize n, value_t* args ) {
  argco("symbol", false, 1, n);

  value_t out = NIL;

  if ( is_symbol(args[0]) )
    out = args[0];

  else if ( is_string(args[0]) )
    out = object(symbol(as_string(args[0])->data));

  else
    error("symbol", args[0], "argument not a symbol or string");

  return out;
}

value_t native_gensym( usize n, value_t* args ) {
  argcos("gensym", n, 2, 0, 1);

  char* name = "";

  if ( n == 0 )
    name = "symbol";

  else if ( is_symbol(args[0]) )
    name = as_symbol(args[0])->name;

  else if ( is_string(args[0]) )
    name = as_buffer(args[0])->data;

  else
    error("gensym", args[0], "argument not a symbol or string");

  value_t out = object(gensym(name));

  return out;
}

value_t native_alist( usize n, value_t* args ) {
  alist_t* al = mk_alist(n, args);
  return object(al);
}

value_t native_table( usize n, value_t* args ) {
  require("table", n % 2 == 0, number(n), "unpaired argument");
  table_t* t = mk_table(n, args);
  return object(t);
}

// collection generics --------------------------------------------------------
value_t native_len( usize n, value_t* args ) {
  argco("len", false, n, 1);

  usize out = 0;

  switch ( type_of(args[0]) ) {
    case LIST:   out = as_list(args[0])->arity; break;
    case ALIST:  out = as_alist(args[0])->cnt; break;
    case TABLE:  out = as_table(args[0])->cnt; break;
    case BUFFER: out = as_buffer(args[0])->cnt; break;
    default:
      error("len", args[0], "type error: not a sequence");
      break;
  }

  return number(out);
}

// accessors/mutators -------------------------------------------------------
value_t native_head( usize n, value_t* args ) {
  argco("head", false, n, 1);
  return to_cons("head", args[0])->head;
}

value_t native_tail( usize n, value_t* args ) {
  argco("tail", false, n, 1);
  return object(to_cons("tail", args[0])->tail);
}

// interpreter --------------------------------------------------------------
value_t native_eval( usize n, value_t* args ) {
  argco("eval", false, n, 1);
  return eval(args[0]);
}

value_t native_read( usize n, value_t* args ) {
  argcos("read", n, 2, 0, 1);
  port_t src = n == 1 ? to_port("read", args[0]) : stdin;
  value_t out = read(src);
  return out;
}

value_t native_print( usize n, value_t* args ) {
  argcos("print", n, 2, 1, 2);
  port_t ios = n == 2 ? to_port("print", args[0]) : stdout;
  value_t expr = n == 2 ? args[1] : args[0];
  print(ios, expr);
  return expr;
}

// runtime ------------------------------------------------------------------
value_t native_exit( usize n, value_t* args ) {
  argcos("exit", n, 2, 0, 1);

  if ( n == 0 )
    exit(0);

  else {
    long status = to_integer("exit", args[0]);
    exit(status);
  }

  return NIL;
}

value_t native_error( usize n, value_t* args ) {
  argco("error", false, n, 2);
  require("error", is_string(args[1]), args[1], "error message must be a string");
  error("exec", args[0], as_buffer(args[1])->data);
  return NIL;
}

// misc ---------------------------------------------------------------------
value_t native_not( usize n, value_t* args ) {
  argco("not", false, n, 1);
  return c_to_rl_bool(!rl_to_c_bool(args[0]));
}

// toplevel initialization ++++++++++++++++++++++++++++++++++++++++++++++++++++
static void define_native( const char* fname, native_t fun ) {
  toplevel_define(fname, native(fun));
}

void toplevel_init_interp( void ) {
  // create keywords ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  AsciiKeyword = object(symbol(":ascii"));
  BytesKeyword = object(symbol(":bytes"));
  
  // create native functions ++++++++++++++++++++++++++++++++++++++++++++++++++
  // comparison ---------------------------------------------------------------
  define_native("id?", native_idp);
  define_native("eq?", native_eqp);
  define_native("compare", native_compare);
  define_native("hash", native_hash);

  // arithmetic ---------------------------------------------------------------
  define_native("+", native_add);
  define_native("-", native_sub);
  define_native("*", native_mul);
  define_native("/", native_div);
  define_native("mod", native_mod);
  define_native("=", native_eql);
  define_native("<", native_ltp);

  // type predicates ----------------------------------------------------------
  define_native("number?", native_numberp);
  define_native("glyph?", native_glyphp);
  define_native("unit?", native_unitp);
  define_native("port?", native_portp);
  define_native("native?", native_nativep);
  define_native("pointer?", native_pointerp);
  define_native("symbol?", native_symbolp);
  define_native("list?", native_listp);
  define_native("alist?", native_alistp);
  define_native("table?", native_tablep);
  define_native("buffer?", native_bufferp);
  define_native("chunk?", native_chunkp);
  define_native("closure?", native_closurep);
  define_native("integer?", native_integerp);
  define_native("cons?", native_consp);
  define_native("keyword?", native_keywordp);
  define_native("string?", native_stringp);

  // constructors -------------------------------------------------------------
  define_native("number", native_number);
  define_native("glyph", native_glyph);
  define_native("list", native_list);
  define_native("cons", native_cons);
  define_native("symbol", native_symbol);
  define_native("gensym", native_gensym);
  define_native("alist", native_alist);
  define_native("table", native_table);
  define_native("buffer", native_buffer);
  define_native("string", native_string);

  // collection generics ------------------------------------------------------
  define_native("len", native_len);
  define_native("empty?", native_emptyp);
  define_native("get", native_get);
  define_native("set", native_set);
  define_native("add", native_add);
  define_native("pop", native_pop);
  define_native("has?", native_hasp);
  define_native("map", native_map);
  define_native("filter", native_filter);
  define_native("reduce", native_reduce);
  define_native("for-each", native_foreach);

  // accessors/mutators -------------------------------------------------------
  define_native("head", native_head);
  define_native("tail", native_tail);

  // interpreter --------------------------------------------------------------
  define_native("eval", native_eval);
  define_native("apply", native_apply);
  define_native("compile", native_compile);
  define_native("exec", native_exec);
  define_native("read", native_read);
  define_native("print", native_print);

  // runtime & os -------------------------------------------------------------
  define_native("exit", native_exit);
  define_native("error", native_error);

  // misc ---------------------------------------------------------------------
  define_native("not", native_not);
}
