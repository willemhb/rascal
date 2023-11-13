#include "util/number.h"
#include "util/collection.h"
#include "util/hashing.h"

#include "lang/equal.h"

#include "vm/memory.h"

#include "val/map.h"
#include "val/symbol.h"
#include "val/func.h"
#include "val/type.h"
#include "val/table.h"

/* Globals */
/* macros */
#define LOADF 0.625

/* Table type */
extern void trace_mdict(void* obj);
extern void finalize_mdict(void* obj);

INIT_OBJECT_TYPE(MutDict, NULL, trace_mdict, finalize_mdict);

/* MutSet type */
extern void trace_mutset(void* slf);
extern void finalize_mutset(void* slf);

INIT_OBJECT_TYPE(MutSet, NULL, trace_mutset, finalize_mutset);

/* External APIs */
/* MutSet API */
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
