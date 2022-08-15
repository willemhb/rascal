#include "numutils.h"
#include "hashing.h"

#include "value.h"

#include "number.h"
#include "array.h"
#include "record.h"
#include "number.h"

// utilities ------------------------------------------------------------------
inline bool_t    Cbool(value_t x) { return x != NIL && x != FALSE; }

repr_t repr(value_t x) {
  if (is_flonum(x))
    return REPR_FLONUM;

  if (x&SIGN)
    return 64 | ((x&DTMASK)>>48);

  return ((x&DTMASK)>>44) | (x&LTMASK);
}

uintptr_t repr_to_tag(repr_t r) {
  if (r <= REPR_NONE)
    return QNAN | ((value_t)(r&48)<<44) | (r&15);

  if (r < REPR_FLONUM)
    return SIGN | QNAN | ((value_t)(r&3)<<48);

  return 0;
}

repr_t tag_to_repr(uintptr_t t) {
  if ((t&QNAN) != QNAN)
    return REPR_FLONUM;
  
  if (t&SIGN)
    return 64 | ((t&DTMASK)>>48);

  return ((t&DTMASK)>>44) | (t&LTMASK);
}

inline flonum_t  flval(value_t x) { return ((C_data_t)x).fval; }
inline ulong_t   ulval(value_t x) { return x & LMASK; }
inline slong_t   slval(value_t x) { return ((C_data_t)x).lval; }
inline uint_t    uival(value_t x) { return x & IMASK; }
inline sint_t    sival(value_t x) { return (sint_t)(x & IMASK); }
inline pointer_t opval(value_t x) { return (pointer_t)(x&OMASK); }
inline pointer_t Cpval(value_t x) { return (pointer_t)(x&PMASK); }

#define make_tagf(ctype, member, type)		\
  value_t tag_##type(ctype x, uintptr_t t)	\
  {						\
    C_data_t r = { .member = x };		\
    return r.uval | t;				\
  }

inline make_tagf(void*, pval, ptr)
inline make_tagf(ulong_t, uval, ulong)
inline make_tagf(slong_t, lval, slong)
inline make_tagf(uint_t, uval, uint)
inline make_tagf(sint_t, uval, sint)

inline bool_t is_allocated(value_t x) { return (x & ALCMASK) == OBJECT0; }
inline bool_t is_immediate(value_t x) { return (x & ALCMASK) != OBJECT0; }

value_t rsp_type(value_t x) {
  repr_t r = repr(x);

  if (r == REPR_RECORD)
    return record_type(r);

  return tuple_ref(BuiltinTypes, r);
}

size_t rsp_size(value_t x) {
  repr_t r   = repr(x);
  size_t out = BaseSizes[r];

  if (Sizeof[r])
    out += Sizeof[r](x);

  return out;
}

sint_t rsp_order(value_t x, value_t y) {
  if (x == y)
    return 0;

  repr_t xr = repr(x), yr = repr(y);

  if (xr != yr)
    return ord_uint(xr, yr);

  if (Order[xr])
    return Order[xr](x, y);

  return ord_ulong(x, y);
}

hash_t rsp_hash(value_t x) {
  repr_t r = repr(x);

  if (Hash[r])
    return Hash[r](x);

  return int64hash(x);
}
