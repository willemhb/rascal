#include "util/hashing.h"

#include "lang/equal.h"

#include "vm/memory.h"

#include "val/func.h"
#include "val/symbol.h"
#include "val/type.h"
#include "val/map.h"
#include "val/seq.h"

/* Globals */
/* empty singletons */
Dict EmptyDict = {
  .obj={
    .type =&DictType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|NOFREE|GRAY,
  },
  .root =NULL,
  .arity=0,
};

Set EmptySet = {
  .obj={
    .type =&SetType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|NOFREE|GRAY,
  },
  .root =NULL,
  .arity=0,
};

/* type objects */
extern void trace_set(void* obj);
extern void finalize_set(void* obj);

INIT_OBJECT_TYPE(Set, NULL, trace_set, finalize_set);

extern void trace_set_node(void* obj);
extern void finalize_set_node(void* obj);

INIT_OBJECT_TYPE(SetNode, NULL, trace_set_node, finalize_set_node);

extern void trace_set_leaf(void* obj);
extern void finalize_set_leaf(void* obj);

INIT_OBJECT_TYPE(SetLeaf, NULL, trace_set_leaf, finalize_set_leaf);

extern void trace_dict(void* obj);
extern void finalize_dict(void* obj);

INIT_OBJECT_TYPE(Dict, NULL, trace_dict, finalize_dict);

extern void trace_dict_node(void* obj);
extern void finalize_dict_node(void* obj);

INIT_OBJECT_TYPE(DictNode, NULL, trace_dict_node, finalize_dict_node);

extern void trace_dict_leaf(void* obj);
extern void finalize_dict_leaf(void* obj);

INIT_OBJECT_TYPE(DictLeaf, NULL, trace_dict_leaf, finalize_dict_leaf);

/* Internal APIs */
static void add_dict_leaf(Dict* d, DictLeaf* l);
static void add_set_leaf(Set* s, SetLeaf* l);

/* External APIs */
/* Dict API */
static Dict* add_to_dict(Dict* d, Value key, )
  
Value dict_get(Dict* d, Value k) {
  Value out = NOTHING;

  if (d->arity > 0) {
    bool fast = is_fasthash(d);
    HashFn hasher = fast ? hash_word : hash;
    EgalFn cmper = fast ? same : equal;
    hash_t hash = hasher(k);
    DictNode* node = d->root;
    
    for (;;) {
      size_t shift  = get_hamt_shift(node);
      size_t bitmap = node->bitmap;
      int index = hamt_hash_to_index(hash, shift, bitmap);

      if (index > -1) {
        Obj* child = node->data[index];

        if (child->type == &DictNodeType) {
          node = (DictNode*)child;
        } else {
          DictLeaf* leaf = (DictLeaf*)child;

          for (;out == NOTHING && leaf != NULL; leaf=leaf->next) {
            if (cmper(k, leaf->key))
              out = leaf->val;
          }

          break;
        }
      } else {
        break;
      }
    }
  }

  return out;
}

Dict* join_dicts(Dict* dx, Dict* dy) {
  Dict* out;

  if (dx->arity == 0)
    out = dy;

  else if (dy->arity == 0)
    out = dx;

  else {
    out = unfreeze(dx);
    DictSeq* seq = mk_seq(dy, true);
    save(2, tag(out), tag(seq));

    for (Value x=first(seq); seq != NULL; seq=rest(seq), x=first(seq))
      add_dict_leaf(out, as_dict_leaf(x));
  }

  freeze(out);

  return out;
}

/* Set API */
bool set_has(Set* s, Value k) {
  bool out = false;
  
  if (s->arity > 0) {  
    bool fast = is_fasthash(s);
    HashFn hasher = fast ? hash_word : hash;
    EgalFn cmper = fast ? same : equal;
    hash_t hash = hasher(k);
    SetNode* node = s->root;

    for (;;) {
      size_t shift  = get_hamt_shift(node);
      size_t bitmap = node->bitmap;
      int index = hamt_hash_to_index(hash, shift, bitmap);

      if (index > -1) {
        Obj* child = node->data[index];

        if (child->type == &SetNodeType) {
          node = (SetNode*)child;
        } else {
          SetLeaf* leaf = (SetLeaf*)child;

          for (;!out && leaf != NULL; leaf=leaf->next)
            out = cmper(k, leaf->val);

          break;
        }
      } else {
        break;
      }
    }
  }

  return out;
}

Set* join_sets(Set* sx, Set* sy) {
  Set* out;

  if (sx->arity == 0)
    out = sy;

  else if (sy->arity == 0)
    out = sx;

  else {
    save(2, tag(sx), tag(sy));
    out = unfreeze(sx);
    add_saved(0, tag(out));
    SetSeq* seq = mk_seq(sy, true);

    for (Value x=first(seq); seq != NULL; seq=rest(seq), x=first(seq))
      add_set_leaf(out, as_set_leaf(x));
  }

  freeze(out);

  return out;
}
