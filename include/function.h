#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"

// C types --------------------------------------------------------------------

typedef enum
  {
    // constructors -----------------------------------------------------------
    f_nil, f_cons, f_list,

    f_bool, f_sym,

    f_func, f_port,

    f_bin, f_vec, f_table,

    f_char, f_real,

    f_none, f_any,

    // predicates/comparison --------------------------------------------------
    f_idp, f_isap, f_flagp, f_not, f_ord,

    // misc utilities ---------------------------------------------------------
    f_type,

    // arithmetic & numbers ---------------------------------------------------
    f_add, f_sub, f_mul, f_div, r_rem,

    f_ltp, f_eqp, f_intp,

    // accessors --------------------------------------------------------------
    f_arity, f_size, f_flags,
    
    f_car, f_cdr, f_nth, f_ref,

    f_xar, f_xdr, f_xth, f_xef,

    f_conj, f_put, f_pop,

    // io ---------------------------------------------------------------------
    f_open, f_close, f_eosp,

    f_read, f_prin, f_load,

    f_readc, f_princ, f_peekc, f_ctypep,

    // compiler & interpreter -------------------------------------------------
    f_comp, f_compf, f_exec, f_apply,

    // runtime support --------------------------------------------------------
    f_error, f_getenv, f_sys, f_exit,

    num_builtins,
  } builtin_t;

typedef enum
  {
    fn_fl_primitive= 0,
    fn_fl_builtin  = 1,
    fn_fl_closure  = 2,
    fn_fl_generic  = 3, // unused

    fn_fl_type     = 4,

    fn_fl_vargs    = 8,

    fn_fl_macro    =16
  } fn_fl_t;

struct function_t
{
  HEADER;

  object_t *name;

  union
  {
    builtin_t   label;
    value_t   (*behavior)(value_t *args, size_t n_args);

    struct
    {
      object_t *locals;
      object_t *closure;
      object_t *constants;
      object_t *instructions;
    };
  };
};

// macros & statics -----------------------------------------------------------


#endif
