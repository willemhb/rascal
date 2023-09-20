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

#define TABLE_TYPE(TableType, KeyType, ValType,                     \
                   compareKeys, hashKey, internKey)                 \
  void init##TableType(TableType* table) {                          \
    table->table    = NULL;                                         \
    table->count    = 0;                                            \
    table->capacity = 0;                                            \
  }                                                                 \
                                                                    \
  void free##TableType(TableType* table) {                          \
    deallocate(table->table,                                        \
               table->capacity * sizeof(TableType##Entry),          \
               false);                                              \
    init##TableType(table);                                         \
  }                                                                 \
                                                                    \
                                                                    \

#endif
