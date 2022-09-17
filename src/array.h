#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

#define ARRAY_SPEC(elType)				\
  elType  space[0]

#define DYNAMIC_ARRAY_SPEC(elType)			\
  int     length;					\
  int     capacity;					\
  elType *data

#define ARRAY_API(arrType, elType)					\
  arrType *new##arrayType(Size count);					\
  Void     init##arrayType(arrType *array, Size count, elType *data);	\
  Void     finalize##arrayType(arrType *array);				\


#define DYNAMIC_ARRAY_API(arrType, elType)				\
  arrType *new##arrayType(Size count);					\
  Void     init##arrayType(arrType *array, Size count, elType *data);	\
  Void     finalize##arrayType(arrType *array);				\
  Void     addTo##arrayType(arrType *array, elType element)

struct Tuple
{
  OBJ_HEAD;
  ARRAY_SPEC(Value);
};

struct String
{
  OBJ_HEAD;
  ARRAY_SPEC(Value);
};

struct ArrayList
{
  OBJ_HEAD;
  DYNAMIC_ARRAY_SPEC(Value);
};

// forward declarations -------------------------------------------------------
DYNAMIC_ARRAY_API(ArrayList, Value);


// utility macros & statics ---------------------------------------------------
#define IS_TUPLE(val)      (isObjType(val, OBJ_TUPLE))
#define IS_STRING(val)     (isObjType(val, OBJ_STRING))
#define IS_ARRAY_LIST(val) (isObjType(val, OBJ_ARRAY_LIST))

#define AS_TUPLE(val)      ((Tuple*)AS_OBJ(val))
#define AS_STRING(val)     ((String*)AS_OBJ(val))
#define AS_ARRAY_LIST(val) ((ArrayList*)AS_OBJ(val))

#endif
