#include "obj/native.h"
#include "obj/real.h"

#include "vm/error.h"

#include "rl/native/arithmetic.h"

/* commentary */

/* C types */

/* globals */

/* API */
void empty_guard( const char *fname, value_t *args )
{
  (void)fname;
  (void)args;
}

void guard_division( const char *fname, value_t *args )
{
  if ( Vm.panic_mode )
    return;

  if ( as_real(args[1]) == 0.0 )
    panic("zero-division error in '(%s %.2f 0)'", fname, as_real(args[0]));
}

#define native_op(fname, op, guard)				\
  value_t native_##fname( value_t *args, size_t n_args )	\
  {								\
    check_argco(#op, 2, n_args);				\
    check_argtypes(#op, args, 2, &RealType, &RealType);		\
    guard(#op, args);						\
    								\
    if (Vm.panic_mode)						\
      return NUL;						\
    								\
    return fname##_reals(args[0], args[1]);			\
  }

#define native_pred(fname, op, guard)					\
  value_t native_##fname( value_t *args, size_t n_args )		\
  {									\
    check_argco(#op, 2, n_args);					\
    check_argtypes(#op, args, 2, &RealType, &RealType);			\
    guard(#op, args);							\
    									\
    return fname##_reals(args[0], args[1]);				\
  }									\

native_op(add, +, empty_guard);
native_op(sub, -, empty_guard);
native_op(mul, *, empty_guard);
native_op(div, /, guard_division);

native_pred(eq, =, empty_guard);
native_pred(lt, <, empty_guard);
native_pred(gt, >, empty_guard);
native_pred(leq, <=, empty_guard);
native_pred(geq, >=, empty_guard);
native_pred(neq, /=, empty_guard);

/* runtime */
void rl_rl_native_arithmetic_init( void )
{
  /* set toplevel bindings for arithmetic functions */
  define_native("+", native_add);
  define_native("-", native_sub);
  define_native("*", native_mul);
  define_native("/", native_div);
  define_native("=", native_eq);
  define_native("<", native_lt);
  define_native(">", native_gt);
  define_native("<=", native_leq);
  define_native(">=", native_geq);
  define_native("/=", native_neq);
}

void rl_rl_native_arithmetic_mark( void ) {}
