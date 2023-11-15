#include "util/number.h"
#include "util/collection.h"
#include "util/hashing.h"

#include "lang/equal.h"

#include "vm/memory.h"

#include "val/seq.h"
#include "val/symbol.h"
#include "val/func.h"
#include "val/type.h"
#include "val/table.h"

/* Globals */
/* Magic numbers */
#define LOADF 0.625

/* Table type */
extern void  trace_mdict(void* obj);
extern void  finalize_mdict(void* obj);
extern void* clone_mdict(void* obj);

INIT_OBJECT_TYPE(MutDict,
                 .tracefn=trace_mdict,
                 .finalizefn=finalize_mdict,
                 .clonefn=clone_mdict);

/* MutSet type */
extern void  trace_mset(void* obj);
extern void  finalize_mset(void* obj);
extern void* clone_mset(void* obj);

INIT_OBJECT_TYPE(MutSet,
                 .tracefn=trace_mset,
                 .finalizefn=finalize_mset,
                 .clonefn=clone_mset);

/* Dict type */
extern void   trace_dict(void* obj);
extern void   finalize_dict(void* obj);
extern void*  clone_dict(void* obj);
extern hash_t hash_dict(Value x);
extern bool   equal_dicts(Value x, Value y);
extern int    order_dicts(Value x, Value y);

INIT_OBJECT_TYPE(Dict,
                 .tracefn=trace_dict,
                 .finalizefn=finalize_dict,
                 .clonefn=clone_dict,
                 .hashfn=hash_dict,
                 .egalfn=equal_dicts,
                 .ordfn=order_dicts);

/* DictNode type */
extern void   trace_dict_node(void* obj);
extern void   finalize_dict_node(void* obj);
extern void*  clone_dict_node(void* obj);

INIT_OBJECT_TYPE(DictNode,
                 .tracefn=trace_dict_node,
                 .finalizefn=finalize_dict_node,
                 .clonefn=clone_dict_node);

/* Dict leaf type */
extern void  trace_dict_leaf(void* obj);
extern void  finalize_dict_leaf(void* obj);

INIT_OBJECT_TYPE(DictLeaf,
                 .tracefn=trace_dict_leaf,
                 .finalizefn=finalize_dict_leaf);

/* Set type */
extern void   trace_set(void* obj);
extern void   finalize_set(void* obj);
extern void*  clone_set(void* obj);
extern hash_t hash_set(Value x);
extern bool   equal_sets(Value x, Value y);
extern int    order_sets(Value x, Value y);

INIT_OBJECT_TYPE(Set,
                 .tracefn=trace_set,
                 .finalizefn=finalize_set,
                 .clonefn=clone_set,
                 .hashfn=hash_set,
                 .egalfn=equal_sets,
                 .ordfn=order_sets);

/* Set node type */
extern void  trace_set_node(void* obj);
extern void  finalize_set_node(void* obj);
extern void* clone_set_node(void* obj);

INIT_OBJECT_TYPE(SetNode,
                 .tracefn=trace_set_node,
                 .finalizefn=finalize_set_node,
                 .clonefn=clone_set_node);

/* Set leaf type */
extern void trace_set_leaf(void* obj);
extern void finalize_set_leaf(void* obj);

INIT_OBJECT_TYPE(SetLeaf,
                 .tracefn=trace_set_leaf,
                 .finalizefn=finalize_set_leaf);

/* Empty singletons */
Dict EmptyDict = {
  .obj={
    .type =&DictType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|NOFREE|NOTRACE|GRAY,
  },
  .root =NULL,
  .arity=0,
};

Set EmptySet = {
  .obj={
    .type =&SetType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|NOFREE|NOTRACE|GRAY,
  },
  .root =NULL,
  .arity=0,
};

/* Internal APIs */
static Entry* find_mdict_entry(Entry* kvs, bool fast, size_t c, Value k) {
  HashFn hasher;
  EgalFn cmpr;
  hash_t h;
  size_t i, m;
  Entry* ts;

  hasher = fast ? hash_word : hash;
  cmpr = fast ? same : equal;
  h = hasher(k);
  m = c - 1;
  i = h & m;
  ts = NULL;

  for (;;) {
      Entry* kv = &kvs[i];

      if (kv->key == NOTHING)
        return ts ? ts : kv;
      else if (kv->key == TOMBSTONE)
        ts = ts ? : kv;
      else if (cmpr(kv->key, k))
        return kv;
      else
        i = (i + 1) & m;
  }
}

