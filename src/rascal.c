// C modules ------------------------------------------------------------------
#include <string.h>
#include <assert.h>

#include "common.h"
#include "memory.h"
#include "lispio.h"

// pseudo-globals ------------------------------------------------------------
#define Sp     (Stack->sp)
#define Fp     (Stack->fp)
#define Values (Stack->data)
#define Tos    (Stack->data[Stack->sp-1])

// safecasts ------------------------------------------------------------------
cons_t   *tocons(value_t x, const char *fname);
symbol_t *tosymbol(value_t x, const char *fname);

bool_t is_vargs(value_t x);
bool_t is_oargs(value_t x);

value_t evalsx(value_t x, value_t e);
value_t lookup(value_t x, value_t e);

value_t vec_ref(value_t vec, uint_t n);
value_t vec_put(value_t vec, value_t val);

size_t  sxp_len(value_t x);

#define eval(x, e) (is_literal(x) ? (x) : evalsx((x), (e)))

// globals --------------------------------------------------------------------
stack_t        *Stack;
heap_t         *Heap;
symbol_table_t *Symbols;
ios_map_t      *Ports;

gc_frame_t     *Saved;
jmp_buf         Toplevel;

char *builtinnames[] = {
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
  [F_EQP] ="=", [F_LTP]="<",

  [F_IDP]="id?", [F_NOT]="not",

  [F_OPEN]="open", [F_CLOSE]="close", [F_EOFP]="eof?",

  [F_READ]="read", [F_PRIN]="prin", [F_LOAD]="load",
  [F_EVAL]="eval", [F_APPLY]="apply", [F_LOOKUP]="lookup",
  [F_ERROR]="error", [F_EXIT]="exit",

  [F_QUOTE]="quote", [F_DO]="do", [F_IF]="if", [F_AND]="and", [F_OR]="or",
  [F_VAL]="val", [F_FUN]="fun", [F_MAC]="let", [F_LABL]="labl"
};

// form names -----------------------------------------------------------------
value_t sym_quote, sym_do;
value_t sym_if, sym_and, sym_or;
value_t sym_val, sym_fun, sym_mac, sym_let, sym_labl;

size_t sxp_len(value_t x)
{
  size_t out = 0;

  if (is_cons(x))
    while (is_cons(x))
    {
      out++;
      x = cdr(x);
    }

  return out;
}

value_t lookup(value_t n, value_t e)
{
  if (sym_constant(n))
    return sym_bind(n);
  
  while (e != TOPLEVEL)
    {
      value_t names = env_names(e), binds = env_binds(e);

      uint_t n_binds = env_length(e);

      if (env_captured(e))
	{
	  for (uint_t i=0; i<n_binds; i++)
	    {
	      value_t argn = vec_ref(names, i), bindn = vec_ref(binds, n);
	      if (argn == n)
		return bindn;
	    }
	}

      else
	{
	  value_t *base = &Values[ival(binds)];

	  for (uint_t i=0; i<n_binds; i++)
	    {
	      value_t argn = vec_ref(names, i);
	      if (argn == n)
		return base[i];
	    }
	}

      e = env_parent(e);
    }

  require( sym_bound(n),
	   n,
	   "eval",
	   "unbound symbol %s",
	   sym_name(n) );

  return sym_bind(n);
}

bool_t is_literal(value_t x)
{
  return is_symbol(x) ? is_keyword(x) : !is_cons(x);
}

bool_t is_special_form(value_t x)
{
  return is_builtin(x) && ival(x) > F_EXIT;
}

bool_t is_macro(value_t x)
{
  return is_closure(x) && clo_macro(x);
}

