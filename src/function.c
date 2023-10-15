#include "runtime.h"
#include "vm.h"
#include "object.h"
#include "function.h"

// pseudo-accessors
static Type* sigGet(Tuple* s, size_t n) {
  return AS_TYPE(s->slots[n]);
}

static size_t methodArity(const Method* m) {
  return m->sig->arity;
}

// dispatch helpers
static bool searchMethodNode(MethodNode* mn, Tuple* s, Objects* ms);

static bool searchExactChildren(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;
  Type* t = sigGet(s, mn->offset);
  MethodNode* child;

  if (typeMapGet(&mn->dtmap, t, &child))
    out = searchMethodNode(child, s, ms);

  return out;
}

static bool searchAbstractChildren(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;
  Type* t  = sigGet(s, mn->offset);
  MethodNode* child;

  for (Type* p = t->parent; p != &AnyType; p=p->parent) {
    if (typeMapGet(&mn->atmap, p, &child))
      searchMethodNode(child, s, ms);
  }

  return out;
}

static bool searchUnionChildren(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;
  Type* t  = sigGet(s, mn->offset);

  for (size_t i = 0;i < mn->utmap.count;i += 2) {
    if (isMember(t, mn->utmap.data[i]))
      searchMethodNode(mn->utmap.data[i+1], s, ms);
  }

  return out;
}

static bool searchAnyChildren(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;

  if (mn->any != NULL)
    searchMethodNode(mn->any, s, ms);

  return out;
}

static bool searchMethodNode(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;

  if (mn->va) { // different search procedure
    if (mn->leaf != NULL && s->arity >= methodArity(mn->leaf))
      writeObjects(ms, mn->leaf);

    searchExactChildren(mn, s, ms);
    searchAbstractChildren(mn, s, ms);
    searchUnionChildren(mn, s, ms);
    searchAnyChildren(mn, s, ms);
  } else {
    if (s->arity == mn->offset && mn->leaf != NULL) {
      writeObjects(ms, mn->leaf);
      out = mn->exact;
    } else {
      out = searchExactChildren(mn, s, ms);

      if (!out) {
        searchAbstractChildren(mn, s, ms);
        searchUnionChildren(mn, s, ms);
        searchAnyChildren(mn, s, ms);
      }
    }
  }

  return out;
}

static bool searchMethodMap(MethodMap* mm, Tuple* s, Objects* ms) {
  bool out = false;

  if (mm != NULL && s->arity >= mm->minA && (mm->va || s->arity <= mm->maxA))
    out = searchMethodNode(mm->root, s, ms);
 
  return out;
}

int orderMethods(const void* x, const void* y) {
  /* heuristic comparison of the specificity of different methods.

     Generally speaking, for two methods `a` and `b`:

       1) `a` is more specific than `b` if it takes more inputs;
       2) otherwise, `a` is more specific than `b` if it contains more exact annotations in its signature;
       3) otherwise, `a` is more specific than `b` if it contains fewer `Any` annotations in its signature;
       4) otherwise, `a` and `b` are regarded as having the same specificity. */

  int o;

  const Method* mx = x;
  const Method* my = y;

  o = (int)mx->sig->arity - (int)my->sig->arity;
  o = o ? : (int)mx->nExact - (int)my->nExact;
  o = o ? : (int)my->nAny - (int)mx->nAny;

  return o;
}

static void sortMethods(size_t cnt, Method** ms) {
  qsort(ms, cnt, sizeof(Method*), orderMethods);
}

static void dispatch(MethodTable* mt, Tuple* s, Objects* ms) {
  bool exact = searchMethodMap(mt->faMethods, s, ms);

  if (!exact)
    exact = searchMethodMap(mt->vaMethods, s, ms);

  // if an exact match hasn't been found, sort applicable methods to find the best one
  if (!exact)
    sortMethods(ms->count, (Method**)ms->data);
}

