#ifndef rascal_tuple_h
#define rascal_tuple_h

#include "rascal.h"
#include "describe/array.h"
#include "describe/safety.h"

struct tuple_t ArrayType(value_t, flags, 0);

// utilities ------------------------------------------------------------------
#define astup(x)   ((tuple_t*)ptr(x))
#define tlen(x)    (astup(x)->len)
#define tcap(x)    (astup(x)->cap)
#define tdata(x)   (astup(x)->data)
#define tspace(x)  (&((astup(x)->space)[0]))

#define tlen_s(x)   SafeGet(x,tuple,len)
#define tcap_s(x)   SafeGet(x,tuple,cap)
#define tdata_s(x)  SafeGet(x,tuple,data)
#define tspace_s(x) (&(SafeGet(x,tuple,space)[0]))

// api ------------------------------------------------------------------------
DeclareSafeCast(tuple, tuple_t);
DeclareArraySize(tuple);
DeclareArrayData(tuple, value_t);

#define tupledata(x)                  GenericArrayData(tuple, x)
#define tuplesize(x, padded, inlined) GenericArraySize(tuple, x, padded, inlined)
#define totuple(x)                    GenericSafeCast(tuple, x)

#endif
