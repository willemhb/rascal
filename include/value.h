#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here

// representation types
typedef uintptr_t       value_t;
typedef struct object_t object_t;

// tags
#define QNAN_BITS 32764
#define SIGN_BIT  32768
#define ISIGN_BIT ((value_t)1<<47)

#define QNAN      ((value_t)QNAN_BITS<<48)
#define SIGN      ((value_t)SIGN_BIT<<48)

#define IMM_TAG   ((value_t)(QNAN_BITS|0)<<48)
#define PTR_TAG   ((value_t)(QNAN_BITS|1)<<48)
#define INT_TAG   ((value_t)(QNAN_BITS|2)<<48)
#define OBJ_TAG   ((value_t)(QNAN_BITS|3)<<48)

#define HDR_TAG   (QNAN|SIGN)

typedef enum repr_t
  {
    SYMBOL,

    CONS,

    // callable types
    FUNCTION, CONTROL, BYTECODE, CLOSURE,

    // stream type
    STREAM,

    // array types
    BINARY, ALIST, BUFFER,

    // table + entry types
    TABLE, ENTRY, ASSOC, VAR,

    BIGINT,

    RECORD,

    BOOL, PRIM, ASCII,

    REAL, INT, PTR,

    NONE, ANY, NUL,

    N_REPR
  } repr_t;

#define CHAR ASCII

#define NUL_VAL      (OBJ_TAG| CONS)
#define EOS_VAL      (IMM_TAG|((value_t)CHAR<<32)|(value_t)EOF)
#define TRUE_VAL     (IMM_TAG|((value_t)BOOL<<32)|(value_t)1)
#define FALSE_VAL    (IMM_TAG|((value_t)BOOL<<32)|(value_t)0)

#define HTMASK       (OBJ_TAG|SIGN)
#define ITMASK       ((value_t)UINT32_MAX<<32)
#define OTMASK       (HDR_TAG|(value_t)15)
#define PTRMASK      (~HTMASK)
#define OBPTRMASK    (~(HTMASK|(value_t)15))

// rascal lisp typedefs
typedef double            real_t;
typedef int64_t           int_t;
typedef void             *ptr_t;
typedef char              ascii_t;
typedef char              char_t;
typedef bool              bool_t;
typedef uint16_t          opcode_t;
typedef uint16_t          primitive_t;

typedef struct symbol_t    symbol_t;
typedef struct cons_t      cons_t;
typedef struct function_t  function_t;
typedef struct control_t   control_t;
typedef struct bytecode_t  bytecode_t;
typedef struct closure_t   closure_t;
typedef struct stream_t    stream_t;
typedef struct binary_t    binary_t;
typedef struct alist_t     alist_t;
typedef struct buffer_t    buffer_t;
typedef struct table_t     table_t;
typedef struct entry_t     entry_t;
typedef struct assoc_t     assoc_t;
typedef struct var_t       var_t;
typedef struct bigint_t    bigint_t;
typedef struct record_t    record_t;

typedef struct function_t  type_t;
typedef struct alist_t     vector_t;

// record types used in the VM
typedef struct module_t  module_t;
typedef struct envt_t    envt_t;
typedef struct closure_t closure_t;
typedef struct ns_t      ns_t;

// internal object types
typedef struct vm_t   vm_t;
typedef struct heap_t heap_t;

typedef void    (*reader_fn_t)(stream_t *stream, char32_t dispatch);
typedef void    (*printer_fn_t)(stream_t *stream, value_t value);
typedef void    (*mark_fn_t)(object_t *obj);
typedef void    (*free_fn_t)(object_t *obj);
typedef size_t  (*size_fn_t)(object_t *obj);
typedef hash_t  (*hash_fn_t)(object_t *obj);
typedef ord_t   (*order_fn_t)(object_t *x, object_t *y);

// union types
typedef union
{
  real_t      as_real;
  value_t     as_val;
  void       *as_ptr;
  object_t   *as_obj;
} val_data_t;

typedef union
{
  value_t (*thunk)(void);
  value_t (*ufunc)(value_t x);
  value_t (*bfunc)(value_t x, value_t y);
  value_t (*tfunc)(value_t x, value_t y, value_t z);
  value_t (*nfunc)(value_t *args, size_t n);
} native_fn_t;

typedef union
{
  opcode_t     primitive;
  native_fn_t  native;
  object_t    *user;
} invoke_t;

typedef union
{
  repr_t    primType;
  record_t *recordType;
  record_t *unionType;
  record_t *classType;
} type_repr_t;

