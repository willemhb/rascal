#ifndef rascal_template_declare_h
#define rascal_template_declare_h

#define ARRAY_DATA
#define TABLE_DATA

#define ARRAY_TYPE(A, X)                                                \
  typedef struct {                                                      \
    X*        data;                                                     \
    size_t    cnt;                                                      \
    size_t    cap;                                                      \
  } A

#define ARRAY_OBJ_TYPE(A, X)                    \
                                                \
  extern struct Type A##Type;                   \
                                                \
  struct A {                                    \
    Obj    obj;                                 \
    X*     data;                                \
    size_t cnt;                                 \
    size_t cap;                                 \
  }

#define ARRAY_API(A, X, _a)                                             \
  void   init##A(A* a);                                                 \
  void   free##A(A* a);                                                 \
  size_t resize##A(A* a, size_t newCnt);                                \
  size_t _a##Push(A* a, X x);                                           \
  size_t _a##Write(A* a, size_t n, X* d);                               \
  size_t _a##PushN(A* a, size_t n, ...);                                \
  X      _a##Pop(A* a);                                                 \
  void   _a##PopN(A* a, size_t n)

#define ARRAY_OBJ_API(A, X, _a)                                         \
  A*     new##A(int fl);                                                \
  void   init##A(A* a);                                                 \
  void   free##A(void* p);                                              \
  size_t resize##A(A* a, size_t newCnt);                                \
  size_t _a##Push(A* a, X x);                                           \
  size_t _a##Write(A* a, size_t n, X* d);                               \
  size_t _a##PushN(A* a, size_t n, ...);                                \
  X      _a##Pop(A* a);                                                 \
  void   _a##PopN(A* a, size_t n)

#define TABLE_TYPE(T, K, V)                                          \
  typedef struct {                                                   \
    K key;                                                           \
    V val;                                                           \
  } T##Kv;                                                           \
                                                                     \
  typedef struct {                                                   \
    T##Kv* kvs;                                                      \
    size_t cnt;                                                      \
    size_t cap;                                                      \
  } T

#define TABLE_OBJ_TYPE(T, K, V)                                      \
                                                                     \
  extern struct Type T##Type;                                        \
                                                                     \
  typedef struct {                                                   \
    K key;                                                           \
    V val;                                                           \
  } T##Kv;                                                           \
                                                                     \
  struct T {                                                         \
    Obj    obj;                                                      \
    T##Kv* kvs;                                                      \
    size_t cnt;                                                      \
    size_t cap;                                                      \
  }

#define TABLE_API(T, K, V, _t)                                       \
  void init##T(T* t);                                                \
  void free##T(T* t);                                                \
  bool _t##Intern(T* t, K k, funcptr_t i, void* s, void* b);         \
  bool _t##Add(T* t, K k, V  v);                                     \
  bool _t##Get(T* t, K k, V* v);                                     \
  bool _t##Has(T* t, K k);                                           \
  bool _t##Set(T* t, K k, V v);                                      \
  bool _t##Del(T* t, K k)

#define TABLE_OBJ_API(T, K, V, _t)                                   \
  T*   new##T(int fl);                                               \
  void init##T(T* t);                                                \
  void free##T(T* t);                                                \
  bool _t##Intern(T* t, K k, funcptr_t i, void* s, void* b);         \
  bool _t##Add(T* t, K k, V  v);                                     \
  bool _t##Get(T* t, K k, V* v);                                     \
  bool _t##Has(T* t, K k);                                           \
  bool _t##Set(T* t, K k, V v);                                      \
  bool _t##Del(T* t, K k)

#endif
