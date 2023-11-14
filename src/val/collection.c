#include "runtime.h"

#include "collection.h"


// collection interfaces
// bits interface
Bits EmptyBits = {
  .obj={
    .type    =&BitsType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .gray    =true,
  },
  .data =NULL,
  .arity=0
};

Bits* clone_bits(Bits* b) {
  if (b->arity == 0)
    return b;

  save(1, tag(b));

  Bits* o = clone_obj(b);
  o->data = duplicate(NULL, b->data, b->arity);

  return o;
}

Value bits_get(Bits* b, size_t n) {
  assert(n < b->arity);

  return tag((Small)b->data[n]);
}

Bits* bits_add(Bits* b, byte_t x) {
  save(1, tag(b));

  Bits* o = new_bits(NULL, b->arity+1);
  memcpy(o->data, b->data, b->arity);
  o->data[o->arity-1] = x;

  return o;
}

Bits* bits_set(Bits* b, size_t n, byte_t x) {
  assert(n < b->arity);

  Bits* o = clone_bits(b);
  o->data[n] = x;

  return o;
}

Bits* bits_del(Bits* b, size_t n) {
  assert(n < b->arity);

  save(1, tag(b));

  Bits* o = new_bits(NULL, b->arity-1);
  memcpy(o->data, b->data, n-1);
  memcpy(o->data+n-1, b->data+n, b->arity-n);

  return o;
}

// string interface
String EmptyString = {
  .obj={
    .type    =&StringType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
    .flags   =ASCII,
  },
  .data ="",
  .arity=0,
};

Encoding get_encoding(String* s) {
  assert(s != NULL);
  return s->obj.flags & 0x7;
}

String* clone_str(String* s) {
  if (s->arity == 0)
    return s;

  String* o = clone_obj(s);
  o->data   = duplicates(NULL, s->data, s->arity);

  return o;
}

Value str_get(String* s, size_t n) {
  assert(n < s->arity);

  return tag((Glyph)s->data[n]);
}

String* str_add(String* s, Glyph g) {
  save(1, tag(s));

  String* o = new_str(NULL, s->arity+1);
  memcpy(o->data, s->data, s->arity);
  o->data[s->arity] = g;

  return o;
}

String* str_set(String* s, size_t n, Glyph g) {
  assert(n < s->arity);
  save(1, tag(s));

  String* o = clone_str(s);
  o->data[n] = g;

  return o;
}

String* str_del(String* s, size_t n) {
  assert(n < s->arity);

  save(1, tag(s));

  String* o = new_str(NULL, s->arity-1);
  memcpy(o->data, s->data, n-1);
  memcpy(o->data+n-1, s->data+n, s->arity-n);

  return o;
}

// tuple interface
Tuple EmptyTuple = {
  .obj={
    .type    =&TupleType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .slots=NULL,
  .arity=0
};

Tuple* clone_tuple(Tuple* t) {
  Tuple* o = t;
  
  if (t->arity > 0) { // don't clone empty tuple
    o = clone_obj(t);
    o->slots = duplicate(NULL, t->slots, t->arity * sizeof(Value));
  }

  return o;
}

Value tuple_get(Tuple* t, size_t n) {
  assert(n < t->arity);

  return t->slots[n];
}

Tuple* tuple_add(Tuple* t, Value x) {
  save(1, tag(t));

  Tuple* o = new_tuple(NULL, t->arity+1);
  memcpy(o->slots, t->slots, t->arity*sizeof(Value));
  o->slots[o->arity-1] = x;

  return o;
}

Tuple* tuple_set(Tuple* t, size_t n, Value x) {
  assert(n < t->arity);
  save(1, tag(t));

  Tuple* o = clone_tuple(t);
  o->slots[n] = x;

  return o;
}

Tuple* tuple_del(Tuple* t, size_t n) {
  assert(n < t->arity);
  save(1, tag(t));

  Tuple* o = new_tuple(NULL, n-1);

  if (o->arity > 1) {
    memcpy(o->slots, t->slots, (n-1)*sizeof(Value));
    memcpy(&o->slots[n], &t->slots[n+1], (t->arity-n)*sizeof(Value));
  }

  return t;
}

// list interface
List EmptyList = {
  .obj={
    .type    =&ListType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
  },
  .arity=0,
  .head =NUL,
  .tail =&EmptyList
};

Value list_get(List* l, size_t n) {
  assert(n < l->arity);

  while (n--)
    l = l->tail;

  return l->head;
}

List* list_add(List* l, Value x) {
  return cons(x, l);
}

List* list_set(List* l, size_t n, Value x) {
  assert(n < l->arity);

  
}

// vector & map interfaces
size_t get_node_idx(uint32_t shift, uint64_t key) {
  return key >> shift & INDEX_MASK;
}

// map interface
bool map_node_has_dx(uint64_t bitMap, uint32_t shift, uint64_t hash) {
  size_t index = get_node_idx(shift, hash);

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
