#include "util/number.h"

#include "runtime.h"

#include "equal.h"

#include "number.h"
#include "environment.h"
#include "array.h"
#include "table.h"
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

static bool method_accepts(Method* m, Tuple* s);

static bool is_exact_sig(Tuple* sig) {
  bool out = true;

  for (size_t i=0; out && i < sig->arity; i++)
    out = get_kind(sig_get(sig, i)) == DATA_KIND;

  return out;
}

void methn_table_intern(Table* t, TableKv* kv, Value key, void* s, Value* b) {
  (void)t;
  (void)b;

  MethodNode* parent, *child;

  parent  = s;
  child   = new_methn(parent->offset+1, get_fl(parent, 0));
  kv->key = key;
  kv->val = *b = tag(child);
}

static Table* new_methn_table(void) {
  return new_table(0, methn_table_intern, NULL, NULL);
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
      objects_push(ms, (Obj*)mn->leaf);
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

  if (mm != NULL && (is_va(mm) || s->arity <= mm->max_a))
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
static Method* insert_method_node(MethodNode* mn, Tuple* s, Obj* m, bool va);

static Method* insert_exact_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  Value child;
  Method* out;

  table_intern(mn->dtmap, tag(t), NULL, &child);
  out = insert_method_node(as_methn(child), s, m, va);

  return out;
}

static Method* insert_abstract_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  Value child;
  Method* out;
  
  table_intern(mn->atmap, tag(t), &mn, &child);
  out = insert_method_node(as_methn(child), s, m, va);

  return out;
}

static Method* insert_union_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  MethodNode* child;
  Method* out;

  for (size_t i = 0; child == NULL && i < mn->utmap->cnt; i += 2) {
    if (is_instance(t, (Type*)mn->utmap->data[i]))
      child = (MethodNode*)mn->utmap->data[i+1];
  }

  if (child == NULL) {
    child = new_methn(mn->offset+1, va);
    objects_pushn(mn->utmap, 2, t, child);
  }

  out = insert_method_node(mn, s, m, va);
  return out;
}

static Method* insert_any_method(MethodNode* mn, Type* t, Tuple* s, Obj* m, bool va) {
  Method* out;
  (void)t;

  if (mn->any == NULL)
    mn->any = new_methn(mn->offset+1, va);

  out = insert_method_node(mn->any, s, m, va);

  return out;
}

static Method* insert_method_node(MethodNode* mn, Tuple* s, Obj* m, bool va) {
  Method* out = NULL;

  assert(va == is_va(mn));

  if (s->arity == mn->offset - is_va(mn)) {
    if (mn->leaf != NULL)
      error("add-method!", "duplicate method signature");

    if (m->type != &MethodType)
      m = (Obj*)new_method(m, s, va);

    out = mn->leaf  = (Method*)m;
  }

  Type* t = sig_get(s, mn->offset);
  Kind  k = get_kind(t);

  switch (k) {
    case BOTTOM_KIND:
      error("add-method!", "can't specialize on `None`");
      break;

    case TOP_KIND:
      out = insert_any_method(mn, t, s, m, va);
      break;

    case DATA_KIND:
      out = insert_exact_method(mn, t, s, m, va);
      break;

    case ABSTRACT_KIND:
      out = insert_abstract_method(mn, t, s, m, va);
      break;

    case DATA_UNION_KIND:
    case ABSTRACT_UNION_KIND:
      out = insert_union_method(mn, t, s, m, va);
      break;
  }

  return out;
}

static Method* insert_method_map(MethodMap* mm, Tuple* s, Obj* m, bool va) {
  Method* out = NULL;

  assert(va == is_va(mm));

  if (mm->root == NULL)
    mm->root = new_methn(0, is_va(mm));

  out = insert_method_node(mm->root, s, m, );

  if (s->arity > mm->max_a)
    mm->max_a = s->arity;

  return out;
}

static Method* insert_method(MethodTable* mt, Tuple* s, Obj* m, bool va) {
  Method* out;

  if (va) {
    if (mt->va_methods == NULL)
      mt->va_methods = new_methm(true);

    out = insert_method_map(mt->va_methods, s, m, va);
  } else {
    if (mt->fa_methods == NULL)
      mt->fa_methods = new_methm(false);

    out = insert_method_map(mt->fa_methods, s, m, va);
  }

  return out;
}