static Value* find_mset_entry(Value* kvs, bool fast, size_t c, Value k) {
  HashFn hasher;
  EgalFn cmpr;
  hash_t h;
  size_t i, m;
  Value* ts;

  hasher = fast ? hash_word : hash;
  cmpr = fast ? same : equal;
  h = hasher(k);
  m = c - 1;
  i = h & m;
  ts = NULL;

  for (;;) {
      Value* kv = &kvs[i];

      if (*kv == NOTHING)
        return ts ? ts : kv;
      else if (*kv == TOMBSTONE)
        ts = ts ? : kv;
      else if (cmpr(*kv, k))
        return kv;
      else
        i = (i + 1) & m;
  }
}

static DictLeaf* new_dict_leaf(Value key, Value val, DictLeaf* next) {
  save(3, key, val, tag(next));

  DictLeaf* out = new_obj(&DictLeafType, 0, 0, 0);

  out->key  = key;
  out->val  = val;
  out->next = next;

  return out;
}

static DictNode* new_dict_node(bool editp, size_t sh) {
  DictNode* out = new_obj(&DictNodeType, 0, editp*EDITP, 0);

  init_hamt(out, (void***)&out->children, NULL, 0, sh);
  out->bitmap = 0;

  return out;
}

static DictLeaf* find_dict_leaf(DictNode* n, Value k) {
  DictLeaf* out = NULL;

  if (n != NULL) {
    hash_t h = hash(k);

    for (;out == NULL;) {
      size_t shift  = get_hamt_shift(n);
      size_t bitmap = n->bitmap;
      int index = hamt_hash_to_index(h, shift, bitmap);

      if (index < 0)
        break;

      Obj* child = n->children[index];

      if (child->type == &DictNodeType)
        n = (DictNode*)child;
      else {
        DictLeaf* leaf = (DictLeaf*)child;

        for (;out == NULL && leaf != NULL; leaf=leaf->next)
          if (equal(k, leaf->key))
            out = leaf;
      }
    }
  }

  return out;
}

static void add_dict_leaf_to_node(DictNode* n, DictLeaf* l, hash_t h) {
  if (is_nosweep(l) || l->next != NULL) {
    save(1, tag(n));
    l = clone_obj(l);
    l->next = NULL;
  }
  
  hamt_add_to_bitmap(n, (void***)&n->children, l, &n->bitmap, h);
}

static void* resolve_collision(DictLeaf* lorig, hash_t hnew, DictLeaf* lnew) {
  hash_t horig = hash(lorig->key);

  if ((horig & VAL_MASK) == (hnew & VAL_MASK)) { // true collision
    return new_dict_leaf(lnew->key, lnew->val, lorig);
  } else { // find the level where they split and create a new node to hold the two leaves
    
  }
}

static DictNode* add_dict_leaf(DictNode* n, bool editp, DictLeaf* l, hash_t h) {
  /* first key */
  if (n == NULL) {
    n = new_dict_node(editp, HAMT_MAX_SHIFT);
    save(1, tag(n));
    add_dict_leaf_to_node(n, l, h);
  } else {
    if (!is_editp(n))
      n = unfreeze(n);
      
    save(1, tag(n));

    
    
    if (!editp)
      freeze(n);
  }

  return n;
}

static SetLeaf* find_set_leaf(SetNode* n, bool fast, Value v) {
  SetLeaf* out = NULL;

  if (n != NULL) {
    HashFn hasher = fast ? hash_word : hash;
    EgalFn cmper = fast ? same : equal;
    hash_t hash = hasher(v);
    
    for (;out == NULL;) {
      size_t shift  = get_hamt_shift(n);
      size_t bitmap = n->bitmap;
      int index = hamt_hash_to_index(hash, shift, bitmap);

      if (index < 0)
        break;

      Obj* child = n->children[index];

      if (child->type == &SetNodeType)
        n = (SetNode*)child;
      else {
        SetLeaf* leaf = (SetLeaf*)child;

        for (;out == NULL && leaf != NULL; leaf=leaf->next)
          if (cmper(v, leaf->val))
            out = leaf;
      }
    }
  }

  return out;
}

/* External APIs */
/* MutSet API */
size_t mdict_arity(MutDict* slf) {
  return slf->cnt - slf->nts;
}

MutDict* new_mdict(bool fast) {
  MutDict* out;

  out = (MutDict*)new_obj(&MutDictType, fast, 0, 0);
  init_mdict(out);

  return out;
}

