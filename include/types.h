#ifndef rascal_types_h
#define rascal_types_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here
// representation types
typedef uintptr_t    val_t;
typedef struct obj_t obj_t;

// immediate types
typedef uint64_t  type_t;
typedef double    real_t;
typedef int64_t   int_t;
typedef char      char_t;
typedef uint16_t  op_t;

// core user object types
typedef struct atom_t atom_t;
typedef struct cons_t cons_t;
typedef struct port_t port_t;

// vm, environment, and compiler types
typedef struct vm_t    vm_t;
typedef struct envt_t  envt_t;
typedef struct var_t   var_t;
typedef struct instr_t instr_t;
typedef struct code_t  code_t;

// internal types
typedef struct symt_t     symt_t;
typedef struct readt_t    readt_t;
typedef struct rentry_t   rentry_t;
typedef struct buffer_t   buffer_t;
typedef struct alist_t    alist_t;
typedef struct stack_t    stack_t;
typedef struct heap_t     heap_t;

// C function typedefs
typedef void  (*reader_fn_t)(port_t *stream, char32_t dispatch);
typedef void  (*printer_fn_t)(port_t *stream, val_t val);
typedef void  (*trace_fn_t)(obj_t *obj);
typedef void  (*free_fn_t)(obj_t *obj);

typedef union
{
  real_t as_real;
  val_t  as_val;
  void  *as_ptr;
  obj_t *as_obj;
} val_data_t;

// tags & masks
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define CHRTAG    0x7ffd000000000000ul
#define NULTAG    0x7ffe000000000000ul
#define OBJ       0xffff000000000000ul

#define PMASK     0x0000fffffffffffful
#define TMASK     0xffff000000000000ul

#define EOS       ((val_t)EOF|CHRTAG)
#define NUL       ((val_t)0  |NULTAG)

// builtin types
enum
  {
    REAL   =0x01,
    CHRTYPE=0x03,
    NULTYPE=0x04,

    HEAP   =0x10,
    VM     =0x11,
    ENVT   =0x12,
    VAR    =0x13,
    INSTR  =0x14,
    CODE   =0x15,
    CONS   =0x16,
    ATOM   =0x17,
    SYMT   =0x18,
    
    RENTRY =0x19,
    READT  =0x1a,
    PORT   =0x1b,
    BUFFER =0x1c,
    ALIST  =0x1d,
    STACK  =0x1e,
 
    ANY    =0x30,
    NONE   =0x31,
    N_TYPES
  };

// globals
extern const char *TypeNames[N_TYPES];

// forward declarations
type_t typeof_val(val_t val);
type_t typeof_obj(obj_t *obj);

#define rtypeof(val)				\
  _Generic((val),				\
	   val_t:typeof_val,			\
	   obj_t*:typeof_obj)((val))

#endif
