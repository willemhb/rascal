#ifndef rl_tpl_impl_table_h
#define rl_tpl_impl_table_h

// describe macros
#define MAKETABLE( T )				\
  T *make##T( Void )				\
  {						\
    return (T*)create(&T##Type);		\
  }

#define FREETABLE( T, E )					\
  Int free##T( T *table )					\
  {								\
    deallocArray(table->entries, table->capacity, sizeof(E*));	\
    return 0;							\
  }

#define INITTABLE( T, E )				\
  Void init##T( T *table, Size count )			\
  {							\
    table->count = 0;					\
    table->capacity = padTableLength(count, 0);		\
    table->entries = allocArray(count, sizeof(E*));	\
  }

#define RESIZETABLE( T, E )						\
  Void rehash##T( T *table, Size newCap );				\
  Size resize##T( T *table, Size newCount )				\
  {									\
    Size oldCap = table->capacity,					\
         newCap = padTableLength(newCount, oldCap);			\
    if ( oldCap != newCap )						\
      rehash##T(table, newCap);						\
    return (table->count=newCount);					\
  }

#define REHASHTABLE( T, E, getHash )					\
  Void rehash##T( T *table, Size newCap )				\
  {									\
    E** newEntries = allocArray(newCap, sizeof(E*));			\
    for ( Size i=0, n=0; i<table->capacity && n<table->count; i++ )	\
      {									\
	if ( table->entries[i] )					\
	  {								\
	    ULong hash = getHash(table->entries[i]);			\
	    Size  index = hash & (newCap-1);				\
	    while ( newEntries[index] )					\
	      index = (index+1) & (newCap-1);				\
	    newEntries[index] = table->entries[i];			\
	    n++;							\
	  }								\
      }									\
    deallocArray(table->entries, table->capacity, sizeof(E));		\
    table->entries = newEntries;					\
    table->capacity = newCap;						\
  }

#define ADDTOTABLE( T, E, K, hashKey, getKey, compare )		\
  E* addTo##T( T *table, K key )				\
  {								\
    ULong hash = hashKey(key);					\
    Size index = hash & (table->capacity-1);			\
    while ( table->entries[index] )				\
      {								\
	if ( compare(key, getKey(table->entries[index])) )	\
	  return table->entries[index];				\
	index = (index+1) & (table->capacity-1);		\
      }								\
    E *entry = table->entries[index] = make##E();		\
    init##E(entry, key, hash);					\
    resize##T(table, table->count+1);				\
    return entry;						\
  }

#define LOOKUPINTABLE( T, E, K, hashKey, getKey, compare )		\
  E* lookupIn##T( T *table, K key )					\
  {									\
    ULong hash = hashKey(key);						\
    Size index = hash & (table->capacity-1);				\
    while ( table->entries[index] )					\
      {									\
	if ( compare(key, getKey(table->entries[index])) )		\
	  return table->entries[index];					\
	index = (index+1) & (table->capacity-1);			\
      }									\
    return NULL;							\
  }


#endif