// globals
extern type_t *BuiltinTypes[N_REPR];

#define SymbolType   (BuiltinTypes[SYMBOL])
#define ConsType     (BuiltinTypes[CONS])
#define FunctionType (BuiltinTypes[FUNCTION])
#define ControlType  (BuiltinTypes[CONTROL])
#define BytecodeType (BuiltinTypes[BYTECODE])
#define ClosureType  (BuiltinTypes[CLOSURE])
#define StreamType   (BuiltinTypes[STREAM])
#define BinaryType   (BuiltinTypes[BINARY])
#define AlistType    (BuiltinTypes[ALIST])
#define BufferType   (BuiltinTypes[BUFFER])
#define TableType    (BuiltinTypes[ARRAY])
#define EntryType    (BuiltinTypes[ENTRY])
#define AssocType    (BuiltinTypes[ASSOC])
#define VarType      (BuiltinTypes[VAR])
#define BigIntType   (BuiltinTypes[BIGINT])
#define BoolType     (BuiltinTypes[BOOL])
#define PrimType     (BuiltinTypes[PRIM])
#define CharType     (BuiltinTypes[CHAR])
#define RealType     (BuiltinTypes[REAL])
#define IntType      (BuiltinTypes[INT])
#define PtrType      (BuiltinTypes[PTR])
#define NoneType     (BuiltinTypes[NONE])
#define NulType      (BuiltinTypes[NUL])
#define AnyType      (BuiltinTypes[ANY])

// convenience
#define as_val(val)       (((val_data_t)(val)).as_val)

#define rl_repr(val)				\
  _Generic((val),				\
	   value_t:val_reprof,			\
	   object_t*:obj_reprof)((val))

#define as_ptr(val)						\
  _Generic((val),						\
	   value_t:val_as_ptr,					\
	   default:ptr_as_ptr)((val))

#define tag(val, repr)				\
  _Generic((val),				\
	   real_t:tag_real,			\
	   int_t:tag_int,			\
	   int32_t:tag_imm,			\
	   uint32_t:tag_imm,			\
	   object_t*:tag_obj,			\
	   default:tag_ptr)((val), (repr))

// forward declarations
void    *ptr_as_ptr(ptr_t ptr);
void    *val_as_ptr(value_t val);

repr_t   val_reprof( value_t val );
repr_t   obj_reprof( object_t *obj );

value_t  tag_real(real_t n, repr_t r);
value_t  tag_int(int_t i, repr_t r);
value_t  tag_imm(uint32_t i, repr_t r);
value_t  tag_obj(object_t *obj, repr_t r);
value_t  tag_ptr(void *ptr, repr_t r);

void     mark_val( value_t val );

size_t   val_size( value_t val );
hash_t   val_hash( value_t val );
ord_t    val_order( value_t x, value_t y );

extern   size_t obj_size( object_t *object );
extern   hash_t obj_hash( object_t *object );
extern   ord_t  obj_order( object_t *x, object_t *y );

// toplevel dispatch for hash, order, and size
#define rl_size(val)   _Generic((val), value_t: val_size, object_t*:  obj_size  )((val))
#define rl_hash(val)   _Generic((val), value_t: val_hash, object_t*:  obj_hash  )((val))
#define rl_order(x, y) _Generic((x),   value_t: val_order, object_t*: obj_order )((x), (y))

// convenience
static inline bool_t as_bool(value_t x) { return x == TRUE_VAL; }
static inline real_t as_real(value_t x) { return ((val_data_t)x).as_real; }

static inline int_t  as_int(value_t x)
{
  return (int_t)((x&ISIGN_BIT) ? (x|HTMASK) : (x&PTRMASK));
}

static inline char_t as_chr(value_t x)
{
  return (int)(x&UINT32_MAX);
}

#define REPR_PRED(type, TYPE)				\
  bool is_##type( value_t val )				\
  {							\
    return rl_repr(val) == TYPE;			\
  }
  
#define VAL_PRED(val, VAL)				\
  bool is_##val( value_t val )				\
  {							\
    return val == VAL;					\
  }

static inline REPR_PRED(int, INT);
static inline REPR_PRED(ptr, PTR);

static inline bool is_repr( value_t val, repr_t r )
{
  return rl_repr(val) == r;
}

static inline bool is_obj( value_t val )
{
  return (val&HTMASK) == OBJ_TAG && (val&OBPTRMASK) != 0;
}

static inline bool is_imm( value_t val )
{
  return (val&HTMASK) == IMM_TAG || (val&OBPTRMASK) == 0;
}

#endif