// external APIs
// pseduo-accessors
size_t max_arity(Function* f) {
  if (f->leaf)
    return method_arity(f->leaf);

  else
    return max(f->metht->fa_methods->max_a, f->metht->va_methods->max_a);
}

bool has_va_methods(Function* f) {
  if (f->leaf)
    return is_va(f->leaf);

  
}

static void clear_method_cache(Function* f) {
  if (f->metht)
    free_table(f->metht->cache);
}

// constructors
MethodTable* new_metht(void) {
  MethodTable* out;

  out = new_obj(&MethodTableType, 0, 0);

  save(1, tag(out));

  out->cache      = new_table(0, NULL, NULL, NULL);
  out->fa_methods = NULL;
  out->va_methods = NULL;

  return out;
}

MethodMap* new_methm(flags_t fl) {
  
}

MethodNode*  new_methn(size_t offset, flags_t fl);
Method*      new_method(Obj* fn, Tuple* sig, flags_t fl);

// multimethods API
Method* get_method(Function* g, Tuple* s) {
  Method* m = NULL;

  if (g->leaf) {
    if (method_accepts(g->leaf, s))
      m = g->leaf;
  } else {
    Method* m = check_method_cache(g, s);

    if (m == NULL) {
      Objects ms = {
        .obj = {
          .type    =&ObjectsType,
          .annot   =&EmptyMap,
          .no_sweep=true,
          .gray    =true,
        },
        .data=NULL,
        .cnt =0,
        .cap =0
      };

      init_objects(&ms);
      dispatch(g->metht, s, &ms);

      if (ms.cnt > 0) {
        m = (Method*)objects_pop(&ms);

        free_objects(&ms);
        cache_method(g, s, m);
      }
    }
  }

  if (m == NULL)
    error(g->name->name, "no method matching given signature");

  return m;
}

Method* add_method(Function* g, Tuple* s, Obj* m, flags_t fl) {
  Method* out, * tmp;

  if (get_fl(g, FINAL))
    error("add-method!", "`%s` does not allow specialization.", g->name->name);

  save(3, tag(g), tag(s), tag(m));

  if (g->leaf) { // no methods yet added; method table must be created
    tmp = g->leaf;
    save(1, tag(tmp));
    g->metht = new_metht();
    g->leaf  = NULL;
    insert_method(g->metht, tmp->sig, (Obj*)tmp, fl);
  }

  out = insert_method(g->metht, s, m, fl);
  /**
   * A new method might be more specific than cached methods, and those caches
   * need to be removed. Clearing the entire cache probably isn't necessary, so
   * this is an obvious target for optimization at some point.
  **/
  clear_method_cache(g);

  return out;
}

// native functions
Value native_idp(Value x, Value y) {
  return x == y ? TRUE : FALSE;
}

Value native_type_of(Value x) {
  return tag(type_of(x));
}

Value native_not(Value x) {
  return is_truthy(x) ? FALSE : TRUE; 
}

Value native_hash(Value x) {
  return tag(hash(x));
}

Value native_eqp(Value x, Value y) {
  return tag(equal(x, y));
}

Value native_ord(Value x, Value y) {
  return tag(order(x, y));
}

Value native_identity(Value x) {
  return x;
}

/**
 * Arithmetic methods.
 *
 * Generally speaking, arithmetic methods have the following implementations:
 *
 * 1) binary imeplementations for matching types.
 * 2) 
 * 3) 
 *
 * 
**/

Value native_abs_small(Value x) {
  Small s = as_small(x);

  return tag(s < 0 ? -s : s);
}

Value native_abs_arity(Value x) {
  Arity a = as_arity(x);

  return tag(new_big(-(int64_t)a));
}

Value native_abs_float(Value x) {
  Float f = as_float(x);

  return tag(f < 0 ? -f : f);
}

Value native_abs_big(Value x) {
  Big *b    = as_big(x);
  int64_t v = b->val;

  return tag(new_big(v < 0 ? -v : v));
}

