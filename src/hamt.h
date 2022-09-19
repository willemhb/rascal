#ifndef rascal_hamt_h
#define rascal_hamt_h

#include "object.h"

// parameters -----------------------------------------------------------------
#define HAMT_MAX_NODE_COUNT 64
#define HAMT_MIN_NODE_COUNT  2

// describe macros ------------------------------------------------------------
#define HAMT_FL_SPEC				\
  UInt16 length    : 8;				\
  UInt16 depth     : 4;				\
  UInt16 shared    : 1;				\
  UInt16 inlined   : 1;				\
  UInt16 internal  : 1;				\
  UInt16 leaf      : 1

#define HAMT_SPEC(elType, flType)		\
  OBJ_HEAD(flType);				\
  union						\
  {						\
    Size bitmap;				\
    Hash hash;					\
  };						\
  union						\
  {						\
    Obj *cache;					\
    Obj *next;					\
    Obj *parent;				\
  };						\
  elType _data

#define HAMT_BUFFER_SPEC(HAMTType, maxDepth)		\
  Arity     count;					\
  Arity     indices[maxDepth];				\
  HAMTType *nodes[maxDepth]

#define HAMT_ELEMENTS(HAMTType, elType, _inl, _ptr)		\
  elType *get##HAMTType##Elements( HAMTType *h )		\
  {								\
    if (h->object.flags.inlined)				\
      return &(h->_data._inl[0]);				\
    return h->_data._ptr;					\
  }

// C types --------------------------------------------------------------------
typedef struct
{
  HAMT_FL_SPEC;
} HAMTFl;

typedef union
{
  Obj   **objects;
  Value  *values;
} HAMTData;

typedef struct
{
  HAMT_SPEC(HAMTFl, HAMTData);
} HAMT;

// utility macros & statics ---------------------------------------------------

static inline Arity HAMTNodeArity( HAMT *h )
{
  return popcount(h->bitmap);
}

#endif
