#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define HEADER					\
  Obj obj

#define NAN_BOXING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

// typedefs -------------------------------------------------------------------
typedef struct VM VM;
typedef struct Obj Obj;
typedef struct ObjString ObjString;
typedef struct ObjVector ObjVector;
typedef struct ObjList   ObjList;
typedef struct ObjSymbol ObjSymbol;

// globals --------------------------------------------------------------------
extern VM vm;

#endif