Value native_add_small_small(Value x, Value y) {
  int64_t r = as_small(x) + as_small(y);
  Value out = r >= INT32_MIN || r <= INT32_MAX ? tag_small(r) : tag(new_big(r));

  return out;
}

Value native_add_float_float(Value x, Value y) {
  return tag(as_float(x) + as_float(y));
}

Value native_add_arity_arity(Value x, Value y) {
  Arity   r = as_arity(x) + as_arity(y);

  if (r > ARITY_MAX)
    return tag(new_big(r));

  return tag(r);
}

Value native_add_big_big(Value x, Value y) {
  int64_t r = as_big(x)->val + as_big(y)->val;

  return tag(new_big(r));
}

Value native_add_number_number(Value x, Value y) {
  Type* xt = type_of(x), * yt = type_of(y);
  Type* common = promote(NULL, &x, &y);
  
  if (common == &SmallType)
    return native_add_small_small(x, y);

  else if (common == &FloatType)
    return native_add_float_float(x, y);

  else if (common == &ArityType)
    return native_add_arity_arity(x, y);

  else if (common == &BigType)
    return native_add_big_big(x, y);

  else {
    error("+", "couldn't find common type for %s and %s", t_name(xt), t_name(yt));

    return NOTHING;
  }
}

Value native_add_many(size_t n, Value* args) {
  Value x = args[0];

  for (size_t i=1; i < n; i++)
    x = native_add_number_number(x, args[i]);

  return x;
}

Value native_head(Value x) {
  List* xs = as_list(x);

  require(xs->arity > 0, "head", "`()` has no head.");
  return xs->head;
}

Value native_tail(Value x) {
  List* xs = as_list(x);
  require(xs->arity > 0, "tail", "`()` has no tail.");
  return tag(xs->tail);
}

Value native_walk_function_list(Value fn, Value xs) {
  
}

Value native_walk_closure_list(Value fn, Value xs) {
  
}

// initialization
static Tuple* builtin_function_signature_v(size_t n, va_list va) {
  Type* type;
  Value types[n];

  for (size_t i=0; i<n; i++) {
    type     = va_arg(va, Type*);
    types[i] = tag(type);
  }

  return new_tuple(n, types);
}

static Tuple* builtin_function_signature(size_t n, ...) {
  Tuple* out;
  va_list va;
  
  va_start(va, n);
  out = builtin_function_signature(n, va);
  va_end(va);

  return out;
}

static Function* init_builtin_function(char* name) {
  Symbol* sym;
  Function* func;

  sym  = symbol(name);
  func = new_func(sym, 0, NULL);
  define(NULL, sym, tag(func), FINAL);

  return func;
}

static void add_builtin_method(Function* func, Native* fn, bool va, size_t n, ...) {
  Tuple* sig;
  Method* meth;

  
}

static void init_builtin_final_function(char* name, Native* fn, bool va, size_t n, ...) {
  Tuple* sig;
  Symbol* sym;
  Function* func;
  Method* meth;
  va_list va_l;

  va_start(va_l, n);
  sig  = builtin_function_signature_v(n, va_l);
  sym  = symbol(name);
  meth = new_method((Obj*)fn, sig, va);
  func = new_func(sym, FINAL, (Obj*)meth);
  define(NULL, sym, tag(func), FINAL);
  va_end(va_l);
}

#define NATIVE_FN0(F, f)                         \
  Native Native##F = {                           \
    .obj={                                       \
      .type =&NativeType,                        \
      .annot=&EmptyMap,                          \
    },                                           \
    .fn.n0=native_##f                            \
  }

#define NATIVE_FN1(F, f)                         \
  Native Native##F = {                           \
    .obj={                                       \
      .type =&NativeType,                        \
      .annot=&EmptyMap,                          \
    },                                           \
    .fn.n1=native_##f                            \
  }

#define NATIVE_FN2(F, f)                         \
  Native Native##F = {                           \
    .obj={                                       \
      .type =&NativeType,                        \
      .annot=&EmptyMap,                          \
    },                                           \
    .fn.n2=native_##f                            \
  }

#define NATIVE_FNN(F, f)                         \
  Native Native##F = {                           \
    .obj={                                       \
      .type =&NativeType,                        \
      .annot=&EmptyMap,                          \
    },                                           \
    .fn.nn=native_##f                            \
  }

