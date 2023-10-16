#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "value.h"

// generics 
#include "tpl/declare.h"

ARRAY_TYPE(Values, Value);
ARRAY_API(Values, Value, values);
ARRAY_TYPE(Objects, Obj*);
ARRAY_API(Objects, Obj*, objects);

// general object flags
typedef enum {
  EDITP = 0x400, // object may be updated in-place
  TOTALP= 0x200, // indicates that an object  is being freshly initialized
} ObjFl;

struct Obj {
  Obj*     next;         // live objects list
  Map*     annot;        // object metadata
  Type*    type;         // type object
  uint64_t hash    : 48; // compressed hash
  uint64_t flags   : 11; // miscellaneous discretionary flags
  uint64_t hashed  :  1; // indicates whether self->hash is valid
  uint64_t noSweep :  1; // indicates that an object wasn't allocated with malloc (don't call deallocate)
  uint64_t noFree  :  1; // indicates that an object's data is allocated statically (don't call free)
  uint64_t gray    :  1; // gc mark flag
  uint8_t  black   :  1; // gc trace flag
  uint8_t  data[];
};

ARRAY_OBJ_TYPE(Binary8, byte_t);
ARRAY_OBJ_TYPE(Binary16, uint16_t);
ARRAY_OBJ_TYPE(Binary32, uint32_t);
ARRAY_OBJ_TYPE(Buffer8, char);
ARRAY_OBJ_TYPE(Buffer16, char16_t);
ARRAY_OBJ_TYPE(Buffer32, char32_t);
ARRAY_OBJ_TYPE(Alist, Value);
TABLE_OBJ_TYPE(Table, Value, Value);

// globals
extern struct Type Buffer8Type, Buffer16Type, Buffer32Type, Binary8Type, Binary16Type, Binary32Type,
  AlistType, TableType;

// miscellaneous utilities
int getFl(void* p, int f, int m);
int setFl(void* p, int f, int m);
int delFl(void* p, int f);

void* newObj(Type* type, int f, size_t extra);
void  initObj(void* p, Type* type, int f);
void* cloneObj(void* p);

// utility collection APIs
ARRAY_OBJ_API(Binary8,  byte_t,   binary8);
ARRAY_OBJ_API(Binary16, uint16_t, binary16);
ARRAY_OBJ_API(Binary32, uint32_t, binary32);
ARRAY_OBJ_API(Buffer8,  char,     buffer8);
ARRAY_OBJ_API(Buffer16, char16_t, buffer16);
ARRAY_OBJ_API(Buffer32, char32_t, buffer32);
ARRAY_OBJ_API(Alist,    Value,    alist);
TABLE_OBJ_API(Table,    Value,    Value, table);

#endif
