
#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"

// 1 million fucking description macros ---------------------------------------
#define  ENTRY_SPEC(keyType, valueType)					\
  keyType   key;							\
  valueType value

#define TABLE_API(tableType, keyType, bindType, entryType)		\
  entryType *new##entryType(  void );					\
  void init##entryType(entryType *entry,keyType key, bindType bind);	\
  void free##entryType( entryType *entry );				\
  void init##tableType( tableType *table );				\
  void free##tableType( tableType *table );				\
  void grow##tableType( tableType *table );				\
  void shrink##tableType( tableType *table );				\
  bool tableType##Get(tableType *table,keyType key,bindType *value);    \
  bool tableType##Put(tableType *table,keyType key,entryType **entry);	\
  bool tableType##Set(tableType *table,keyType key,bindType  value);	\
  bool tableType##Delete( tableType *table, keyType key );		\
  void tableType##AddAll( tableType *from, tableType *to )

#define TABLE_MIN_NKEYS       8
#define TABLE_RESIZE_PRESSURE 0.75

#define TABLE_IMPL(tableType, keyType, bindType,			\
		   entryType, hashFn, compare )				\
  entryType *new##entryType( void )					\
  {									\
    return safeMalloc( __func__, sizeof(entryType) );			\
  }									\
  void init##entryType(entryType *entry,keyType key,bindType bind)	\
  {									\
    entry->key   = key;							\
    entry->value = bind;						\
  }									\
  void free##entryType( entryType *entry )				\
  {									\
    safeFree( __func__, entry );					\
  }									\
  void init##tableType( tableType *table )				\
  {									\
    table->length   = 0;						\
    table->capacity = TABLE_MIN_NKEYS;					\
    table->data     = safeMalloc(__func__,				\
				 sizeof(void*)*table->capacity);	\
  }									\
  void free##tableType( tableType *table )				\
  {									\
    safeFree( __func__, table->data );					\
    safeFree( __func__, table );					\
  }									\
  void grow##tableType( tableType *table )				\
  {									\
    int oldCap          = table->capacity;				\
    int newCap          = table->capacity = oldCap << 1;		\
    entryType **buffer  = table->data;					\
    entryType **new     = safeMalloc( __func__,				\
				      sizeof(void*)*newCap);		\
    for (int i=0; i<oldCap; i++)					\
      {									\
	entryType *entry = buffer[i];					\
	if (entry == NULL)						\
	  {								\
	    continue;							\
	  }								\
	hash_t h   = hashFn( (const keyType)entry->key );		\
	int idx    = h & (newCap-1);					\
	while (new[idx])						\
	  {								\
	    idx++;							\
	    if (idx >= newCap)						\
	      {								\
		idx = 0;						\
	      }								\
	  }								\
	new[idx] = entry;						\
      }									\
    table->data = new;							\
    safeFree( __func__, buffer );					\
  }									\
  									\
  void shrink##tableType( tableType *table )				\
  {									\
    int oldCap          = table->capacity;				\
    int newCap          = table->capacity = oldCap >> 1;		\
    entryType **buffer  = table->data;					\
    entryType **new     = safeMalloc( __func__,				\
				      sizeof(void*)*newCap);		\
    for (int i=0; i<oldCap; i++)					\
      {									\
	entryType *entry = buffer[i];					\
	if (entry == NULL)						\
	  {								\
	    continue;							\
	  }								\
	hash_t h   = hashFn( (const keyType)entry->key );		\
	int idx = h & (newCap-1);					\
	while (new[idx])						\
	  {								\
	    idx++;							\
	    if (idx >= newCap)						\
	      {								\
		idx = 0;						\
	      }								\
	  }								\
	new[idx] = entry;						\
      }									\
    table->data = new;						\
    safeFree( __func__, buffer );					\
  }									\
  									\
  bool tableType##Get( tableType *table, keyType key, bindType *value )	\
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    int idx             = h & (table->capacity-1);			\
    entryType **entries = table->data;				\
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
	  {								\
	    idx = 0;							\
	  }								\
      }									\
    return false;							\
  }									\
  bool tableType##Put(tableType *table,keyType key,entryType **entry)	\
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    int idx             = h & (table->capacity-1);			\
    entryType **entries = table->data;				\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey = entries[idx]->key;				\
	if (compare( key, eKey ))					\
	  {								\
	    *entry = entries[idx];					\
	    return false;						\
	  }								\
	idx++;								\
	if ( idx >= table->capacity )					\
	  {								\
	    idx = 0;							\
	  }								\
      }									\
    entries[idx] = new##entryType( );					\
    init##entryType( entries[idx], key, (bindType)0 );			\
    *entry       = entries[idx];					\
    if (++table->length >= table->capacity * TABLE_RESIZE_PRESSURE )	\
      {									\
	grow##tableType( table );					\
      }									\
    return true;							\
  }									\
  bool tableType##Set( tableType *table, keyType key, bindType  value )	\
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    int idx          = h & (table->capacity-1);				\
    entryType **entries = table->data;				\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey = entries[idx]->key;				\
	if (compare( key, eKey ))					\
	  {								\
	    entries[idx]->value = value;				\
	    return true;						\
	  }								\
	idx++;								\
	if ( idx >= table->capacity )					\
	  {								\
	    idx = 0;							\
	  }								\
      }									\
    return false;							\
  }									\
  bool tableType##Delete( tableType *table, keyType key )		\
  {									\
    hash_t h            = hashFn( (const keyType)key );			\
    int idx             = h & (table->capacity-1);			\
    entryType **entries = table->data;					\
    while ( entries[idx] != NULL )					\
      {									\
	keyType eKey = entries[idx]->key;				\
	if (compare( key, eKey ))					\
	  {								\
	    free##entryType( entries[idx] );				\
	    entries[idx] = NULL;					\
	    if ( --table->length <=					\
		 table->capacity / 2 * TABLE_RESIZE_PRESSURE )		\
	      {								\
		shrink##tableType( table );				\
	      }								\
	    return true;						\
	  }								\
	idx++;								\
	if ( idx >= table->capacity )					\
	  { 								\
	    idx = 0;							\
	  }								\
      }									\
    return false;							\
  }									\
  void tableType##AddAll( tableType *from, tableType *to )		\
  {									\
    for (int i=0; i < from->capacity; i++)				\
      {									\
	entryType *fromE = from->data[i];				\
	if (fromE == NULL)						\
	  {								\
	    continue;							\
	  }								\
	entryType *toE;							\
	tableType##Put( to, fromE->key, &toE );				\
	toE->value = fromE->value;					\
      }									\
  }

typedef struct
{
  ENTRY_SPEC(Value, Value);
} MapEntry;

typedef struct
{
  ENTRY_SPEC(Value, Value);
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
hash_t mixHash( hash_t xHash, hash_t yHash );

// user table type APIS -------------------------------------------------------
TABLE_API(Map, Value, Value, MapEntry);
TABLE_API(Set, Value, Value, SetEntry);

#endif
