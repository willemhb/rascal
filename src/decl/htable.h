#ifndef rascal_decl_htable_h
#define rascal_decl_htable_h

#define HTABLE_API(T, K, V)			   \
  T   *create##T(void);				   \
  void destroy##T(T *table);			   \
  void init##T(T *table);                          \
  void free##T(T *table);                          \
  V    T##Get(T *table, K key);			   \
  bool T##Set(T *table, K key, V val);		   \
  V    T##Intern(T *table, K key);		   \
  bool T##Delete(T *table, K key);		   \
  void merge##T##s(T *from, T *to)

#define HTABLE(T, K, V)				   \
  typedef struct T##Entry {			   \
    K key;					   \
    V val;					   \
  } T##Entry;                                      \
  						   \
  typedef struct T {				   \
    T##Entry *table;				   \
    int count;                                     \
    int capacity;                                  \
  } T;                                             \
  						   \
  HTABLE_API(T, K, V)


#endif
