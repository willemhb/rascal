#ifndef rascal_vector_h
#define rascal_vector_h

#include "hamt.h"

// describe macros ------------------------------------------------------------
typedef struct
{
  HAMT_FL_SPEC;
} VectorFl;

typedef union
{
  Value  space[0];
  Value *data;
} VectorData;

struct Vector
{
  HAMT_SPEC(VectorData, VectorFl);
};

// statics and utility macros -------------------------------------------------
#define vecArity(vec)      (asVector(vec)->object.arity)
#define vecLength(vec)     (asVector(vec)->object.flags.length)
#define vecDepth(vec)      (asVector(vec)->object.flags.depth)
#define vecShared(vec)     (asVector(vec)->object.flags.shared)
#define vecInlined(vec)    (asVector(vec)->object.flags.inlined)
#define vecInternal(vec)   (asVector(vec)->object.flags.internal)
#define vecLeaf(vec)       (asVector(vec)->object.flags.leaf)
#define vecBitmap(vec)     (asVector(vec)->bitmap)
#define vecCache(vec)      (asVector(vec)->cache)
#define vecParent(vec)     (asVector(vec)->parent)
#define vecSpace(vec)      (asVector(vec)->_data.space)
#define vecData(vec)       (asVector(vec)->_data.data)
#define vecElements(vec)   (getVectorElements(asVector(vec)))

HAMT_ELEMENTS(Vector, Value, space, data)

#endif
