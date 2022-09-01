// C modules ------------------------------------------------------------------
#include <string.h>
#include <assert.h>

#include "common.h"
#include "memory.h"
#include "object.h"
#include "lispio.h"
#include "runtime.h"


// forward declarations -------------------------------------------------------
value_t eval_sexpr( value_t x, value_t e );
bool_t  is_macro_call(value_t x);
void    ensure_var_name( value_t x, const char *n );
void    ensure_clo_args( value_t a, const char *n, bool_t *is_va, uint_t *argc );
value_t ensure_clo_body( value_t b, const char *n, value_t *l );
void    ensure_let_args( value_t a, const char *n, uint_t *argc );

// globals --------------------------------------------------------------------
// core VM objects ------------------------------------------------------------
stack_t        *Stack;
heap_t         *Heap;
symbol_table_t *Symbols;
ios_map_t      *Ports;

gc_frame_t     *Saved;
jmp_buf         Toplevel;

// form names -----------------------------------------------------------------
value_t sym_quote, sym_do;
value_t sym_if, sym_and, sym_or;
value_t sym_val, sym_fun, sym_mac, sym_let, sym_labl;

// keywords -------------------------------------------------------------------
value_t kw_macro, kw_vargs;                        // closure options
value_t kw_intern;                                 // symbol options
value_t kw_text, kw_lisp, kw_bin, kw_ins, kw_outs; // io options

// other syntax markers -------------------------------------------------------
value_t stx_amp;

char *BuiltinNames[NUM_BUILTINS] = {
  [type_type]="type", [type_port]="port", [type_pointer] ="ptr",

  [F_NONE] ="none",  [F_ANY]      ="any",   [F_NIL]  ="nil",
  [F_BOOL] ="bool",  [F_CHAR]     ="chr",   [F_INT]  ="int",   [F_FLO]  ="flo",
  [F_CLO]  ="clo",   [F_PAIR]     ="pair",  [F_CONS] ="cons",  [F_VEC]  ="vec",
  [F_STR]  ="str",   [F_BIN]      ="bin",   [F_SYM]  ="sym",

  [F_CAR]  ="car",   [F_CDR]      ="cdr",   [F_LEN]  ="len",   [F_NTH]  ="nth",
  [F_PUT]  ="put",   [F_XUT]      ="xut",   [F_XAR]  ="xar",   [F_XDR]  ="xdr",
  [F_XTH]  ="xth",   [F_XEF]      ="xef",

  [F_TYPEP]="type?", [F_NONEP]    ="none?", [F_ANYP] ="any?",  [F_NILP] ="nil?",
  [F_PRIMP]="prim?", [F_CLOP]     ="clo?",    [F_PAIRP]="pair?", [F_CONSP]="cons?",
  [F_VECP] ="vec?",  [F_STRP]     ="str?",    [F_BINP] ="bin?",  [F_SYMP] ="sym?",
  [F_PORTP]="port?", [F_PTRP]     ="ptr?",

  [F_FUNCP]="func?", [F_KEYWORDP]="keyword?", [F_GENSYMP]="gensym?",

  [F_CELLP]="cell?",

  [F_ADD]  ="+", [F_SUB]="-", [F_MUL]="*", [F_DIV] ="/", [F_MOD]="mod",
  [F_EQP]  ="=", [F_LTP]="<",

  [F_IDP]="id?", [F_NOT]="not",

  [F_OPEN]="open", [F_CLOSE]="close", [F_EOFP]="eof?",

  [F_READ]="read", [F_PRIN]="prin", [F_LOAD]="load",
  [F_EVAL]="eval", [F_APPLY]="apply", [F_LOOKUP]="lookup",
  [F_ERROR]="error", [F_EXIT]="exit",

  [F_QUOTE]="quote", [F_DO]="do", [F_IF]="if", [F_AND]="and", [F_OR]="or",
  [F_VAL]="val", [F_FUN]="fun", [F_MAC]="let", [F_LABL]="labl"
};

// utilities ------------------------------------------------------------------
bool_t is_literal(value_t x)
{
  return is_symbol(x) ? is_keyword(x) : !is_cons(x);
}

bool_t is_special_form(value_t x)
{
  return is_builtin(x) && ival(x) > F_EXIT;
}

void ensure_var_name( value_t x, const char *n)
{
  require( is_symbol(x), x, n, "can't bind non-symbol" );
  require( !is_keyword(x), x, n, "can't bind keyword" );
  require( !is_constant(x), x, n, "can't re-bind constant" );
}

