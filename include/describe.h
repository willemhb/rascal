#ifndef rascal_describe_h
#define rascal_describe_h

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

#define TABLE_TYPE(TableType, KeyType, ValType,                         \
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
      Entry* entry = &entries[index];                                   \
      if (entry->key == noKey) {                                        \
        if (entry->value == noValue) {                                  \
          return tombstone != NULL ? tombstone : entry;                 \
        } else {                                                        \
          if (tombstone == NULL)                                        \
            tombstone = entry;                                          \
        }                                                               \
      } else if (compare(entry->key, key)) {                            \
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
      entries[i].key   = noKey;                                         \
      entries[i].value = noValue;                                       \
    }                                                                   \
    /* reset table init count */                                        \
    table->count = 0;                                                   \
  }                                                                     \


#endif
