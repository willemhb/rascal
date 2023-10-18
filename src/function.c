#include "runtime.h"
#include "vm.h"

#include "array.h"
#include "type.h"
#include "collection.h"
#include "function.h"

// pseudo-accessors
static Type* sig_get(Tuple* s, size_t n) {
  return as_type(s->slots[n]);
}

static size_t method_arity(const Method* m) {
  return m->sig->arity;
}

static bool is_va(void* p) {
  return get_fl(p, VARIADIC);
}

static bool is_exact(void* p) {
  return get_fl(p, EXACT);
}

// dispatch helpers
static bool search_method_node(MethodNode* mn, Tuple* s, Objects* ms);

static bool search_exact_children(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;
  Type* t = sig_get(s, mn->offset);
  Value child;

  if (table_get(mn->dtmap, tag(t), &child))
    out = search_method_node(as_methn(child), s, ms);

  return out;
}

static bool search_abstract_children(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;
  Type* t  = sig_get(s, mn->offset);
  Value child;

  for (Type* p = t->parent; p != &AnyType; p=p->parent) {
    if (table_get(mn->atmap, tag(p), &child))
      search_method_node(as_methn(child), s, ms);
  }

  return out;
}

static bool search_union_children(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;
  Type* t  = sig_get(s, mn->offset);

  for (size_t i = 0;i < mn->utmap->cnt; i += 2) {
    if (is_instance(t, (Type*)mn->utmap->data[i]))
      search_method_node((MethodNode*)mn->utmap->data[i+1], s, ms);
  }

  return out;
}

static bool search_any_children(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;

  if (mn->any != NULL)
    search_method_node(mn->any, s, ms);

  return out;
}

static bool search_method_node(MethodNode* mn, Tuple* s, Objects* ms) {
  bool out = false;

  if (is_va(mn)) { // different search procedure
    if (mn->leaf != NULL && s->arity >= method_arity(mn->leaf))
      objects_push(ms, as_obj(mn->leaf));

    search_exact_children(mn, s, ms);
    search_abstract_children(mn, s, ms);
    search_union_children(mn, s, ms);
    search_any_children(mn, s, ms);
  } else {
    if (s->arity == mn->offset && mn->leaf != NULL) {
      objects_push(ms, mn->leaf);
      out = is_exact(mn);
    } else {
      out = search_exact_children(mn, s, ms);

      if (!out) {
        search_abstract_children(mn, s, ms);
        search_union_children(mn, s, ms);
        search_any_children(mn, s, ms);
      }
    }
  }

  return out;
}

static bool search_method_map(MethodMap* mm, Tuple* s, Objects* ms) {
  bool out = false;

  if (mm != NULL && s->arity >= (is_va(mm) || s->arity <= mm->max_a))
    out = search_method_node(mm->root, s, ms);
 
  return out;
}

int order_methods(const void* x, const void* y) {
  return order_sigs(x, y);
}

static void sort_methods(size_t cnt, Method** ms) {
  qsort(ms, cnt, sizeof(Method*), order_methods);
}

static void dispatch(MethodTable* mt, Tuple* s, Objects* ms) {
  bool exact = search_method_map(mt->fa_methods, s, ms);

  if (!exact)
    exact = search_method_map(mt->va_methods, s, ms);

  // if an exact match hasn't been found, sort applicable methods to find the best one
  if (!exact)
    sort_methods(ms->cnt, (Method**)ms->data);
}

static void cache_method(Function* g, Tuple* s, Method* m) {
  table_set(g->metht->cache, tag(s), tag(m));
}

static Method* check_method_cache(Function* g, Tuple* s) {
  Value b;

  table_get(g->metht->cache, tag(s), &b);

  return as_method(b);
}

// specialize helpers
static void insert_method_node(MethodNode* mn, Tuple* s, Obj* m, bool va);

static void insert_exact_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  Value child;

  table_intern(mn->dtmap, tag(t), NULL, &child);
  insert_method_node(as_methn(child), s, m, va);
}

