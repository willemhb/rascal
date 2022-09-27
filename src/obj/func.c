#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils/num.h"
#include "utils/arr.h"
#include "utils/str.h"

#include "obj/func.h"

#include "mem.h"
#include "exec.h"
#include "rt.h"



static const size_t meths_min_cap = 32;

#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

NEW_ARRAY(meths, method_t*);
PAD_STACK_SIZE(meths, methods_t*);
INIT_ARRAY(meths, method_t*);
RESIZE_ARRAY(meths, method_t*);

static void meths_put( meths_t *meths, arity_t n, method_t **buf)
{
  
}

static bool get_method_from_cache( meths_t  *cache, sig_t *args, arity_t *index, method_t **buf);
static bool get_method_from_table( method_t *root,  sig_t *args, method_t **buf);

bool get_method(func_t *func, arity_t n, val_t *args, method_t **buf )
{
  methods_t *table   = func->methods;
  arity_t    argmin  = table->argmin;
  arity_t    argmax  = table->argmax;
  bool       vargs   = flag_p(table->object.flags, method_fl_vargs);

  // ensure a method exists that can accommodate
  if (n < argmin)
    return false;

  if (vargs && n >= argmax)
    {
      arity_t nvargs = n - argmax;
      val_t   vlist  = listn(args+argmax, nvargs);
      popn( &Vm.stack, nvargs );
      push( &Vm.stack, vlist );

      n = argmax;
    }

  type_t types[n];
  hash_t hash = FNV64_OFFSET;

  // compute types and signature hash at the same time
  for (arity_t i=0; i<n; i++)
    {
      types[n] = typeof_val(args[n]);
      hash    ^= types[n];
      hash    *= FNV64_PRIME;
    }

  sig_t argsig = { types, n, 0, hash };
  arity_t index;

  if (get_method_from_cache(&table->cache, &argsig, &index, buf))
    return true;

  if (get_method_from_table(table->tree, &argsig, buf))
    {
      if (buf)
	{
	  
	}
    }
}
