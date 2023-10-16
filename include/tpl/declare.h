#ifndef rascal_template_declare_h
#define rascal_template_declare_h

#define ARRAY_TYPE(A, X)                                                \
                                                                        \
  typedef struct {                                                      \
    X*      data;                                                       \
    size_t  cnt;                                                        \
    size_t  cap;                                                        \
  } A;                                                                  \
                                                                        \
  void   init##A(A* a);                                                 \
  void   free##A(A* a);                                                 \
  size_t resize##A(A* a, size_t newCnt);                                \
  size_t write##A(A* a, X x);                                           \
  size_t nWrite##A(A* a, size_t n, X* d);                               \
  size_t vWrite##A(A* a, size_t n, ...);                                \
  X      pop##A(A* a);                                                  \
  void   nPop##A(A* a, size_t n)

#define TABLE_TYPE(T, _t, K, V)                                      \
  typedef struct {                                                   \
    K key;                                                           \
    V val;                                                           \
  } T##Kv;                                                           \
                                                                     \
  typedef struct {                                                   \
    T##Kv* kvs;                                                      \
    size_t cnt;                                                      \
    size_t cap;                                                      \
  } T;                                                               \
                                                                     \
  void init##T(T* t);                                                \
  void free##T(T* t);                                                \
  bool _t##Add(T* t, K k, V* v, void* s);                            \
  bool _t##Get(T* t, K k, V* v);                                     \
  bool _t##Has(T* t, K k);                                           \
  bool _t##Set(T* t, K k, V v, void* s);                             \
  bool _t##Del(T* t, K k)

#endif
