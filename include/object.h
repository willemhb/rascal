#ifndef rascal_object_h
#define rascal_object_h

#include "common.h"
#include "value.h"

// general object flags
typedef enum {
  EDITP=0x400, // object may be updated in-place
} ObjFl;

struct Obj {
  Obj*     next;         // live objects list
  Map*     annot;        // object metadata
  Type*    type;         // type object
  uint64_t hash    : 48; // compressed hash
  uint64_t flags   : 11; // miscellaneous discretionary flags
  uint64_t hashed  :  1; // indicates whether self->hash is valid
  uint64_t noSweep :  1; // indicates that an object wasn't allocated with malloc (don't call deallocate)
  uint64_t noFree  :  1; // indicates that an object's data was allocated statically (don't free)
  uint64_t gray    :  1; // gc mark flag
  uint8_t  black   :  1; // gc trace flag
  uint8_t  data[];
};

// miscellaneous utilities
bool getFl(void* p, flags_t f);
bool setFl(void* p, flags_t f);
bool delFl(void* p, flags_t f);

void  markObj(void* obj);
void  unmarkObj(void* obj);
void  markObjs(size_t n, void** objs);
void  unmarkObjs(size_t n, void** objs);
void* newObj(Type* type, int f, size_t extra);
void  initObj(void* p, Type* type, int f);
void* cloneObj(void* p);

#endif
