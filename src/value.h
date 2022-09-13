#ifndef rascal_value_h
#define rascal_value_h

#include <string.h>

#include "common.h"

typedef enum
  {
    // immediate types --------------------------------------------------------
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
  } ValueType;

#ifdef NAN_BOXING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN     ((uint64_t)0x7ffc000000000000)


#define TAG_NIL   1 // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE  3 // 11.

typedef uint64_t Value;

typedef union
{
  uint64_t bits;
  double   num;
  Obj  *ob;
} ValueData;

#define IS_OBJ(value)    (((value) & (QNAN|SIGN_BIT)) == (QNAN|SIGN_BIT))
#define IS_BOOL(value)   (((value) | 1) == TRUE_VAL)
#define IS_NIL(value)    ((value) == NIL_VAL)
#define IS_NUMBER(value) (((value) & QNAN) != QNAN)

#define AS_OBJ(value)    ((Obj*)(uintptr_t)((value)&~(SIGN_BIT|QNAN)))
#define AS_BOOL(value)   (Cbool(value))
#define AS_NUMBER(value) (((ValueData)(value)).num)

#define NUMBER_VAL(num)  (((ValueData)(num)).bits)
#define OBJ_VAL(obj)     ((Value)(SIGN_BIT|QNAN|(uint64_t)(uintptr_t)(obj)))
#define BOOL_VAL(b)      ((b) ? TRUE_VAL : FALSE_VAL)

#define FALSE_VAL ((Value)(uint64_t)(QNAN|TAG_FALSE))
#define TRUE_VAL  ((Value)(uint64_t)(QNAN|TAG_TRUE))
#define NIL_VAL   ((Value)(uint64_t)(QNAN|TAG_NIL))

static inline bool Cbool(Value v)
{
  return v != NIL_VAL && v != FALSE_VAL;
}

#else

typedef double Value;

#endif

typedef struct
{
  ARRAY_HEADER;
  Value *values;
} ValueArray;


bool valuesEqual( Value a, Value b );
bool valuesSame( Value a, Value b );

void initValueArray( ValueArray *array );
void writeValueArray( ValueArray *array, Value value );
void freeValueArray( ValueArray *array );

void printValue( Value value );


#endif
