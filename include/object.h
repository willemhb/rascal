#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

// general object flags
typedef enum {
  EDITP=0x400, // object may be updated in-place
} ObjFl;

struct Obj {
  Obj*     next;         // live objects list
  Map*     annot;        // object metadata
  Type*    type;         // type object
  uint64_t hash     : 48; // compressed hash
  uint64_t flags    : 11; // miscellaneous discretionary flags
  uint64_t hashed   :  1; // indicates whether self->hash is valid
  uint64_t no_sweep :  1; // indicates that an object wasn't allocated with malloc (don't call deallocate)
  uint64_t no_free  :  1; // indicates that an object's data was allocated statically (don't free)
  uint64_t gray     :  1; // gc mark flag
  uint8_t  black    :  1; // gc trace flag
  uint8_t  data[];
};

// empty singletons
extern struct Bits   EmptyBits;
extern struct String EmptyString;
extern struct Tuple  EmptyTuple;
extern struct List   EmptyList;
extern struct Vector EmptyVector;
extern struct Map    EmptyMap;

// miscellaneous utilities
#define obj_head(o, T, f, args...)                      \
  *((Obj*)o) = (Obj)                                    \
    {                                                   \
      .next=NULL,                                       \
      .type=&T##Type,                                   \
      .annot=&EmptyMap,                                 \
      .flags=f,                                         \
      .gray=true,                                       \
      args                                              \
    }

bool get_fl(void* p, flags_t f);
bool set_fl(void* p, flags_t f);
bool del_fl(void* p, flags_t f);

void* new_obj(Type* type, flags_t fl, size_t extra);
void  init_obj(void* p, Type* type, flags_t fl);
void* clone_obj(void* p);

#endif
