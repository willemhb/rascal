#ifndef rascal_describe_h
#define rascal_describe_h

#undef ARRAY_TYPE
#undef TABLE_TYPE

#define ARRAY_TYPE(ArrayType, ElType)                           \
  void init##ArrayType(ArrayType* array) {                      \
    array->data     = NULL;                                     \
    array->count    = 0;                                        \
    array->capacity = 0;                                        \
  }                                                             \

#endif
