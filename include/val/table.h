#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Hashed collection types.

   Dicts and Sets are immutable HAMT-based collections inspired by Clojure.

   MutDicts and MutSets are mutable hashed collections based on common hashing
   techniques.

   TODO: update MutDict and MutSet implementation to use the same algorithm
   as Lua for better lookup and space efficiency, (see "R. P. Brent, Reducing the
   retrieval time of scatter storage techniques, Communications of the
   ACM 16 (1973), 105-109."). */

/* C types */
typedef struct DictNode DictNode;
typedef struct DictLeaf DictLeaf;

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

struct Dict {
  HEADER;
  DictNode* root;
  size_t    arity;
};

struct DictNode {
  HEADER;
  Obj**  children;
  size_t bitmap;
  size_t shift;
};

struct DictLeaf {
  HEADER;
  DictLeaf* next;
  Value     key;
  Value     val;
};

/* Globals */
/* Types */
extern Type MutDictType, DictType;

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

#endif