NATIVE_FN2(Idp, idp);
NATIVE_FN2(Eqp, eqp);
NATIVE_FN1(TypeOf, type_of);
NATIVE_FN1(Not, not);
NATIVE_FN1(Hash, hash);
NATIVE_FN1(Identity, identity);

/* arithmetic */
NATIVE_FN1(AbsSmall,        abs_small);
NATIVE_FN1(AbsFloat,        abs_float);
NATIVE_FN1(AbsArity,        abs_arity);
NATIVE_FN1(AbsBig,          abs_big);
NATIVE_FN2(AddSmallSmall,   add_small_small);
NATIVE_FN2(AddFloatFloat,   add_float_float);
NATIVE_FN2(AddArityArity,   add_arity_arity);
NATIVE_FN2(AddBigBig,       add_big_big);
NATIVE_FN2(AddNumberNumber, add_number_number);
NATIVE_FNN(AddMany,         add_many);

/* collections */
NATIVE_FN1(Head, head);
NATIVE_FN1(Tail, tail);

void init_builtin_functions(void) {
  Function* func;

  /**
   * These functions don't benefit from specialization for one reason or another,
   * so they're declared final. At some point in the future the ability to override
   * some of these probably ought to be exposed.
   **/

  init_builtin_final_function("id?", &NativeIdp, false, 2, &AnyType, &AnyType);
  init_builtin_final_function("eq?", &NativeEqp, false, 2, &AnyType, &AnyType);
  init_builtin_final_function("type-of", &NativeTypeOf, false, 1, &AnyType);
  init_builtin_final_function("not", &NativeNot, false, 1, &AnyType);
  init_builtin_final_function("hash", &NativeHash, false, 1, &AnyType);
  init_builtin_final_function("identity", &NativeIdentity, false, 1, &AnyType);

  /**
   * Arithmetic functions.
   *
   * Generally speaking, arithmetic functions 
   **/

  func = init_builtin_function("abs");

  add_builtin_method(func, &NativeAbsSmall, false, 1, &SmallType);
  add_builtin_method(func, &NativeAbsArity, false, 1, &ArityType);
  add_builtin_method(func, &NativeAbsFloat, false, 1, &FloatType);
  add_builtin_method(func, &NativeAbsBig,   false, 1, &BigType);

  func = init_builtin_function("+");

  add_builtin_method(func, &NativeIdentity, false, 1, &SmallType);
  add_builtin_method(func, &NativeIdentity, false, 1, &FloatType);
  add_builtin_method(func, &NativeIdentity, false, 1, &ArityType);
  add_builtin_method(func, &NativeIdentity, false, 1, &BigType);
  add_builtin_method(func, &NativeAddSmallSmall, false, 2, &SmallType, &SmallType);
  add_builtin_method(func, &NativeAddArityArity, false, 2, &ArityType, &ArityType);
  add_builtin_method(func, &NativeAddFloatFloat, false, 2, &FloatType, &FloatType);
  add_builtin_method(func, &NativeAddBigBig, false, 2, &BigType, &BigType);
  add_builtin_method(func, &NativeAddNumberNumber, false, 2, &NumberType, &NumberType);
  add_builtin_method(func, &NativeAddMany, true, 2, &NumberType, &NumberType);

  func = init_builtin_function("-");

  func = init_builtin_function("*");

  func = init_builtin_function("/");

  func = init_builtin_function("=");

  func = init_builtin_function("<");

  func = init_builtin_function("rem");

  func = init_builtin_function("promote");

  /**
   * Collection functions.
   *
   **/

  init_builtin_final_function("head", &NativeHead, false, 1, &ListType);
  init_builtin_final_function("tail", &NativeTail, false, 1, &ListType);

  func = init_builtin_function("cnt");

  func = init_builtin_function("ref");

  func = init_builtin_function("xef");

  func = init_builtin_function("add");

  func = init_builtin_function("del");

  func = init_builtin_function("walk");

  func = init_builtin_function("keep");

  func = init_builtin_function("fold");

  func = init_builtin_function("cat");

  func = init_builtin_function("rev");

  
}
