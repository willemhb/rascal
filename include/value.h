#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// C types --------------------------------------------------------------------

// utilities ------------------------------------------------------------------
// misc -----------------------------------------------------------------------
bool_t        Cbool(value_t x);

// tags & reprs ---------------------------------------------------------------
repr_t        repr(value_t x);
uintptr_t     repr_to_tag(repr_t r);

flonum_t      flval(value_t x);
ulong_t       ulval(value_t x);
slong_t       slval(value_t x);
uint_t        uival(value_t x);
sint_t        sival(value_t x);
uchar_t      *ouval(value_t x);
pointer_t     opval(value_t x);
pointer_t     Cpval(value_t x);

value_t       tag_ptr(void* p, uintptr_t t);
value_t       tag_ulong(ulong_t u, uintptr_t t);
value_t       tag_slong(slong_t s, uintptr_t t);
value_t       tag_uint(uint_t u, uintptr_t t);
value_t       tag_sint(sint_t u, uintptr_t t);

bool_t        is_allocated(value_t x);
bool_t        is_immediate(value_t x);
bool_t        is_reference(value_t x);
bool_t        is_bits(value_t x);

size_t        rsp_size(value_t x);
hash_t        rsp_hash(value_t x);
sint_t        rsp_order(value_t x, value_t y);

// native functions -----------------------------------------------------------
value_t native_size(value_t *args, size_t n_args);
value_t native_type(value_t *args, size_t n_args);
value_t native_hash(value_t *args, size_t n_args);
value_t native_order(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void value_globals_init(void);
void value_types_init(void);
void value_natives_init(void);

#endif
