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

void* newObj(Type* t, int f, size_t e) {
  void* o;
  save(1, tag(t));
  o = allocate(&RlVm, t->vTable->objSize+e);
  initObj(o, t, f);
  unsave(1);
  return o;
}

void* cloneObj(void* p) {
  Obj* o;
  assert(p != NULL);
  save(1, tag(p));
  o = duplicate(&RlVm, p, sizeOf(p));
  addToHeap(o);
  unsave(1);
  return o;
}

void initObj(void* p, Type* t, int f) {
  Obj* o   = p;
  o->type  = t;
  o->annot = &emptyMap;
  o->flags = f;
  o->gray  = true;

  addToHeap(o);
}

// collection interfaces
// bits interface
size_t getElSize(Bits* b) {
  return b->obj.flags & 0x7;
}

bool fitsElSize(Bits* b, int i) {
  size_t es = getElSize(b);
  bool out;

  if (es == 1)
    out = i <= INT8_MAX && i >= INT8_MIN;

  else if (es == 2)
    out = i <= INT16_MAX && i >= INT16_MIN;

  else
    out = true;

  return out;
}

Bits* cloneBits(Bits* b) {
  Bits* o   = cloneObj(b);
  size_t es = getElSize(o);
  o->data   = duplicate(NULL, b->data, b->arity * es);

  return o;
}

Value bitsGet(Bits* b, size_t n) {
  assert(n < b->arity);

  size_t es = getElSize(b);
  int o;

  memcpy(&o, b->data+n*es, es);

  return tag(o);
}

Bits* bitsAdd(Bits* b, int i) {
  assert(fitsElSize(b, i));
  save(1, tag(b));
  size_t es = getElSize(b);
  Bits* o = newBits(NULL, b->arity+1, es);
  memcpy(o->data, b->data, es*b->arity);
  memcpy(o->data+es*b->arity, &i, es);
  unsave(1);
  return o;
}

Bits* bitsSet(Bits* b, size_t n, int i) {
  assert(n < b->arity);
  assert(fitsElSize(b, i));
  size_t es = getElSize(b);
  Bits* o = cloneBits(b);
  memcpy(o->data+n*es, &i, es);
  return o;
}

Bits* bitsDel(Bits* b, size_t n) {
  assert(n < b->arity);
  save(1, tag(b));
  size_t es = getElSize(b);
  Bits* o = newBits(NULL, b->arity-1, es);
  memcpy(o->data, b->data, (n-1)*es);
  memcpy(o->data, b->data+n*es, (b->arity-n)*es);
  unsave(1);
  return o;
}

// string interface
String emptyString = {
  .obj={
    .type   =&StringType,
    .annot  =&emptyMap,
    .noSweep=true,
    .noFree =true,
    .gray   =true
  },
  .data ="",
  .arity=0
};

Encoding getEncoding(String* s) {
  assert(s != NULL);
  return s->obj.flags & 0x7;
}

String* cloneString(String* s) {
  if (s->arity == 0)
    return s;

  String* o = cloneObj(s);
  o->data   = duplicates(NULL, s->data, s->arity);

  return o;
}

Value strGet(String* s, size_t n) {
  assert(n < s->arity);
  Glyph g = s->data[n];

  return tag(g);
}

String* strAdd(String* s, Glyph g) {
  save(1, tag(s));
  Encoding e = getEncoding(s);
  String* o = newString(NULL, s->arity+1, e);
  o->data[s->arity] = g;
  memcpy(o->data, s->data, s->arity);
  unsave(1);
  return o;
}

String* strSet(String* s, size_t n, Glyph g) {
  assert(n < s->arity);
  save(1, tag(s));
  String* o = cloneString(s);
  o->data[n] = g;
  unsave(1);
  return o;
}

String* strDel(String* s, size_t n) {
  
}

// tuple interface
Tuple* cloneTuple(Tuple* t) {
  Tuple* o = t;
  
  if (t->arity > 0) // don't clone empty tuple
    o = cloneObj(t);

  return o;
}

// vector & map interfaces
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
  return v->arity < (1ul << getShift(v));
}

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
      save(2, x, tag(v));
      pushTail(v);
      unsave(2);
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
