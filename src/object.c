#include <string.h>

#include "util/number.h"
#include "util/hashing.h"

#include "vm.h"
#include "equal.h"
#include "object.h"

// generics
#include "tpl/describe.h"

ARRAY_TYPE(Values, Value, Value, false);
ARRAY_TYPE(Objects, Obj*, Obj*, false);
ARRAY_TYPE(ByteCode, uint16_t, int, false);

// external APIs
// flags
int getFl(void* p, int f, int m) {
  assert(p != NULL);
  
  Obj* o = p;
  int out;

  if (m == 0)
    out = !!(o->flags & f);

  else if (f == 0)
    out = o->flags & m;

  else
    out = (o->flags & m) == f;

  return out;
}

int setFl(void* p, int f, int m) {
  assert(p != NULL);

  int out;
  Obj* o = p;

  if (m == 0) {
    out       = !!(o->flags & f);
    o->flags |= f;
  } else if (f == 0) {
    out       = o->flags & m;
    o->flags &= ~m;
  } else {
    out       = o->flags & m;
    o->flags &= ~m;
    o->flags |= f;
  }

  return out;
}

int delFl(void* p, int f) {
  return setFl(p, 0, f);
}

// object copying
void* clone(void* p, int f) {
  assert(p != NULL);

  save(1, tag(p));

  Obj* out   = NULL;
  Obj*    o  = p;
  Type*   t  = o->type;
  CloneFn fn = t->vTable->clone;

  if (fn)
    out = fn(p, f);

  else
    out = duplicate(&RlVm, p, sizeOf(p));

  // make sure to add it to live objects
  out->next      = RlVm.heap.objs;
  RlVm.heap.objs = out;

  unsave(1);
  return out;
}

void* newObj(Type* type, int fl, size_t n, void* data) {
  void* out = allocObj(type, fl, n);
  initObj(out, type, fl, n, data);
  return out;
}

void* allocObj(Type* t, int f, size_t n) {
  AllocFn alloc = t->vTable->alloc;
  void* out;

  if (alloc)
    out = alloc(t, f, n);

  else
    out = allocate(&RlVm, t->vTable->objSize);

  return out;
}

void initObj(void* p, Type* t, int f, size_t n, void* d) {
  Obj* o = p;
  InitFn init = t->vTable->init;

  if (init)
    init(p, t, f, n, d);
  
  o->type  = t;
  o->annot = &emptyMap;
  o->flags = f;
  o->gray  = true;
  o->next  = RlVm.heap.objs;
  RlVm.heap.objs = o;
}

// constructors and lifetime methods
// Symbol type
void initSymbol(void* p, Type* t, int f, size_t n, void* d) {
  (void)t;
  (void)f;
  (void)n;

  char* i = d;

  if (i == NULL)
    i = "symbol";

  if (*i == ':') {
    assert(strlen(i) > 1);
    i++;
    f |= LITERALP;
  }

  Symbol* s = p;
  s->idno   = ++RlVm.toplevel.nSymbols;
  s->name   = duplicate(NULL, i, strlen(i)+1);
}

void* cloneSymbol(void* p, int f) {
  /* a cloned symbol is always a gensym */
  (void)f;

  Symbol* out  = duplicate(&RlVm, p, sizeof(Symbol));
  Symbol* orig = p;
  out->name    = duplicate(NULL, orig->name, strlen(orig->name)+1);
  out->idno    = ++RlVm.toplevel.nSymbols;
  out->special = NULL;                                              // can't duplicate special forms
  
  delFl(out, INTERNEDP);

  return out;
}



Symbol* newSymbol(char* name, bool interned) {
  if (name == NULL || *name == '\0')
    name = "symbol";

  return newObj(&SymbolType, interned * INTERNEDP, strlen(name), name);
}

Symbol* symbol(char* token) {
  Symbol* out;

  symbolCacheAdd(&RlVm.toplevel.symbols, token, &out);

  return out;
}

Symbol* gensym(char* name) {
  return newSymbol(name, false);
}