static void insert_abstract_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  MethodNode* child = NULL;

  typeMapAdd(&mn->atmap, t, &child, mn);
  insert_method_node(child, s, m, va);
}

static void insert_union_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  MethodNode* child = NULL;

  for (size_t i = 0; child == NULL && i < mn->utmap.count; i += 2) {
    if (isMember(t, mn->utmap.data[i]))
      child = mn->utmap.data[i+1];
  }

  if (child == NULL) {
    child = new_methn(mn->offset+1, mn->exact, mn->va);
    vWriteObjects(&mn->utmap, 2, t, child);
  }

  insert_method_node(mn, s, m, va);
}

static void insertAnyMethod(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  (void)t;

  if (mn->any == NULL)
    mn->any = new_methn(mn->offset+1, false, mn->va);

  insert_method_node(mn->any, s, m, va);
}

static void insert_method_node(MethodNode* mn, Tuple* s, Obj* m, bool va) {
  assert(va == is_va(mn));

  if (s->arity == mn->offset - mn->va) {
    if (mn->leaf != NULL)
      raise("add-method!", "duplicate method signature");

    if (m->type != &MethodType)
      m = (Obj*)new_method(m, s, va);

    mn->leaf  = (Method*)m;
  }

  Type* t = sig_get(s, mn->offset);
  Kind  k = get_kind(t);

  switch (k) {
    case BOTTOM_KIND:
      raise("add-method!", "can't specialize on `None`");
      break;

    case TOP_KIND:
      insertAnyMethod(mn, t, s, m, va);
      break;

    case DATA_KIND:
      insert_exact_method(mn, t, s, m, va);
      break;

    case ABSTRACT_KIND:
      insert_abstract_method(mn, t, s, m, va);
      break;

    case DATA_UNION_KIND:
    case ABSTRACT_UNION_KIND:
      insert_union_method(mn, t, s, m, va);
      break;
  }
}

static void insert_methm(MethodMap* mm, Tuple* s, Obj* m, bool va) {
  assert(va == mm->va);

  if (mm->root == NULL)
    mm->root = new_methn(0, is_va(mm), !is_va(mm));

  insert_method_node(mm->root, s, m, va);

  if (s->arity > mm->maxA)
    mm->maxA = s->arity;
}

static void insert_method(MethodTable* mt, Tuple* s, Obj* m, bool va) {
  if (va) {
    if (mt->va_methods == NULL)
      mt->va_methods = new_methm(true);

    insert_methm(mt->va_methods, s, m, va);
  } else {
    if (mt->fa_methods == NULL)
      mt->fa_methods = new_methm(false);

    insert_methm(mt->fa_methods, s, m, va);
  }
}

// external APIs
Method* get_method(Function* g, Tuple* s) {
  Method* m;

  if (g->leaf)
    m = g->leaf;

  else {
    Method* m = check_method_cache(g, s);

    if (m == NULL) {
      Objects ms;
      init_objects(&ms);
      dispatch(g->metht, s, &ms);

      if (ms.cnt == 0)
        raise(g->name->name, "no method matching given types");

      m = (Method*)objects_pop(&ms);

      free_objects(&ms);
      cache_method(g, s, m);
    }
  }

  return m;
}

Method* add_method(Function* g, Tuple* s, Obj* m, bool va) {
  size_t nsv;
  Method* out, * tmp;
  
  if (get_fl(g, FINAL))
    raise("add-method!", "`%s` does not allow specialization.", g->name->name);

  nsv = save(3, tag(g), tag(s), tag(m));

  if (g->leaf) { // no methods yet added; method table must be created
    tmp = g->leaf;
    nsv += save(1, tag(tmp));
    g->metht = new_metht();
    g->leaf  = NULL;
    insert_method(g->metht, tmp->sig, (Obj*)tmp, va);
  }

  out = insert_method(g->metht, s, m, va);
  unsave(nsv);
}