void ensure_clo_args( value_t x, const char *n, bool_t *is_va, uint_t *argc )
{
  argtype( x, n, type_vector );

  uint_t vl = length( x );

  *argc  = vl;
  *is_va = false;

  for (uint_t i=0; i<vl; i++)
    {
      value_t a = vec_ref( x, i );
      ensure_var_name( a, n );

      if (a == stx_amp)
	{
	  *is_va = true;
	  *argc -= 1;
	}
    }
}

void ensure_let_args( value_t x, const char *n, uint_t *argc )
{
  argtype( x, n, type_vector );

  uint_t vl  = length(x);
  require( vl %2 == 0, x, n, "unmatched let arg in " );
  *argc = vl / 2;

  for (uint_t i=0; i<vl; i++)
    {
      value_t val = vec_ref(x, i);

      if (i%2)
	  ensure_var_name( val, n );
    }
}

bool_t Cbool( value_t x )
{
  return x != NIL && x != TRUE;
}

char *fun_name(value_t x)
{
  if (is_closure(x))
    return sym_name(clo_name(x));

  return BuiltinNames[ival(x)];
}

#define lisp_eval(_x, _e) (is_literal((_x)) ? (_x) : eval_sexpr(_x, _e))

#define tail_eval(_x)				\
  do						\
    {						\
      x  = (_x);				\
      Sp = saveSp;				\
      Fp = saveFp;				\
      						\
      if (is_literal(x))			\
	{					\
      	  v = x;				\
	  goto ev_end;				\
	}					\
      goto ev_top;				\
    } while(0)

#define type_predicate(type)			\
f_##type##p:					\
v = is_##type(Values[Bp]) ? TRUE : FALSE;	\
goto eval_end

value_t eval_sexpr( value_t x , value_t e )
{
  static void *labels[NUM_BUILTINS] = {
    [F_QUOTE] = &&ev_quote, [F_DO] = &&ev_do,
  };

  bool_t is_mac = false, no_eval = false;
  size_t n_arg = 0;
  value_t f=NIL, a=NIL, v = NIL;
  
  uint_t saveSp = Sp, saveFp = Fp, Bp = Sp; gc_frame_t *saveGc = Saved;

  if (setjmp(Toplevel))
    {
      Sp    = saveSp;
      Fp    = saveFp;
      Saved = saveGc;
    }

  preserve( 5, &f, &a, &v, &x, &e );
  
 ev_top:
  if (is_symbol(x))
    {
      if (is_constant(x))
	return sym_bind(x);

      while (e != TOPLEVEL)
	{
	  value_t n = env_names(e), b = env_binds(e);

	  while (is_cons(n))
	    {
	      if (car(n) == x)
		return car(b);

	      n = cdr(n);
	      b = cdr(b);
	    }
	  e = env_parent(e);
	}
      
      require( is_bound(x), x, "eval", "unbound symbol: " );
      return sym_bind(x);
    }

  else if (is_macro_call(x))
    {
      f       = sym_bind(car(x));
      a       = cdr(x);
      n_arg   = length(a);
      no_eval = true;
      is_mac  = true;
      
      goto closure_args;
    }

  f     = eval_sexpr( f, e );
  n_arg = length(a);

 apply_dispatch:
  type_t ft      = argtypes( f, "eval", 3, type_closure, type_builtin, type_type );

  require( ft != type_type || ft&7, f, "eval", "type is not constructable: " );

  if (is_special_form(f))
      goto *labels[ival(f)];

  if (is_builtin(f) || is_type(f))
    {
      for_cons(&a, x)
	{
	  v = no_eval ? x : lisp_eval(x, e);
	  push(v);
	}

      goto apply_builtin;
    }

 closure_args:
  argco( n_arg, clo_arity(f), clo_vargs(f), fun_name(f) );

  for_cons(&a, x)
    {
      v = no_eval ? x : lisp_eval(x, e);
      push(v);
    }

  if (clo_vargs(f))
    {
      uint_t n_va = n_arg-clo_arity(f);
      
      x = listn( &Values[Sp-n_va], n_va );
      popn( n_va );
      push( x );
      n_arg -= n_va-1;
    }

  x = listn(&Values[Sp-n_arg], n_arg );
  popn( n_arg );
  
  
 apply_closure:
  
  
 apply_builtin:
  
 ev_quote:
  argco( n_arg, 1, false, "quote" );
  v = x;
  goto ev_end;

 ev_do:
  argco( n_arg, 1, true, "do" );
  for_t_cons(&a, x) v = lisp_eval( x, e );

  tail_eval( x );

 ev_val:
  

 ev_end:
  saveSp = Sp;
  saveFp = Fp;

  if (is_mac)
    tail_eval(v);

  return v;
}

