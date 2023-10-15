#include "runtime.h"
#include "object.h"
#include "function.h"

static Type* getNthType(Tuple* s, size_t n) {
  return AS_TYPE(s->slots[n]);
}

static void dispatch(MethodTable* mt, Tuple* s, Objects* ms);

static void cacheMethod(Function* g, Tuple* s, Method* m) {
  methodCacheAdd(&g->methods->cache, s, &m);
}

static Method* checkMethodCache(Function* g, Tuple* s) {
  Method* m;

  methodCacheGet(&g->methods->cache, s, &m);

  return m;
}

// external APIs
Method* getMethod(Function* g, Tuple* s) {
  Method* m;

  if (g->singleton)
    m = g->singleton;

  else {
    Method* m = checkMethodCache(g, s);

    if (m == NULL) {
      Objects ms;
      initObjects(&ms);
      dispatch(g->methods, s, &ms);

      if (ms.count == 0)
        raise(g->name->name, "no method matching given types");

      m = (Method*)popObjects(&ms);

      freeObjects(&ms);
      cacheMethod(g, s, m);
    }
  }
  
  return m;
}

void addMethod(Function* generic, Tuple* sig, Obj* specialized);
