#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Mumutdict hashed collection types. */

/* C types */
typedef enum {
  FASTHASH=0x01u,
} TableFl;

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

/* globals */
extern Type MutDictType, MutSetType;

/* External APIs */
#define is_mdict(x)    has_type(x, &MutDictType)
#define as_mdict(x)    as(MutDict*, untag48, x)
#define is_mset(x)     has_type(x, &MutSetType)
#define as_mset(x)     as(MutSet*, untag48, x)
#define is_fasthash(x) get_fl(x, FASTHASH)

/* MutDict API */
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

#endif
