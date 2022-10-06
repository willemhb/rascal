#ifndef rascal_types_h
#define rascal_types_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here
// representation types
typedef uintptr_t    val_t;
typedef struct obj_t obj_t;

// immediate types
typedef uint32_t  type_t;
typedef double    real_t;
typedef int64_t   int_t;
typedef char      char_t;
typedef uint16_t  op_t;

// core user object types
typedef struct atom_t  atom_t;
typedef struct cons_t  cons_t;
typedef struct port_t  port_t;
typedef struct func_t  func_t;
typedef struct tuple_t tuple_t;
typedef struct str_t   str_t;

// vm, environment, and compiler types
typedef struct vm_t    vm_t;
typedef struct envt_t  envt_t;
typedef struct var_t   var_t;
typedef struct instr_t instr_t;
typedef struct code_t  code_t;

// method and dispatch types
typedef struct slots_t   slots_t;
typedef struct slot_t    slot_t;
typedef struct dtype_t   dtype_t;
typedef struct utype_t   utype_t;
typedef struct runtime_t runtime_t;
typedef struct meth_t    meth_t;
typedef struct metht_t   metht_t;

// internal types
typedef struct symt_t   symt_t;
typedef struct readt_t  readt_t;
typedef struct rentry_t rentry_t;
typedef struct buffer_t buffer_t;
typedef struct alist_t  alist_t;
typedef struct stack_t  stack_t;
typedef struct heap_t   heap_t;

// C function typedefs
typedef void  (*reader_fn_t)(port_t *stream, char32_t dispatch);
typedef void  (*printer_fn_t)(port_t *stream, val_t val);
typedef void  (*mark_fn_t)(obj_t *obj);
typedef void  (*free_fn_t)(obj_t *obj);
typedef val_t (*native_fn_t)(val_t *args, arity_t n);

// union types
typedef union
{
  real_t  as_real;
  val_t   as_val;
  void   *as_ptr;
  obj_t  *as_obj;
} val_data_t;

typedef union
{
  native_fn_t native;
  code_t     *code;
} invoke_t;

// tags & masks
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define CHRTAG    0x7ffd000000000000ul
#define NULTAG    0x7ffe000000000000ul
#define BOOLTAG   0x7fff000000000000ul
#define INTTAG    0xfffd000000000000ul
#define PTRTAG    0xfffe000000000000ul
#define OBJ       0xffff000000000000ul

#define PMASK     0x0000fffffffffffful
#define TMASK     0xffff000000000000ul

#define EOS       ((val_t)EOF|CHRTAG)
#define NUL       ((val_t)0  |NULTAG)

// builtin types
enum
  {
    REAL    =0x01,
    INTTYPE =0x02,
    BOOLTYPE=0x03,
    CHRTYPE =0x04,
    NULTYPE =0x05,
    PTRTYPE =0x06,

    HEAP    =0x07,
    VM      =0x08,
    
    ENVT    =0x09,
    VAR     =0x0a,
    SLOT    =0x0b,
    SLOTS   =0x0c,    

    FUNC    =0x0d,
    DTYPE   =0x0e,
    UTYPE   =0x0f,
    RUNTIME =0x10,

    INSTR   =0x11,
    CODE    =0x12,
    
    TUPLE   =0x13,
    CONS    =0x14,
    
    ATOM    =0x15,
    SYMT    =0x16,
    
    RENTRY  =0x17,
    READT   =0x18,
    PORT    =0x19,
    BUFFER  =0x1a,
    ALIST   =0x1b,
    STACK   =0x1c,
 
    ANY     =0x30,
    NONE    =0x31,
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
