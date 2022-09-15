#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"

#define  ENTRY_SPEC(keyType, bindType)		\
  keyType  key;					\
  bindType bind

#define TABLE_API(tableType, keyType, bindType, entryType)		\
  void init##entryType( entryType *entry, keyType key, bindType bind );	\
  void free##entryType( entryType *entry );				\
  void init##tableType( tableType *table );				\
  void free##tableType( tableType *table );				\
  void grow##tableType( tableType *table );				\
  void shrink##tableType( tableType *table );				\
  bool tableType##Get( tableType *table, keyType key, bindType *value ); \
  bool tableType##Put( tableType *table, keyType key, entryType *entry ); \
  bool tableType##Set( tableType *table, keyType key, bindType  value ); \
  bool tableType##Delete( tableType *table, keyType key );		\
  void tableType##AddAll( tableType *from, tableType *to )

#define TABLE_MIN_NKEYS       8
#define TABLE_RESIZE_PRESSURE 0.75

#define TABLE_IMPL(tableType, keyType, bindType,			\
		   entryType, hashFn, compare )				\
  void init##entryType( entryType *entry, keyType key, bindType bind )	\
  {									\
    entry->key   = key;							\
    entry->value = bind;						\
  }									\
  void free##entryType( entryType *entry )				\
  {									\
    free( entry );							\
  }									\
  void init##tableType( tableType *table )				\
  {									\
    table->count    = 0;						\
    table->capacity = TABLE_MIN_NKEYS;					\
    table->entries  = safeMalloc(__func__,				\
				 sizeof(void*)*newCap);			\
  }									\
  void free##tableType( tableType *table )				\
  {									\
    safeFree( __func__, table->entries );				\
    safeFree( table );							\
  }									\
  void grow##tableType( tableType *table )				\
  {									\
    int oldCap          = table->capacity;				\
    int newCap          = table->capacity = oldCap << 1;		\
    entryType **buffer  = table->entries;				\
    entryType **new     = safeMalloc( __func__,				\
				      sizeof(void*)*newCap);		\
    for (int i=0; i<oldCap; i++)					\
      {									\
	entryType *entry = buffer[i];					\
	if (entry == NULL)						\
	  continue;							\
	hash_t h   = hashFn( (const keyType)entry->key );		\
	size_t idx = h & (newCap-1);					\
	while (new[idx])						\
	  {								\
	    idx++;							\
	    if (idx >= newCap)						\
	      idx = 0;							\
	  }								\
	new[idx] = entry;						\
      }									\
    table->entries = new;						\
    safeFree( __func__, buffer );					\
  }									\
  									\
  void shrink##tableType( tableType *table )				\
  {									\
    int oldCap          = table->capacity;				\
    int newCap          = table->capacity = oldcap >> 1;		\
    entryType **buffer  = table->entries;				\
    entryType **new     = safeMalloc( __func__,				\
				      sizeof(void*)*newCap);		\
    for (int i=0; i<oldCap; i++)					\
      {									\
	entryType *entry = buffer[i];					\
	if (entry == NULL)						\
	  continue;							\
	hash_t h = hashFn( (const keyType)entry->key );			\
	while (new[idx])						\
	  {								\
	    idx++;							\
	    if (idx >= newCap)						\
	      idx = 0;							\
	  }								\
	new[idx] = entry;						\
      }									\
    table->entries = new;						\
    safeFree( __func__, buffer );					\
  }									\
  									\
  bool tableType##Get( tableType *table, keyType key, bindType *value ) \
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    size_t idx          = h & (table->capacity-1);			\
    entryType **entries = table->entries;				\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey =	entries[idx]->key;				\
	if (!compare( key, eKey ))					\
	  {								\
	    *value = entries[idx]->value;				\
	    return true;						\
	  }								\
	idx++;								\
	if (idx >= table->capacity )					\
	  idx = 0;							\
      }									\
    return false;							\
  }									\
  bool tableType##Put( tableType *table, keyType key, entryType **entry ) \
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    size_t idx          = h & (table->capacity-1);			\
    entryType **entries = table->entries;				\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey = entries[idx]->key;				\
	if (!compare( key, eKey ))					\
	  {								\
	    *value = entries[idx]->value;				\
	    return false;						\
	  }								\
	idx++;								\
	if ( idx >= table->capacity )					\
	  idx = 0;							\
      }									\
    entries[idx] = constructor( key, hash );				\
    *entry       = entries[idx];					\
    if (++table->length >= table->capacity * TABLE_RESIZE_PRESSURE )	\
      grow##tableType( table );						\
    return true;							\
  }									\
  bool tableType##Set( tableType *table, keyType key, bindType  value ) \
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    size_t idx          = h & (table->capacity-1);			\
    entryType **entries = table->entries;				\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey = entries[idx]->key;				\
	if (!compare( key, eKey ))					\
	  {								\
	    entries[idx]->value = value;				\
	    return true;						\
	  }								\
	idx++;								\
	if ( idx >= table->capacity )					\
	  idx = 0;							\
      }									\
    return false;							\
  }									\
  bool tableType##Delete( tableType *table, keyType key )		\
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    size_t idx          = h & (table->capacity-1);			\
    entryType **entries = table->entries;				\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey = entries[idx]->key;				\
	if (!compare( key, eKey ))					\
	  {								\
	    free##entryType( entries[idx] );				\
	    entries[idx] = NULL;					\
	    if ( --table->length <=					\
		 table->capacity / 2 * TABLE_RESIZE_PRESS )		\
	      shrink##tableType( table );				\
	    return true;						\
	  }								\
	idx++;								\
	if ( idx >= table->capacity )					\
	  idx = 0;							\
      }									\
    return false;							\
  }									\
  void tableType##AddAll( tableType *from, tableType *to )		\
  {									\
    for (int i=0; i < from->capacity; i++)				\
      {									\
	entryType *fromE = from->entries[i];				\
	if (fromE == NULL)						\
	  continue;							\
	entryType *toE;							\
	tableType##Put( to, fromE->key, &toE );				\
	toE->value = fromE->value;					\
      }									\
  }
    

typedef struct
{
  Value key;
  Value bind;
} MapEntry;

typedef struct
{
  Value key;
  Value bind;
} SetEntry;

struct Map
{
  OBJ_HEAD;
  ARRAY_SPEC(MapEntry*);
};

struct Set
{
  OBJ_HEAD;
  ARRAY_SPEC(SetEntry*);
};

// forward declarations -------------------------------------------------------
// hashing utilities ----------------------------------------------------------
hash_t hashCstring( const Cstring cstr );
hash_t hashMemory( const uint8_t *bytes, int nBytes );
hash_t hashInt( uint64_t i );
hash_t hashReal( Real real );
hash_t hashPointer( const Pointer p );

// user table type APIS -------------------------------------------------------
TABLE_API(Map, Value, Value, MapEntry);
TABLE_API(Set, Value, Value, SetEntry);

#endif