void init_mdict(MutDict* slf) {
  slf->data = NULL;
  slf->cnt  = 0;
  slf->cap  = 0;
  slf->nts  = 0;
}

void free_mdict(MutDict* slf) {
  deallocate(NULL, slf->data, 0);
  init_mdict(slf);
}

void resize_mdict(MutDict* slf, size_t new_cnt) {
  if (new_cnt == 0)
    free_mdict(slf);

  else {
    size_t old_cnt, old_cap, new_cap;

    old_cnt = slf->cnt;
    old_cap = slf->cap;
    new_cap = pad_table_size(old_cnt, new_cnt, old_cap, LOADF);

    if (new_cap != old_cap) {
      Entry* kvs;

      kvs = allocate(NULL, new_cap * sizeof(Entry));

      /* initialize new kvs */
      for (size_t i=0; i<new_cap; i++)
        kvs[i] = (Entry) { NOTHING, NOTHING };

      /* rehash */
      if (slf->data != NULL) {
        bool fast;
        Entry* src, * dst;

        fast = is_fasthash(slf);
        slf->cnt = 0;
        slf->nts = 0;

        for (size_t i=0; i<old_cap; i++) {
          src = &slf->data[i];

          if (src->key == NOTHING || src->key == TOMBSTONE)
            continue;

          dst = find_mdict_entry(kvs, fast, new_cap, src->key);
          dst->key = src->key;
          dst->val = src->val;
          slf->cnt++;
        }

        deallocate(NULL, slf->data, 0);
      }

      slf->data = kvs;
      slf->cap  = new_cap;
    }
  }
}

Entry* mdict_find(MutDict* slf, Value key) {
  bool fast = is_fasthash(slf);
  size_t cap = slf->cap;
  Entry* out = find_mdict_entry(slf->data, fast, cap, key);

  return out;
}

Entry* mdict_intern(MutDict* slf, Value key) {
  resize_mdict(slf, slf->cnt+1);

  Entry* out = mdict_find(slf, key);

  if (out->key == NOTHING) {
    out->key = key;
    slf->cnt++;
  } else if (out->key == TOMBSTONE) {
    out->key = key;
  }

  return out;
}

Value mdict_get(MutDict* slf, Value key) {
  Entry* out = mdict_find(slf, key);

  return out->val;
}

bool mdict_has(MutDict* slf, Value key) {
  Entry* out = mdict_find(slf, key);

  return out->key != NOTHING && out->key != TOMBSTONE;
}

bool mdict_set(MutDict* slf, Value key, Value val) {
  resize_mdict(slf, slf->cnt+1);

  Entry* loc = mdict_find(slf, key);

  bool out;

  if (loc->key == NOTHING) {
    out = true;
    loc->key = key;
    slf->cnt++;
  } else if (loc->key == TOMBSTONE) {
    out = true;
    loc->key = key;
    slf->nts--;
  } else {
    out = false;
  }

  loc->val = val;
  return out;
}

bool mdict_del(MutDict* slf, Value key) {
  Entry* loc = mdict_find(slf, key);

  bool out;

  if (loc->key == NOTHING || loc->key == TOMBSTONE)
    out = false;

  else {
    out = true;
    loc->key = TOMBSTONE;
    loc->val = NOTHING;
    slf->nts++;
  }

  return out;
}

void join_mdicts(MutDict* slf, MutDict* other) {
  Entry* okvs;
  size_t ocap;

  okvs = other->data;
  ocap = other->cap;

  if (mdict_arity(other) > 0) {
    for (size_t i=0; i<ocap; i++) {
      Entry* okv = &okvs[i];
      
      if (okv->key == NOTHING || okv->key == TOMBSTONE)
        continue;
      
      mdict_set(slf, okv->key, okv->val);
    }
  }
}

/* MutSet API */
size_t mset_arity(MutSet* slf) {
  return slf->cnt - slf->nts;
}

MutSet* new_mset(bool fast) {
  MutSet* out;

  out = new_obj(&MutSetType, fast, 0, 0);
  init_mset(out);

  return out;  
}

void init_mset(MutSet* slf) {
  slf->data = NULL;
  slf->cnt  = 0;
  slf->cap  = 0;
  slf->nts  = 0;
}

void free_mset(MutSet* slf) {
  deallocate(NULL, slf->data, 0);
  init_mset(slf);
}

