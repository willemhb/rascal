#ifndef rascal_val_h
#define rascal_val_h

#include "core.h"

#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define SMALL     0x7ffc000000000000ul
#define INTEGER   0x7ffd000000000000ul
#define POINTER   0x7ffe000000000000ul
#define ARITY     0x7fff000000000000ul
#define OBJECT    0xfffc000000000000ul
#define TAGMASK   0xffff000000000000ul

typedef uintptr_t  val_t;

typedef uint32_t   type_t;
typedef uint16_t   opcode_t;

typedef struct obj_t  obj_t;
typedef struct repr_t repr_t;

// function pointer types
typedef hash_t  (*hash_fn_t)(val_t val);
typedef int     (*ord_fn_t)(val_t x, val_t y);
typedef arity_t (*prin_fn_t)(obj_t *io, val_t x);
typedef val_t   (*call_fn_t)(val_t f, val_t *args, arity_t n);

// native function pointer types
typedef val_t (*thunk_fn_t)(void);
typedef val_t (*unary_fn_t)(val_t x);
typedef val_t (*binary_fn_t)(val_t x, val_t y);
typedef val_t (*ternary_fn_t)(val_t x, val_t y, val_t z);
typedef val_t (*nary_fn_t)(val_t *args, arity_t n);

typedef union
{
  thunk_fn_t   thunk;
  unary_fn_t   unary;
  binary_fn_t  binary;
  ternary_fn_t ternary;
  nary_fn_t    nary;
} native_fn_t;

// value types
enum
  {
    none_type=1,
    any_type,

    int_type,
    real_type,
    bool_type,
    char_type,
    
    nil_type,
    cons_type,
    atom_type,
    func_type,
    port_type,
    str_type,
    table_type,

    // internal types
    code_type,
    clo_type,
    envt_type,
    multi_type,

    num_val_types
  };

#define BOOLEAN   (SMALL|((val_t)bool_type<<32))
#define CHARACTER (SMALL|((val_t)char_type<<32))
#define NIL       (SMALL|((val_t)nil_type<<32))


#define NUM_TYPES_PAD 256

typedef union
{
  val_t   as_val;
  real_t  as_real;
  obj_t  *as_obj;
  ptr_t   as_ptr;

  struct
  {
    int_t as_int : 48;
    int_t        : 16;
  };
} val_data_t;

typedef struct
{
  char_t     *name;
  hash_fn_t   hash;
  prin_fn_t   prin;
  ord_fn_t    ord;
  call_fn_t   call;
} val_api_t;

#include "template/arr.h"

DECL_ALIST(vals, val_t);
DECL_ALIST_API(vals, val_t);

// globals
extern val_api_t ValApis[num_val_types];

// forward declarations -------------------------------------------------------
type_t  typeof_val(val_t x);
size_t  sizeof_val(val_t x);
int     ord_vals(val_t x, val_t y);
hash_t  hash_val(val_t x);

// initialization -------------------------------------------------------------
void val_init( void );

// convenience ----------------------------------------------------------------
#define DECL_VAL_API(T)					\
  int_t   cmp_##T(val_t x, val_t y);			\
  arity_t prin_##T(obj_t *stream, val_t val);		\
  hash_t  hash_##T(val_t val);				\
  val_t   call_##T(val_t val, val_t *args, arity_t n)

#define val_tag(val)      ((val)&ARITY)
#define tag_val(val, tag) ((((val_data_t)(val)).as_val)|(tag))
#define imm_type(val)     ((val)&(NUM_TYPES_PAD-1))
#define small_type(val)   ((val)>>32&(NUM_TYPES_PAD-1))

#define as_small(val)      ((val)&UINT32_MAX)
#define as_char(val)       ((char)as_small(val))
#define as_float(val)      (float_bits((uint32_t)as_small(val)))
#define as_real(val)       (((val_data_t)(val)).as_real)
#define as_ptr(val)        ((((val_data_t)((val)&~TAGMASK))).as_ptr)

static inline int64_t as_int(val_t val)
{
  return ((val_data_t)(val)).as_int;
}

#endif
