#ifndef rascal_impl_htable_h
#define rascal_impl_htable_h

#include <assert.h>
#include <stdlib.h>

#include "../runtime.h"
#include "../util/collection.h"
#include "../util/number.h"
#include "../util/hashing.h"
#include "../util/ios.h"

#undef HTABLE

#define HTABLE(T, K, V, compare, hashKey, reHash, intern, noKey, noVal)	\
  static T##Entry *T##Locate(T *table, K key, uhash hash) {             \
    usize idx = hash & (table->capacity-1);                             \
    T##Entry *entry = table->table+idx;                               \
    for (;;) {                                                          \
      if (entry->key == noKey) {                                        \
        break;                                                          \
      } if (compare(entry->key, key)) {                                 \
        break;                                                          \
      }                                                                 \
      idx = (idx+1) & (table->capacity-1);                              \
      entry = table->table+idx;                                       \
    }                                                                   \
    return entry;                                                       \
  }                                                                     \
                                                                        \
  static void init##T##Table(T##Entry *table, int cap) {            \
    for (int i=0; i<cap; i++) {                                         \
      table[i].key = noKey;                                           \
      table[i].val = noVal;                                           \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void resize##T(T *table, int newCount) {                       \
    if (unlikely(newCount < 0)) { /* assume overflow */                 \
      eprintf("Requested size %u exceeds maximum %s capacity.\n",       \
              (uint)newCount,                                           \
              #T);                                                      \
      exit(1);                                                          \
    }                                                                   \
    int oldCap = table->capacity;                                       \
    int newCap = padTableSize(newCount, oldCap);                        \
    if (newCap != oldCap) {                                             \
      T##Entry *newTable = allocArr(newCap, sizeof(T##Entry));        \
      T##Entry *oldTable = table->table;                            \
      init##T##Table(newTable, newCap);                             \
      for (int i=0; i < oldCap; i++) {                                  \
        if (oldTable[i].key == noKey) {                               \
          continue;                                                     \
        }                                                               \
        uhash hash  = reHash(oldTable[i]);                            \
        ulong index = hash & (newCap-1);                                \
        for (;;) {                                                      \
          if (newTable[i].key == noKey) {                             \
            break;                                                      \
          }                                                             \
          index = (index + 1) & (newCap-1);                             \
        }                                                               \
        newTable[index].key = oldTable[index].key;                  \
        newTable[index].val = oldTable[index].val;                  \
      }                                                                 \
      table->table = newTable;                                      \
      table->capacity= newCap;                                          \
      deallocArr(oldTable, table->count, oldCap);                     \
    }                                                                   \
    table->count = newCount;                                            \
  }                                                                     \
                                                                        \
  T *create##T(void) {                                                  \
    return allocate(sizeof(struct T));                                  \
  }                                                                     \
                                                                        \
  void destroy##T(T *table) {                                           \
    free##T(table);                                                     \
    deallocate(table, sizeof(struct T));                                \
  }                                                                     \
                                                                        \
  void init##T(T *table) {                                              \
    table->count    = 0;                                                \
    table->capacity = MinCap;                                           \
    table->table  = allocArr(MinCap, sizeof(T##Entry));               \
    init##T##Table(table->table, table->capacity);                  \
  }                                                                     \
                                                                        \
  void free##T(T *table) {                                              \
    deallocArr(table->table, table->capacity, sizeof(T##Entry));      \
    init##T(table);                                                     \
  }                                                                     \
                                                                        \
  V T##Get(T *table, K key) {                                           \
    uhash hash = hashKey(key);                                          \
    T##Entry *entry = T##Locate(table, key, hash);                      \
    return entry->val;                                                  \
  }                                                                     \
                                                                        \
  bool T##Set(T *table, K key, V val) {                                 \
    uhash hash = hashKey(key);                                          \
    T##Entry *entry = T##Locate(table, key, hash);                      \
    bool out = false;                                                   \
    entry->val = val;                                                   \
    if (entry->key == noKey) {                                          \
      entry->key = key;                                                 \
      resize##T(table, table->count+1);                                 \
      out = true;                                                       \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  V T##Intern(T *table, K key) {                                        \
    uhash hash = hashKey(key);                                          \
    V out;                                                              \
    T##Entry *entry = T##Locate(table, key, hash);                      \
    if (entry->key == noKey) {                                          \
      out = intern(entry, key, hash);                                   \
      resize##T(table, table->count+1);                                 \
    } else {                                                            \
      out = entry->val;                                                 \
    }                                                                   \
    return out;                                                         \
  }                                                                     \
                                                                        \
  bool T##Delete(T *table, K key) {                                     \
    uhash hash = hashKey(key);                                          \
    T##Entry *entry = T##Locate(table, key, hash);                      \
    if (entry->key == noKey) {                                          \
      return false;                                                     \
    }                                                                   \
    entry->key = noKey;                                                 \
    entry->val = noVal;                                                 \
    resize##T(table, table->count-1);                                   \
    return true;                                                        \
  }                                                                     \
                                                                        \
  void merge##T##s(T *from, T *to) {                                    \
    T##Entry *fromTable = from->table;                                  \
    int fromCap = from->capacity;                                       \
    for (int i=0; i<fromCap; i++) {                                     \
      if (fromTable[i].key == noKey) {                                  \
        continue;                                                       \
      }                                                                 \
      T##Set(to, fromTable[i].key, fromTable[i].val);                   \
    }                                                                   \
  }

#endif
