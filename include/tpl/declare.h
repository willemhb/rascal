#ifndef rascal_template_declare_h
#define rascal_template_declare_h

#define ARRAY_TYPE(ArrayType, ElType)                                   \
                                                                        \
  typedef struct {                                                      \
    ElType* data;                                                       \
    size_t  count;                                                      \
    size_t  capacity;                                                   \
  } ArrayType;                                                          \
                                                                        \
  void   init##ArrayType(ArrayType* array);                             \
  void   free##ArrayType(ArrayType* array);                             \
  size_t resize##ArrayType(ArrayType* array, size_t newCount);          \
  size_t write##ArrayType(ArrayType* array, ElType x);                  \
  size_t nWrite##ArrayType(ArrayType* array, size_t n, ElType* data);   \
  size_t vWrite##ArrayType(ArrayType* array, size_t n, ...);            \
  ElType pop##ArrayType(ArrayType* array);                              \
  void   nPop##ArrayType(ArrayType* array, size_t n)

#define TABLE_TYPE(TableType, tableType, KeyType, ValType)          \
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
  bool tableType##Add(TableType* table, KeyType key, ValType* val); \
  bool tableType##Get(TableType* table, KeyType key, ValType* val); \
  bool tableType##Set(TableType* table, KeyType key, ValType val);  \
  bool tableType##Delete(TableType* table, KeyType key)

#endif
