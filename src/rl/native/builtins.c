#include <stdlib.h>

#include "vm/error.h"
#include "vm/value.h"

#include "obj/fixnum.h"
#include "obj/bool.h"
#include "obj/type.h"
#include "obj/symbol.h"
#include "obj/native.h"

#include "rl/native/builtins.h"

/* commentary

   miscellaneous builtin functions. */

/* C types */

/* globals */

/* API */
value_t native_idp( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("id?", 2, nargs)) < 0 )
    return NUL;

  return args[0] == args[1] ? TRUE : FALSE;
}

value_t native_isap( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("isa?", 2, nargs)) < 0 )
    return NUL;

  if ( (status=check_argtypes("isa?", args, 2, NULL, &TypeType)) )
    return NUL;

  return rl_isa(args[0], as_type(args[1])) ? TRUE : FALSE;
}

value_t native_not( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("not", 1, nargs)) < 0 )
    return NUL;

  return as_cbool(args[0]) ? FALSE : TRUE;
}

value_t native_typeof( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("typeof", 1, nargs)) < 0 )
    return NUL;

  return tag_object(rl_typeof(args[0]));
}

value_t native_lookup( value_t *args, int nargs )
{
  int status;
  
  if ( (status=check_argco("lookup", 1, nargs)) < 0 )
    return NUL;

  if ( (status=check_argtypes("lookup", args, 1, &SymbolType)) < 0 )
    return NUL;

  if ( !is_bound_at_toplevel(as_symbol(args[0])) )
    {
      panic("Unbound symbol %s", get_symbol_name(as_symbol(args[0])));
      return NUL;
    }

  return lookup_at_toplevel(as_symbol(args[0]));
}

value_t native_boundp( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("bound?", 1, nargs)) < 0 )
    return NUL;

  if ( (status=check_argtypes("bound?", args, 1, &SymbolType)) < 0 )
    return NUL;

  return is_bound_at_toplevel(as_symbol(args[0])) ? TRUE : FALSE;
}

value_t native_panic( value_t *args, int nargs )
{
  (void)args;

  int status;

  if ( (status=check_argco("panic", 0, nargs)) < 0 )
    return NUL;

  panic("Something went wrong");
  return NUL;
}

value_t native_exit( value_t *args, int nargs )
{
  (void)args;
  
  int status;

  if ( (status=check_argco("exit", 0, nargs)) < 0 )
    return NUL;

  exit(0);
}


/* runtime dispatch */
void rl_rl_native_builtins_init( void )
{
  define_native("id?", native_idp);
  define_native("isa?", native_isap);
  define_native("not", native_not);
  define_native("typeof", native_typeof);
  define_native("lookup", native_lookup);
  define_native("bound?", native_boundp);
  define_native("panic", native_panic);
  define_native("exit", native_exit);
}

void rl_rl_native_builtins_mark( void ) {}
void rl_rl_native_builtins_cleanup( void ) {}

