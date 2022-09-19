
#ifndef rascal_table_h
#define rascal_table_h

#include "binary.h"

// parameters -----------------------------------------------------------------
// C types --------------------------------------------------------------------
struct ArrayList
{
  OBJ_HEAD(BinaryFl);
  Arity  length;
  Arity  cap;
  Value *values;
};

typedef enum __attribute__((packed))
  {
    KEY_STRING,
    KEY_POINTER,
    KEY_OBJECT,
    KEY_VALUE,
  } KeyType;

typedef enum __attribute__((packed))
  {
    BIND_ATOM,
    BIND_OBJECT,
    BIND_VALUE,
    BIND_ARITY,
  } BindType;

typedef struct
{
  KeyType  keyType;
  BindType bindType;
} TableFl;

typedef struct
{
  OBJ_HEAD(TableFl);
  Hash    hash;

  union
  {
    String *str;
    Void   *ptr;
    Obj    *obj;
    Value   val;
  } key;

  union
  {
    Atom   *atom;
    Value   val;
    Obj    *obj;
    UInt64  arity;
  } bind;
} Entry;

struct Table
{
  OBJ_HEAD(TableFl);
  Idno    counter;
  Arity   length;
  Arity   cap;
  Entry **entries;
};

// forward declarations -------------------------------------------------------
// hashing utilities ----------------------------------------------------------
Hash hashCstring( const Char *cstr );
Hash hashMemory( const UInt8 *bytes, Arity nBytes );
Hash hashInt( UInt64 i );
Hash hashReal( Real real );
Hash hashPointer( const Void *p );
Hash mixHash( Hash xHash, Hash yHash );

// arraylist API --------------------------------------------------------------
Value  arrListGet( ArrayList *array, Arity index );
Void   arrListPush( ArrayList *array, Value val );
Value  arrListPop( ArrayList *array );

// table API ------------------------------------------------------------------
#define tableAccessor(method, table, key, ...)				\
  _Generic((key),							\
	   Char*:table##method##Cstr,					\
	   Value:table##method##Val,					\
	   Void*:table##method##Ptr,					\
	   Obj*:table##method##Obj,					\
	   String*:table##method##Str)((table),				\
				       (key)				\
				       __VA_OPT__(,) __VA_ARGS__)

#define declareTableAccessors(method)				\
  Bool table##method##Cstr(Table*, Char*, Entry*);		\
  Bool table##method##Str(Table*, String*, Entry*);		\
  Bool table##method##Ptr(Table*, Void*, Entry*);		\
  Bool table##method##Obj(Table*, Obj*, Entry*);		\
  Bool table##method##Val(Table*, Value, Entry* )

#define tableGet(table, key, buf) tableAccessor(Get, table, key, buf)
#define tablePut(table, key, buf) tableAccessor(Put, table, key, buf)
#define tablePop(table, key, buf) tableAccessor(Pop, table, key, buf)

declareTableAccessors(Get);
declareTableAccessors(Put);
declareTableAccessors(Pop);

// utility macros -------------------------------------------------------------
#define asArrList(val)     (asObjType(ArrayList, val))
#define isArrList(val)     (isObjType(val, VAL_ARRAYLIST))
#define asTable(val)       (asObjType(Table, val))
#define isTable(val)       (isObjType(val, VAL_TABLE))
#define asEntry(val)       (asObjType(Entry, val))
#define isEntry(val)       (isObjType(val, VAL_ENTRY))

#define arrListLength(val) (asArrList(val)->length)
#define arrListCap(val)    (asArrList(val)->cap)
#define arrListVals(val)   (asArrayList(val)->values)

#define tableKeyType(val)  (asTable(val)->object.flags.keyType)
#define tableBindType(val) (asTable(val)->object.flags.bindType)
#define tableCount(val)    (asTable(val)->count)
#define tableLength(val)   (asTable(val)->length)
#define tableCap(val)      (asTable(val)->cap)
#define tableEntries(val)  (asTable(val)->entries)

#define entryKeyType(val)  (asEntry(val)->object.flags.keyType)
#define entryBindType(val) (asEntry(val)->object.flags.bindType)
#define entryHash(val)     (asEntry(val)->hash)
#define entryKey(val)      (asEntry(val)->key)
#define entryBind(val)     (asEntry(val)->bind)

#endif
