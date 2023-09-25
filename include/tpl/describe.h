#ifndef rascal_template_describe_h
#define rascal_template_describe_h

#include <string.h>
#include <stdarg.h>

#include "util/number.h"

#include "memory.h"

#undef ARRAY_TYPE
#undef TABLE_TYPE

#define ARRAY_TYPE(ArrayType, ElType, VaType)                           \
  void init##ArrayType(ArrayType* array) {                              \
    array->data     = NULL;                                             \
    array->count    = 0;                                                \
    array->capacity = 0;                                                \
  }                                                                     \
                                                                        \
  void free##ArrayType(ArrayType* array) {                              \
    deallocate(array->data, array->capacity * sizeof(ElType), false);   \
    init##ArrayType(array);                                             \
  }                                                                     \
                                                                        \
  size_t resize##ArrayType(ArrayType* array, size_t newCount) {         \
    size_t oldCount = array->count;                                     \
    if (newCount == 0) {                                                \
      if (array->count != 0)                                            \
        free##ArrayType(array);                                         \
    } else if (newCount > array->capacity ||                            \
               (newCount < (array->capacity >> 1))) {                   \
      size_t oldCap   = array->capacity;                                \
      size_t newCap   = max(8u, ceilPow2(newCount+1));                  \
                                                                        \
      if (oldCap != newCap) {                                           \
        size_t oldSize  = oldCap * sizeof(ElType);                      \
        size_t newSize  = newCap * sizeof(ElType);                      \
        if (array->data == NULL)                                        \
          array->data = allocate(newSize, false);                       \
        else                                                            \
          array->data = reallocate(array->data,                         \
                                   oldSize,                             \
                                   newSize,                             \
                                   false);                              \
        array->capacity = newCap;                                       \
      }                                                                 \
    }                                                                   \
    array->count = newCount;                                            \
    assert(array->capacity >= array->count);                            \
    return oldCount;                                                    \
  }                                                                     \
                                                                        \
  void write##ArrayType(ArrayType* array, ElType x) {                   \
    size_t offset = resize##ArrayType(array, array->count+1);           \
    array->data[offset] = x;                                            \
  }                                                                     \
                                                                        \
  void write##ArrayType##N(ArrayType* array, size_t n, ElType* data) {  \
    size_t offset = resize##ArrayType(array, array->count+n);           \
    memcpy(array->data+offset, data, n*sizeof(ElType));                 \
  }                                                                     \
                                                                        \
  void write##ArrayType##V(ArrayType* array, size_t n, ...) {           \
    size_t offset = resize##ArrayType(array, array->count+n);           \
    va_list va; va_start(va, n);                                        \
                                                                        \
    for (size_t i=offset; i<array->count; i++)                          \
      array->data[i] = va_arg(va, VaType);                              \
                                                                        \
    va_end(va);                                                         \
  }                                                                     \
                                                                        \
  ElType pop##ArrayType(ArrayType* array) {                             \
    assert(array->count > 0);                                           \
    ElType x = array->data[array->count-1];                             \
    resize##ArrayType(array, array->count-1);                           \
    return x;                                                           \
  }                                                                     \
                                                                        \
  void pop##ArrayType##N(ArrayType* array, size_t n) {                  \
    assert(n <= array->count);                                          \
    if (array->count > 0 && n > 0)                                      \
      resize##ArrayType(array, array->count-n);                         \
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
    if (table->table != NULL) {                                         \
      for (size_t i=0; i<table->capacity; i++) {                        \
        TableType##Entry* entry = &table->table[i];                     \
                                                                        \
        if (entry->key == noKey) {                                      \
          continue;                                                     \
        }                                                               \
                                                                        \
        TableType##Entry* dest = find##TableType##Entry(entries,        \
                                                        capacity,       \
                                                        entry->key);    \
        dest->key = entry->key;                                         \
        dest->val = entry->val;                                         \
        table->count++;                                                 \
      }                                                                 \
      deallocate(table->table,                                          \
                 table->capacity*sizeof(TableType##Entry),              \
                 false);                                                \
    }                                                                   \
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