bool_t validate_argc(value_t *x, size_t argc)
{
  static size_t builtin_arities[NUM_BUILTINS] = {

  };
  
  if (is_closure(*x))
    {
      uint_t arity = clo_arity(*x);
      
      if (clo_vargs(*x))
	return argc >= arity;

      if (clo_oargs(*x))
	{

	  value_t options = clo_args(*x);
	  
	  for (uint_t i=0; i<arity; i++)
	    {
	      value_t option   = vec_ref(options, i);
	      if (validate_argc(&option, argc))
		{
		  *x = option;
		  return true;
		}
	    }

	  return false;
	}

      return arity == argc;
    }

  if (is_vargs(*x))
    return argc >= builtin_arities[ival(*x)];

  if (is_oargs(*x))
    switch (*x)
      {
      case F_CLO:
	return argc > 2 && argc < 7;

      case F_SYM: case F_EXIT: case F_READ:
	return argc == 0 || argc == 1;

      case F_EVAL: case F_PRIN: case F_ERROR:
	return argc == 1 || argc == 2;

      case F_IF: case F_VAL:
	return argc == 2 || argc == 3;

      case F_FUN:
	return argc >= 2 && argc <= 4;

      case F_MAC: default:
	return argc == 2 || argc == 3;
      }

  else
    return argc == builtin_arities[ival(*x)];
}

bool_t is_oargs(value_t x)
{
  static bool_t builtin_oargs[NUM_BUILTINS] = {
    [F_CLO]   = true,  [F_SYM]   = true,
    [F_READ]  = true,  [F_PRIN]  = true,
    [F_EVAL]  = true,  [F_EXIT]  = true,
    [F_ERROR] = true,

    [F_IF]    = true, [F_VAL]    = true,
    [F_FUN]   = true, [F_MAC]    = true
  };
  
  if (is_closure(x))
    return clo_oargs(x);

  assert(is_builtin(x));
  return builtin_oargs[ival(x)];
}

bool_t is_vargs(value_t x)
{
  static bool_t builtin_vargs[NUM_BUILTINS] = {
    [F_VEC]  = true, [F_STR] = true, [F_BIN] = true,
    [F_AND]  = true, [F_OR]  = true,

    [F_OPEN] = true,
    
    [F_ADD]  = true, [F_SUB] = true, [F_MUL] = true,
    [F_DIV]  = true,
    
  };
  
  if (is_closure(x))
    return clo_vargs(x);

  assert(is_builtin(x));
  return builtin_vargs[ival(x)];
}

char *fun_name(value_t x)
{
  if (is_closure(x))
    return sym_name(clo_name(x));

  return builtinnames[ival(x)];
}

#define evalt(x)				\
  do						\
    {						\
      Sp = saveSp;				\
      Fp = saveFp;				\
      						\
      if (is_literal(x))			\
	return x;				\
      goto eval_top;				\
  						\
    } while(0)

#define type_predicate(type)			\
f_##type##p:					\
v = is_##type(Values[Bp]) ? TRUE : FALSE;	\
goto eval_end