void resize_mset(MutSet* slf, size_t new_cnt) {
  if (new_cnt == 0)
    free_mset(slf);

  else {
    size_t old_cnt, old_cap, new_cap;

    old_cnt = slf->cnt;
    old_cap = slf->cap;
    new_cap = pad_table_size(old_cnt, new_cnt, old_cap, LOADF);

    if (new_cap != old_cap) {
      Value* kvs;

      kvs = allocate(NULL, new_cap * sizeof(Value));

      /* initialize new kvs */
      for (size_t i=0; i<new_cap; i++)
        kvs[i] = NOTHING;

      /* rehash */
      if (slf->data != NULL) {
        bool fast;

        Value* src, * dst;

        fast = is_fasthash(slf);
        slf->cnt = 0;
        slf->nts = 0;

        for (size_t i=0; i<old_cap; i++) {
          src = &slf->data[i];

          if (*src == NOTHING || *src == TOMBSTONE)
            continue;

          dst = find_mset_entry(kvs, fast, new_cap, *src);
          *dst = *src;
          slf->cnt++;
        }

        deallocate(NULL, slf->data, 0);
      }

      slf->data = kvs;
      slf->cap = new_cap;
    }
  }
}

Value*  mset_find(MutSet* slf, Value key) {
  bool fast = is_fasthash(slf);
  size_t cap = slf->cap;
  Value* out = find_mset_entry(slf->data, fast, cap, key);

  return out;
}

bool mset_has(MutSet* slf, Value val) {
  Value* loc = mset_find(slf, val);

  return *loc != NOTHING && *loc != TOMBSTONE;
}

bool mset_add(MutSet* slf, Value val) {
  Value* loc = mset_find(slf, val);
  bool out;

  if (*loc == NOTHING) {
    out = true;
    *loc = val;
    slf->cnt++;
  } else if (*loc == TOMBSTONE) {
    out = true;
    *loc = val;
    slf->nts--;
  } else {
    out = false;
  }

  return out;
}

bool mset_del(MutSet* slf, Value val) {
  Value* loc = mset_find(slf, val);

  bool out;

  if (*loc == NOTHING || *loc == TOMBSTONE)
    out = false;

  else {
    out = true;
    *loc = TOMBSTONE;
    slf->nts++;
  }

  return out;
}

void join_msets(MutSet* slf, MutSet* other) {
  Value* okvs = other->data;
  size_t ocap = other->cap;

  if (mset_arity(other) > 0) {
    for (size_t i=0; i<ocap; i++) {
      Value v = okvs[i];

      if (v == NOTHING || v == TOMBSTONE)
        continue;

      mset_add(slf, v);
    }
  }
}

/* Dict API */
Value dict_get(Dict* d, Value k) {
  DictLeaf* l = find_dict_leaf(d->root, k);

  if (l == NULL)
    return NOTHING;

  return l->val;
}

/* Dict HAMT apis */
Dict* freeze_dict(Dict* d) {
  if (del_mfl(d, EDITP))
    freeze_dict_node(d->root);

  return d;
}

Dict* unfreeze_dict(Dict* d) {
  if (!is_editp(d)) {
    d = clone_obj(d);
    set_mfl(d, EDITP);
  }

  return d;
}

DictNode* freeze_dict_node(DictNode* n) {
  if (del_mfl(n, EDITP)) {
    size_t cnt = get_hamt_cnt(n);

    for (size_t i=0; i<cnt; i++) {
      Obj* child = n->children[i];

      if (is_editp(child)) {
        if (child->type == &DictLeafType)
          freeze_dict_leaf((DictLeaf*)child);
        else
          freeze_dict_node((DictNode*)child);
      }
    }

    freeze_hamt(n);
  }

  return n;
}

DictNode* unfreeze_dict_node(DictNode* n) {
  if (!is_editp(n)) {
    n = clone_obj(n);
    set_mfl(n, EDITP);
  }

  return n;
}

DictLeaf* freeze_dict_leaf(DictLeaf* l) {
  if (del_mfl(l, EDITP)) {
    DictLeaf* n = l->next;

    while (n != NULL) {
      del_mfl(l, EDITP);
      n = n->next;
    }
  }

  return l;
}

DictLeaf* unfreeze_dict_leaf(DictLeaf* l) {
  if (!is_editp(l)) {
    l = clone_obj(l);
    set_mfl(l, EDITP);
  }

  return l;
}

/* Set API */
bool set_has(Set* s, Value k) {
  SetLeaf* l = find_set_leaf(s->root, is_fasthash(s), k);
  bool out = l != NULL;

  return out;
}