// Function type
void initFunction(void* p, Type* t, int fl, size_t n, void* data) {
  (void)t;
  (void)fl;
  (void)n;
  
  Function* fn = p;

  if (data) {
    struct { Symbol* name; Obj* ini; }* init = data;

    fn->name = init->name;

    if (init->ini) {
      if (init->ini->type == &MethodTableType)
        fn->methods = (MethodTable*)init->ini;

      else
        fn->singleton = init->ini;
    }
  }
}

void* cloneFunction(void* p, int fl) {
  Function* f = duplicate(&RlVm, p, sizeof(Function));

  // the method table needs to be cloned if this is a deep copy
  if (!!(fl & DEEP) && f->methods != NULL) {
    size_t nSaved = save(1, tag(f));
    f->methods = clone(f->methods, fl);
    unsave(nSaved);
  }

  return f;
}

Function* newFunction(Symbol* name, Obj* ini, bool generic, bool macro) {
  int flags = generic*GENERICP | macro*MACROP;
  size_t nSaved = save(2, tag(name), tag(ini));
  struct { Symbol* name; Obj* ini; } init = { name, ini };
  Function* out = newObj(&FunctionType, flags, 0, &init);
  unsave(nSaved);
  return out;
}

// vector leaf
void initVecLeaf(void* p, Type* t, int f, size_t n, void* d) {
  (void)t;
  (void)f;
  assert(n == );
  assert(d != NULL);

  VecLeaf* l = p;
  memcpy(l->slots, d, n * sizeof(Value));
}

VecLeaf* newVecLeaf(Value* tail) {
  return newObj(&VecLeafType, 0, 64, tail);
}

// vector node
void initVecNode(void* p, Type* t, int f, size_t n, void* d) {
  assert(d != NULL);
  
  VecNode* vn = p;
  struct { Obj** children; size_t shift; } *ini = d;

  if (!!(f & EDITP)) {
    vn->capacity = 
  }
}

// collection interfaces
size_t getNodeIndex(uint32_t shift, uint64_t key) {
  return key >> shift & INDEX_MASK;
}

// map interface
bool mapNodeHasIndex(uint64_t bitMap, uint32_t shift, uint64_t hash) {
  size_t index = getNodeIndex(shift, hash);

  return !!(bitMap & (1 << index));
}

size_t getMapNodeIndex(uint64_t bitMap, uint64_t shift, uint64_t hash) {
  size_t index = getNodeIndex(shift, hash);

  return popc(bitMap & ((1 << index) - 1));
}

size_t rootSize(Vector* v) {
  size_t out;
  
  if (v->arity < BRANCH_FACTOR)
    out = 0;

  else if ((v->arity & INDEX_MASK) == 0)
    out = (v->arity & ~INDEX_MASK) - BRANCH_FACTOR;

  else
    out = v->arity & ~INDEX_MASK;

  return out;
}

size_t tailSize(Vector* v) {
  size_t out;

  if (v->arity < BRANCH_FACTOR)
    out = v->arity;

  else if ((v->arity & INDEX_MASK) == 0)
    out = BRANCH_FACTOR;

  else
    out = v->arity & INDEX_MASK;

  return out;
}

bool tailHasSpace(Vector* v) {
  return tailSize(v) < BRANCH_FACTOR;
}

bool rootHasSpace(Vector* v) {
  return v->arity < (1ul << v->shift);
}

// helpers for freezing and unfreezing nodes, which you have to do a lot
// with these fucking things
void freezeVecNode(VecNode* n) {
  if (n && getFl(n, EDITP, 0)) {
    delFl(n, EDITP);

    if (n->capacity > n->count) {
      n->children = reallocate(NULL, n->children, 0, n->count*sizeof(Obj*));
      n->count    = n->capacity;
    }

    if (n->shift > LEVEL_SHIFT)
      for (size_t i=0; i<n->count; i++)
        freezeVecNode((VecNode*)n->children[i]);
  }
}

void freezeVector(Vector* v) {
  if (getFl(v, EDITP, 0)) {
    delFl(v, EDITP);
    size_t ts = tailSize(v);

    if (ts < BRANCH_FACTOR)
      v->tail = reallocate(NULL, v->tail, 0, ts*sizeof(Value));

    freezeVecNode(v->root);
  }
}