value_t evalsx(value_t x, value_t e) {
  static void *labels[NUM_BUILTINS] = {
    // special forms ----------------------------------------------------------
    [F_QUOTE]     = &&ev_quote,    [F_DO]       = &&ev_do,

    [F_IF]        = &&ev_if,       [F_OR]       = &&ev_or,      [F_AND]   = &&ev_and,

    [F_VAL]       = &&ev_val,      [F_FUN]      = &&ev_fun,     [F_MAC]   = &&ev_mac,
    [F_LET]       = &&ev_let,      [F_LABL]     = &&ev_labl,

    // common functions -------------------------------------------------------
    // constructors -----------------------------------------------------------
    [F_PAIR]     = &&f_pair,       [F_CONS]  = &&f_cons,
    [F_LIST]      = &&f_list,      [F_SYM]      = &&f_sym,      [F_VEC]   = &&f_vec,
    [F_INT]       = &&f_int,       [F_BOOL]     = &&f_bool,

    // accessors --------------------------------------------------------------
    [F_CAR]       = &&f_car,       [F_CDR]      = &&f_cdr,      [F_FST]   = &&f_fst,
    [F_SND]       = &&f_fst,       [F_XST]      = &&f_sxt,      [F_XND]   = &&f_xnd,
    [F_HD]        = &&f_hd,        [F_TL]       = &&f_tl,

  // type predicates ----------------------------------------------------------
    [F_NONEP]     = &&f_nonep,     [F_ANYP]     = &&f_anyp,     [F_BOOLP]    = &&f_booleanp,
    [F_PORTP]     = &&f_portp,     [F_INTP]     = &&f_integerp, [F_PRIMP]    = &&f_builtinp,
    [F_CLOP]      = &&f_closurep,  [F_VECP]     = &&f_vectorp,  [F_PAIRP]    = &&f_pairp,
    [F_CONSP]     = &&f_consp,     [F_SYMP]     = &&f_symbolp,  [F_STRP]     = &&f_stringp,
    [F_BINP]      = &&f_binaryp,   [F_NILP]     = &&f_nilp,

  // type-ish predicates ------------------------------------------------------
    [F_FUNCP]     = &&f_functionp, [F_CELLP]    = &&f_cellp,
    [F_GENSYMP]   = &&f_gensymp,   [F_KEYWORDP] = &&f_keywordp,

    // arithmetic -------------------------------------------------------------
    [F_ADD]       = &&f_add,       [F_SUB]      = &&f_sub,      [F_MUL]      = &&f_mul,
    [F_DIV]       = &&f_div,       [F_MOD]      = &&f_mod,
    [F_EQP]       = &&f_eqp,       [F_LTP]      = &&f_ltp,

    // other predicates -------------------------------------------------------
    [F_IDP]       = &&f_idp,       [F_NOT]      = &&f_not,

    // io ---------------------------------------------------------------------
    [F_OPEN]      = &&f_open,      [F_CLOSE]    = &&f_close,    [F_EOFP]     = &&f_eofp,
    [F_READ]      = &&f_read,      [F_PRIN]     = &&f_prin,     [F_LOAD]     = &&f_load,

    // interpreter ------------------------------------------------------------
    [F_EVAL]      = &&f_eval,      [F_APPLY]    = &&f_apply,    [F_LOOKUP]   = &&f_lookup,

    // runtime ----------------------------------------------------------------
    [F_ERROR]     = &&f_error,     [F_EXIT]     = &&f_exit
  };

  uint_t saveSp = Sp, saveFp = Fp, Bp = Sp;

  gc_frame_t *save_frame = Saved;

  value_t f = NIL, a = NIL, v = NIL, pe = e;

  value_t tmpx, tmpy, tmpz, tmpa, tmpb, tmpc;

  if (setjmp(Toplevel)) {
    Sp    = saveSp;
    Fp    = saveFp;
    Saved = save_frame;
    
    return NIL;
  }

  preserve( 6, &x, &e, &f, &a, &v, &pe );

 eval_top:
  if (is_symbol(x))
    return lookup(x, e);

  f = car(x); a = cdr(x); f = eval(f, a);

  argtype( f, "eval", 2, type_closure, type_builtin, type_type );

  if (is_type(f))
    {
      require( f,
	       !(f&7),
	       "type %s not constructable",
	       builtinnames[ival(f)] );
      f = (f & IMASK) | FUNCTION;
    }

  size_t n_args = 0;
  
  if (is_special_form(f))
    {
      n_args = sxp_len(a);
      require( validate_argc( &f, n_args ),
	       f,
	       fun_name(f),
	       "bad arity" );
      
      goto *labels[ival(f)];
    }

  Bp = Sp;
  
  bool_t expand = is_macro(f);
  
  for_cons(&a, x)
    {
      v = expand ? x : eval(x, e);
      n_args++;
    }

  require( validate_argc( &f, n_args ),
	   f,
	   fun_name(f),
	   "bad arity" );

  if (is_builtin(f))
    goto apply_builtin;
  
 apply_closure:
  a = clo_body(f);
  e = clo_envt(f);

 apply_builtin:
  
  type_predicate(cons);
  type_predicate(pair);
  type_predicate(nil);
  type_predicate(none);
  type_predicate(symbol);
  type_predicate(vector);
  type_predicate(integer);
  type_predicate(boolean);
  type_predicate(binary);
  type_predicate(string);
  
  
  
 f_car:
  v = car_s(Values[Bp], "car");
  
  goto eval_end;

 f_cdr:
  v = cdr_s(Values[Bp], "cdr");

  goto eval_end;

  
    
 eval_end:
  Sp = saveSp;
  Fp = saveFp;

  if (expand)
    {
      x = v;
      e = pe;
      
      evalt(x);
    }

  return x;
}
