#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/hamt.h"

/* Hashed collection types.

   Dicts and Sets are immutable HAMT-based collections inspired by Clojure.

   MutDicts and MutSets are mutable hashed collections based on common hashing
   techniques.

   TODO: update MutDict and MutSet implementation to use the same algorithm
   as Lua for better lookup and space efficiency, (see "R. P. Brent, Reducing the
   retrieval time of scatter storage techniques, Communications of the
   ACM 16 (1973), 105-109."). */

/* C types */
typedef struct {
  Value key;
  Value val;
} Entry;

struct MutDict {
  HEADER;
  Entry* data;
  size_t cnt;
  size_t cap;
  size_t nts;
};

struct MutSet {
  HEADER;
  Value*  data;
  size_t  cnt;
  size_t  cap;
  size_t  nts;
};

struct DictNode {
  HEADER;
  Obj**   children;
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
  Obj**  children;
  size_t bitmap;
};

struct SetLeaf {
  HEADER;
  SetLeaf* next; // used for collision resolution
  Value    val;
};

struct Set {
  HEADER;
  SetNode* root;
  size_t   arity;
};

/* Globals */
/* Types */
extern Type MutDictType, MutSetType,
  DictType, DictNodeType, DictLeafType,
  SetType, SetNodeType, SetLeafType;

/* Empty singletons */
extern Dict EmptyDict;
extern Set  EmptySet;

/* External APIs */
/* general table API. */
#define GLOBAL_MUT_DICT(Name)                          \
  MutDict Name = {                                     \
    .obj={                                             \
      .type =&MutDictType,                             \
      .meta =&EmptyDict,                               \
      .flags=NOSWEEP|GRAY|EDITP                        \
    },                                                 \
    .data=NULL,                                        \
    .cnt =0,                                           \
    .cap =0,                                           \
    .nts =0                                            \
  }

/* MutDict API */
#define is_mdict(x)    has_type(x, &MutDictType)
#define as_mdict(x)    as(MutDict*, untag48, x)

size_t    mdict_arity(MutDict* slf);
MutDict*  new_mdict(bool fast);
void      init_mdict(MutDict* slf);
void      free_mdict(MutDict* slf);
void      resize_mdict(MutDict* slf, size_t new_cnt);
Entry*    mdict_find(MutDict* slf, Value key);
Entry*    mdict_intern(MutDict* slf, Value key);
Value     mdict_get(MutDict* slf, Value key);
bool      mdict_has(MutDict* slf, Value key);
bool      mdict_set(MutDict* slf, Value key, Value val);
bool      mdict_del(MutDict* slf, Value key);
void      join_mdicts(MutDict* slf, MutDict* other);

/* MutSet API */
#define is_mset(x)     has_type(x, &MutSetType)
#define as_mset(x)     as(MutSet*, untag48, x)

size_t  mset_arity(MutSet* slf);
MutSet* new_mset(bool fast);
void    init_mset(MutSet* slf);
void    free_mset(MutSet* slf);
void    resize_mset(MutSet* slf, size_t new_cnt);
Value*  mset_find(MutSet* slf, Value key);
bool    mset_has(MutSet* slf, Value val);
bool    mset_add(MutSet* slf, Value val);
bool    mset_del(MutSet* slf, Value val);
void    join_msets(MutSet* slf, MutSet* other);

/* Dict API */
#define is_dict(x)      has_type(x, &DictType)
#define as_dict(x)      as(Dict*, untag48, x)
#define is_dict_node(x) has_type(x, &DictNodeType)
#define as_dict_node(x) as(DictNode*, untag48, x)
#define is_dict_leaf(x) has_type(x, &DictLeafType)
#define as_dict_leaf(x) as(DictLeaf*, untag48, x)

Dict* mk_dict(size_t n, Value* kvs);
Value dict_get(Dict* d, Value k);
bool  dict_has(Dict* d, Value k);
Dict* dict_set(Dict* d, Value k, Value v);
Dict* dict_del(Dict* d, Value k);
Dict* join_dicts(Dict* dx, Dict* dy);

/* Dict HAMT apis */
Dict*     freeze_dict(Dict* d);
Dict*     unfreeze_dict(Dict* d);
DictNode* freeze_dict_node(DictNode* n);
DictNode* unfreeze_dict_node(DictNode* n);
DictLeaf* freeze_dict_leaf(DictLeaf* l);
DictLeaf* unfreeze_dict_leaf(DictLeaf* l);

/* Set API */
#define is_set(x)       has_type(x, &SetType)
#define as_set(x)       as(Set*, untag48, x)
#define is_set_node(x)  has_type(x, &SetNodeType)
#define as_set_node(x)  as(SetNode*, untag48, x)
#define is_set_leaf(x)  has_type(x, &SetLeafType)
#define as_set_leaf(x)  as(SetLeaf*, untag48, x)

Set*  mk_set(size_t n, Value* ks);
bool  set_has(Set* s, Value k);
Set*  set_add(Set* s, Value k);
Set*  set_del(Set* s, Value k);
Set*  join_sets(Set* sx, Set* sy);

/* Set HAMT apis */
Set*     freeze_set(Set* s);
Set*     unfreeze_set(Set* s);
SetNode* freeze_set_node(SetNode* n);
SetNode* unfreeze_set_node(SetNode* n);
SetLeaf* freeze_set_leaf(SetLeaf* l);
SetLeaf* unfreeze_set_leaf(SetLeaf* l);

#endif
