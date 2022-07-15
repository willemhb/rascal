#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>

#include "common.h"
#include "describe/utils.h"

// tag system -----------------------------------------------------------------
#define tag_QNaN      0x7ff8000000000000ul
#define tag_mQNaN     0xfff8000000000000ul

// regular tags
#define val_nil       0x7ff8000000000001ul
#define val_true      0x7ff8000000000002ul
#define val_false     0x7ff8000000000003ul

#define tag_fixnum    0x7ff9000000000000ul
#define tag_symbol    0x7ffa000000000000ul
#define tag_cons      0x7ffb000000000000ul
#define tag_vector    0x7ffc000000000000ul
#define tag_string    0x7ffd000000000000ul
#define tag_function  0x7ffe000000000000ul

// special tags
#define tag_forward   0x7fff000000000000ul // indicates a forwarding pointer

// masks
#define tag_mask      0x7fff000000000000ul
#define ptr_mask      0x0000fffffffffffful

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

typedef enum {
  type_number,
  type_nil,
  type_bool,
  type_fixnum,

  type_symbol,
  type_cons,
  type_vector,
  type_string,
  type_function
} type_t;

#define type_pad 16

typedef double   number_t;
typedef bool     boolean_t;
typedef void  ( *builtin_t )( value_t args, value_t envt );
typedef char    *string_t;
typedef value_t *vector_t;

typedef struct {
  value_t car;
  value_t cdr;
} cons_t;

typedef struct {
  value_t bind;
  ulong hash, idno;
  value_t len;
  char name[0];
} symbol_t;

typedef struct symbols_t {
  struct symbols_t *left, *right;
  symbol_t base;
} symbols_t;

typedef enum {
  C_sint8,
  C_uint8,
  C_sint16,
  C_uint16,
  C_sint32,
  C_uint32,
  C_int64,
  C_float64
} Ctype_t;

typedef enum {
  op_noop,
  
  op_push, op_pop,
  
  op_loadval, op_loadgl, op_loadloc,  op_loadupv,
  
  op_storegl, op_storeloc, op_storeupv,

  op_jump, op_jumpt, op_jumpf,

  op_argc, op_vargc,

  op_openupv, op_closeupv,
  
  op_call, op_return, op_closure, op_capture,

  op_done,
  
  num_instructions
} opcode_t;


#define numval(x)     (((ieee64_t)(x)).fp)
#define uptrval(x)    (((value_t)(x))&ptr_mask)
#define idxval(x)     uptrval(x)
#define ptrval(x)     ((void*)uptrval(x))
#define fixval(x)     (((value_t)(x))&ptr_mask)
#define longval(x)    ((long)uptrval(x))

#define gettag(x)     (((value_t)(x))&tag_mask)
#define settag(x, t)  (uptrval(x)|(t))
#define movtag(p, x)  settag(p, gettag(x))

#endif