static void cacheMethod(Function* g, Tuple* s, Method* m) {
  methodCacheAdd(&g->methods->cache, s, &m, NULL);
}

static Method* checkMethodCache(Function* g, Tuple* s) {
  Method* m;

  methodCacheGet(&g->methods->cache, s, &m);

  return m;
}

// specialize helpers
static void insertMethodNode(MethodNode* mn, Tuple* s, Obj* m, bool va);

static void insertExactMethod(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  MethodNode* child = NULL;

  typeMapAdd(&mn->dtmap, t, &child, mn);
  insertMethodNode(child, s, m, va);
}

static void insertAbstractMethod(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  MethodNode* child = NULL;

  typeMapAdd(&mn->atmap, t, &child, mn);
  insertMethodNode(child, s, m, va);
}

static void insertUnionMethod(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  MethodNode* child = NULL;

  for (size_t i = 0; child == NULL && i < mn->utmap.count; i += 2) {
    if (isMember(t, mn->utmap.data[i]))
      child = mn->utmap.data[i+1];
  }

  if (child == NULL) {
    child = newMethodNode(mn->offset+1, mn->exact, mn->va);
    vWriteObjects(&mn->utmap, 2, t, child);
  }

  insertMethodNode(mn, s, m, va);
}

static void insertAnyMethod(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  (void)t;

  if (mn->any == NULL)
    mn->any = newMethodNode(mn->offset+1, false, mn->va);

  insertMethodNode(mn->any, s, m, va);
}

static void insertMethodNode(MethodNode* mn, Tuple* s, Obj* m, bool va) {
  assert(va == mn->va);

  if (s->arity == mn->offset - mn->va) {
    if (mn->leaf != NULL)
      raise("add-method!", "duplicate method signature");

    if (m->type != &MethodType)
      m = (Obj*)newMethod(m, s, va);

    mn->leaf  = (Method*)m;
    mn->exact = mn->leaf->exact;
  }

  Type* t = sigGet(s, mn->offset);
  Kind  k = getKind(t);

  switch (k) {
    case BOTTOM_KIND:
      raise("add-method!", "can't specialize on `None`");
      break;

    case TOP_KIND:
      insertAnyMethod(mn, t, s, m, va);
      break;

    case DATA_TYPE_KIND:
      insertExactMethod(mn, t, s, m, va);
      break;

    case ABSTRACT_TYPE_KIND:
      insertAbstractMethod(mn, t, s, m, va);
      break;

    case UNION_TYPE_KIND:
      insertUnionMethod(mn, t, s, m, va);
      break;
  }
}

static void insertMethodMap(MethodMap* mm, Tuple* s, Obj* m, bool va) {
  assert(va == mm->va);

  if (mm->root == NULL)
    mm->root = newMethodNode(0, mm->va, !mm->va);

  insertMethodNode(mm->root, s, m, va);

  if (s->arity > mm->maxA)
    mm->maxA = s->arity;
}

static void insertMethod(MethodTable* mt, Tuple* s, Obj* m, bool va) {
  if (va) {
    if (mt->vaMethods == NULL)
      mt->vaMethods = newMethodMap(true);

    insertMethodMap(mt->vaMethods, s, m, va);
  } else {
    if (mt->faMethods == NULL)
      mt->faMethods = newMethodMap(false);

    insertMethodMap(mt->faMethods, s, m, va);
  }
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

void addMethod(Function* g, Tuple* s, Obj* m, bool va) {
  if (getFl(g, FINAL, 0))
    raise("add-method!", "`%s` does not allow specialization.", g->name->name);

  save(3, tag(g), tag(s), tag(m));

  if (g->singleton) { // no methods yet added; method table must be created
    Method* st = g->singleton;
    save(1, tag(st));
    g->methods   = newMethodTable();
    g->singleton = NULL;
    insertMethod(g->methods, st->sig, (Obj*)st, st->va);
    unsave(1);
  }

  insertMethod(g->methods, s, m, va);
  unsave(3);
}
