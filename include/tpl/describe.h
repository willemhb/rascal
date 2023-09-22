#ifndef rascal_template_describe_h
#define rascal_template_describe_h

#include "memory.h"

#undef ARRAY_TYPE
#undef TABLE_TYPE

#define ARRAY_TYPE(ArrayType, ElType)                                   \
  void init##ArrayType(ArrayType* array) {                              \
    array->data     = NULL;                                             \
    array->count    = 0;                                                \
    array->capacity = 0;                                                \
  }                                                                     \
                                                                        \
  void write##ArrayType(ArrayType* array, ElType x) {                   \
    if (array->capacity < array->count + 1) {                           \
      size_t oldCap  = array->capacity;                                 \
      size_t newCap  = oldCap < 8 ? 8 : oldCap << 1;                    \
      size_t oldSize = oldCap * sizeof(ElType);                         \
      size_t newSize = newCap * sizeof(ElType);                         \
      array->data    = reallocate(array->data,                          \
                                  oldSize,                              \
                                  newSize,                              \
                                  false);                               \
    }                                                                   \
    array->data[array->count++] = x;                                    \
  }                                                                     \
                                                                        \
  void free##ArrayType(ArrayType* array) {                              \
    deallocate(array->data, array->capacity * sizeof(ElType), false);   \
    init##ArrayType(array);                                             \
  }

#define TABLE_MAX_LOAD 0.75

#define TABLE_TYPE(TableType, tableType, KeyType, ValType,              \
                   compareKeys, hashKey, internKey,                     \
                   noKey, noValue)                                      \
  void init##TableType(TableType* table) {                              \
    table->table    = NULL;                                             \
    table->count    = 0;                                                \
    table->capacity = 0;                                                \
  }                                                                     \
                                                                        \
  void free##TableType(TableType* table) {                              \
    deallocate(table->table,                                            \
               table->capacity * sizeof(TableType##Entry),              \
               false);                                                  \
    init##TableType(table);                                             \
  }                                                                     \
                                                                        \
  static TableType##Entry* find##TableType##Entry(TableType##Entry* entries, \
                                                  size_t capacity,      \
                                                  KeyType key) {        \
    uint64_t hash = hashKey(key);                                       \
    uint64_t mask = capacity - 1;                                       \
    uint64_t index = hash & mask;                                       \
    TableType##Entry* tombstone = NULL;                                 \
                                                                        \
    for (;;) {                                                          \
      TableType##Entry* entry = &entries[index];                        \
      if (entry->key == noKey) {                                        \
        if (entry->val == noValue) {                                    \
          return tombstone != NULL ? tombstone : entry;                 \
        } else {                                                        \
          if (tombstone == NULL)                                        \
            tombstone = entry;                                          \
        }                                                               \
      } else if (compareKeys(entry->key, key)) {                        \
        return entry;                                                   \
      } else {                                                          \
        index = (index + 1) & mask;                                     \
      }                                                                 \
      }                                                                 \
  }                                                                     \
                                                                        \
  static void adjust##TableType##Capacity(TableType* table,             \
                                          size_t capacity) {            \
    size_t newSize = capacity * sizeof(TableType##Entry);               \
    TableType##Entry* entries = allocate(newSize, false);               \
    /* fill table */                                                    \
    for (size_t i=0; i<capacity; i++) {                                 \
      entries[i].key = noKey;                                           \
      entries[i].val = noValue;                                         \
    }                                                                   \
    /* reset table init count */                                        \
    table->count = 0;                                                   \
    for (size_t i=0; i<table->capacity; i++) {                          \
      TableType##Entry* entry = &table->table[i];                       \
                                                                        \
      if (entry->key == noKey) {                                        \
        continue;                                                       \
      }                                                                 \
                                                                        \
      TableType##Entry* dest = find##TableType##Entry(entries,          \
                                           capacity,                    \
                                           entry->key);                 \
      dest->key = entry->key;                                           \
      dest->val = entry->val;                                           \
      table->count++;                                                   \
    }                                                                   \
    deallocate(table->table,                                            \
               table->capacity*sizeof(TableType##Entry),                \
               false);                                                  \
    table->table = entries;                                             \
    table->capacity = capacity;                                         \
  }                                                                     \
                                                                        \
  bool tableType##Add(TableType* table, KeyType key, ValType* value) {  \
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {          \
      size_t capacity = table->capacity < 8 ? 8 : table->capacity << 1; \
      adjust##TableType##Capacity(table, capacity);                     \
    }                                                                   \
                                                                        \
    TableType##Entry* entry = find##TableType##Entry(table->table,      \
                                                     table->capacity,   \
                                                     key);              \
                                                                        \
    bool isNewKey = entry->key == noKey;                                \
                                                                        \
    if (isNewKey) {                                                     \
      table->count++;                                                   \
      if (entry->val == noValue) {                                      \
        table->count++;                                                 \
      }                                                                 \
      internKey(entry, key, value);                                     \
    } else {                                                            \
      *value = entry->val;                                              \
    }                                                                   \
                                                                        \
    return isNewKey;                                                    \
  }                                                                     \
                                                                        \
  bool tableType##Get(TableType* table, KeyType key, ValType* val) {    \
    if (table->count == 0) {                                            \
      *val = noValue;                                                   \
      return false;                                                     \
    }                                                                   \
                                                                        \
    TableType##Entry* entry = find##TableType##Entry(table->table,      \
                                                     table->capacity,   \
                                                     key);              \
                                                                        \
    if (entry->key == noKey) {                                          \
      *val = noValue;                                                   \
      return false;                                                     \
    }                                                                   \
                                                                        \
    *val = entry->val;                                                  \
    return true;                                                        \
  }                                                                     \
                                                                        \
  bool tableType##Set(TableType* table, KeyType key, ValType value) {   \
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {          \
      size_t capacity = table->capacity < 8 ? 8 : table->capacity << 1; \
      adjust##TableType##Capacity(table, capacity);                     \
    }                                                                   \
                                                                        \
    TableType##Entry* entry = find##TableType##Entry(table->table,      \
                                                     table->capacity,   \
                                                     key);              \
                                                                        \
    bool isNewKey = entry->key == noKey;                                \
                                                                        \
    if (isNewKey && entry->val == noValue) {                            \
      table->count++;                                                   \
    }                                                                   \
                                                                        \
    entry->key = key;                                                   \
    entry->val = value;                                                 \
                                                                        \
    return isNewKey;                                                    \
  }                                                                     \
                                                                        \
  bool tableType##Delete(TableType* table, KeyType key) {               \
    if (table->count == 0) {                                            \
      return false;                                                     \
    }                                                                   \
                                                                        \
    TableType##Entry* entry = find##TableType##Entry(table->table,      \
                                                     table->capacity,   \
                                                     key);              \
                                                                        \
    if (entry->key == noKey) {                                          \
      return false;                                                     \
    }                                                                   \
                                                                        \
    entry->key = noKey;                                                 \
    return true;                                                        \
  }


#endif
