#include "object.h"

#include "apply.h"

#include "runtime.h"

#include "util/number.h"

// internal API ---------------------------------------------------------------
Tuple* get_sig(usize n, Val*args);

int compare_sigs(Tuple* args, Tuple* method);

// external API ---------------------------------------------------------------
Func* add_method(Func* func, bool va, Val method, Tuple* sig) {
  Method** table = &func->methods, *next = NULL;
  Dict* cache = NULL;

  while (*table != NULL) {
    next = *table;

    if (cache == NULL)
      cache = next->cache;

    int o = compare_sigs(sig, next->signature);

    if (o < 0)
      break;

    else if (o > 0)
      table = &next->next;

    else
      PANIC(NULL, EVAL_ERROR, "overlapping method signatures");
  }

  *table = mk_method(va*VARGS, next, cache, sig, method);

  return func;
}

Method* dispatch(Func* func, usize n, Val* args) {
  Method* table  = func->methods;
  Tuple*  argsig = get_sig(n, args);
  Val     cached = dict_get(table->cache, tag(argsig));

  if (cached != NOTFOUND)
    return as_method(cached);

  while (table) {
    
  }
}
