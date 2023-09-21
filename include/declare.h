#ifndef rascal_declare_h
#define rascal_declare_h

#define ARRAY_TYPE(ArrayType, ElType)                    \
                                                         \
  typedef struct {                                       \
    ElType* data;                                        \
    size_t  count;                                       \
    size_t  capacity;                                    \
  } ArrayType;                                           \
                                                         \
  void init##ArrayType(ArrayType* array);                \
  void write##ArrayType(ArrayType* array, ElType x);     \
  void free##ArrayType(ArrayType* array)

#define TABLE_TYPE(TableType, KeyType, ValType)                     \
  typedef struct {                                                  \
    KeyType key;                                                    \
    ValType val;                                                    \
  } TableType##Entry;                                               \
                                                                    \
  typedef struct {                                                  \
    TableType##Entry* table;                                        \
    size_t count;                                                   \
    size_t capacity;                                                \
  } TableType;                                                      \
                                                                    \
  void init##TableType(TableType* table);                           \
  void free##TableType(TableType* table);                           \
  bool TableType##Get(TableType* table, KeyType key, ValType* val); \
  bool TableType##Set(TableType* table, KeyType key, ValType val);  \
  bool TableType##Delete(TableType* table, KeyType key)

#endif
