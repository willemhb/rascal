#ifndef rl_val_map_h
#define rl_val_map_h

#include "val/object.h"

/* Clojure-like persistent unordered mapping types. */
/* C types */
typedef struct DictNode DictNode;
typedef struct DictLeaf DictLeaf;
typedef struct SetNode  SetNode;
typedef struct SetLeaf  SetLeaf;

struct DictNode {
  HEADER;
  Obj**   data;
  size_t  bitmap;
};

struct DictLeaf {
  HEADER;
  DictLeaf* next;   // used for collision resolution
  Value     key;
  Value     val;
};

struct Dict {
  HEADER;
  DictNode* root;
  size_t    arity;
};

struct SetNode {
  HEADER;
  Obj**  data;
  size_t bitmap;
};

struct SetLeaf {
  HEADER;
  SetLeaf* next; // used for collision resolution
  Value    val;
};

/* globals */
extern Dict EmptyDict;
extern Set  EmptySet;
extern Type DictType, DictNodeType, DictLeafType, SetType, SetNodeType, SetLeafType;

/* external API */
Dict* new_dict(void);
Dict* mk_dict(size_t n, Value* kvs);
Dict* freeze_dict(Dict* d);
Value dict_ref(Dict* d, Value k);
Dict* dict_set(Dict* d, Value k, Value v);
Dict* dict_del(Dict* d, Value k);
Dict* join_dicts(Dict* dx, Dict* dy);

Set*  new_set(void);
Set*  mk_set(size_t n, Value* ks);
Set*  freeze_set(Set* s);
bool  set_has(Set* s, Value k);
Set*  set_add(Set* s, Value k);
Set*  set_del(Set* s, Value k);
Set*  join_sets(Set* sx, Set* sy);

#endif
