#ifndef rascal_val_h
#define rascal_val_h

#include "core.h"

#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define SMALL     0x7ffd000000000000ul
#define IMMEDIATE 0x7ffe000000000000ul
#define INTEGER   0x7fff000000000000ul
#define ARITY     0xfffd000000000000ul
#define POINTER   0xfffe000000000000ul
#define OBJECT    0xffff000000000000ul

typedef uintptr_t  val_t;
typedef double     real_t;
typedef intptr_t   int_t;
typedef char       char_t;
typedef bool       bool_t;
typedef void      *ptr_t;

typedef uint32_t   val_type_t;
typedef uint16_t   opcode_t;

typedef struct obj_t  obj_t;
typedef struct repr_t repr_t;

// value types
enum
  {
    none_type,
    nil_type=1,

    int_type,
    real_type,
    bool_type,
    char_type,

    atom_type,
    cons_type,
    func_type,
    port_type,
    str_type,
    any_type,

    // internal types
    native_type,
    code_type,
    clo_type,
    stack_type,
    iobuf_type,
    instr_type,
    alist_type,

    symt_type,
    symt_kv_type,
    repr_type,
    heap_type,
    vm_type,
    ns_type,

    num_val_types
  };

#define BOOLEAN   (SMALL|((val_t)bool_type<<32))
#define CHARACTER (SMALL|((val_t)char_type<<32))
#define NIL       (IMMEDIATE|nil_type)

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

// forward declarations -------------------------------------------------------
val_type_t  typeof_val(val_t x);
repr_t     *reprof_val(val_t x);
size_t      sizeof_val(val_t x);
int_t       cmp_vals(val_t x, val_t y);
hash_t      hash_val(val_t x);

// convenience ----------------------------------------------------------------
#define val_tag(val)    ((val)&ARITY)
#define imm_type(val)   ((val)&(NUM_TYPES_PAD-1))
#define small_type(val) ((val)>>32&(NUM_TYPES_PAD-1))

#define as_char(val)    ((char)((val)&UINT32_MAX))

#endif