void freezeMapNode(MapNode* n) {
  if (n && getFl(n, EDITP, 0)) {
    delFl(n, EDITP);
 
    if (n->capacity > n->count) {
      n->children = reallocate(NULL, n->children, 0, n->count*sizeof(Obj*));
      n->count     = n->capacity;
    }

    for (size_t i=0; i<n->capacity; i++) {
      if (n->children[i]->type == &MapNodeType)
        freezeMapNode((MapNode*)n->children[i]);
    }
  }
}

void freezeMap(Map* m) {
  if (getFl(m, EDITP, 0)) {
    delFl(m, EDITP);
    freezeMapNode(m->root);
  }
}

Vector* unfreezeVector(Vector* v) {
  if (!getFl(v, EDITP, 0))
    v = clone(v, EDITP);

  return v;
}

VecNode* unfreezeVecNode(VecNode* n) {
  if (!getFl(n, EDITP, 0))
    n = clone(n, EDITP);

  return n;
}

Map* unfreezeMap(Map* m) {
  if (!getFl(m, EDITP, 0))
    m = clone(m, EDITP);

  return m;
}

MapNode* unfreezeMapNode(MapNode* n) {
  if (!getFl(n, EDITP, 0))
    n = clone(n, EDITP);

  return n;
}

#define unfreeze(o)                             \
  _Generic((o),                                 \
           Vector*:unfreezeVector,              \
           VecNode*:unfreezeVecNode,            \
           Map*:unfreezeMap,                    \
           MapNode*:unfreezeMapNode)(o)

#define freeze(o)                               \
  _Generic((o),                                 \
           Vector*:freezeVector,                \
           VecNode*:freezeVecNode,              \
           Map*:freezeMap,                      \
           MapNode*:freezeMapNode)(o)

Value mapGet(Map* m, Value k) {
  uint64_t h = hash(k);
  MapNode* n = m->root;
  Value out = NOTHING;

  while (n) {
    uint32_t sh = n->shift;
    uint64_t bm = n->bitmap;

    if (!mapNodeHasIndex(bm, sh, h))
      break;
    
    size_t tidx  = getMapNodeIndex(bm, sh, h);
    Obj*   child = n->children[tidx];
    
    if (child->type == &MapNodeType)
      n = (MapNode*)child;
    
    else {
      for (MapLeaf* l =(MapLeaf*)child;l != NULL;l=l->next) {
        if (equal(k, l->key)) {
          out = l->val;
          break;
        }
      }
      break;
    }
  }

  return out;
}

// vector interface
Value vecGet(Vector* v, size_t i) {
  Value out = NOTHING;

  if (i < v->arity) {
    if (i > rootSize(v))
      out = v->tail[getNodeIndex(0, i)];

    else {
      VecNode* n = v->root;
      VecLeaf* l = NULL;

      while (l == NULL) {
        uint16_t sh = n->shift;
        uint16_t j  = getNodeIndex(sh, i);
        Obj* o      = n->children[j];

        if (sh == 6)
          l = (VecLeaf*)o;

        else
          n = (VecNode*)o;
      }

      out = l->slots[getNodeIndex(0, i)];
    }
  }

  return out;
}

static void pushTail(Vector* v) {
  // note: vector already saved
  if (rootHasSpace(v)) {
    
  } else {
    VecNode* node;
    
    for (size_t i=v->shift; i>0; i-=LEVEL_SHIFT) {
      
    }
  }
}

Vector* vecAdd(Vector* v, Value x) {
  if (getFl(v, EDITP, 0)) {
    if (!tailHasSpace(v)) {
      size_t nSaved = save(2, x, tag(v));
      pushTail(v);
      unsave(nSaved);
    }
    size_t a = v->arity;
    size_t i = getNodeIndex(0, a);

    v->tail[i] = x;
    v->arity   = a+1;
  } else {
    save(1, x);
    v = unfreeze(v);
    unsave(1);
    v = vecAdd(v, x);
    freeze(v);
  }

  return v;
}
