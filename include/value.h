#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here
// representation types
typedef uintptr_t value_t;

// tags
#define QNBITS       (0x7ffc>>2)
#define QNAN         ((value_t)0x7ffc<<48)
#define SIGN         ((value_t)1<<63)

#define OBJECT_TAG   (SIGN|QNAN)

enum
  {
    REAL, INT, BOOL, UTF8, LATIN1, UTF16, UTF32, PRIMITIVE, CTYPE,

    NUL, NUM_IMM_TYPES,

    // these are only here because there's no other practicable way to hash them
    NONE=NUM_IMM_TYPES, ANY,
  };

#define REAL_TAG     (QNAN|((value_t)REAL<<40))
#define INT_TAG      (QNAN|((value_t)INT<<40))
#define BOOL_TAG     (QNAN|((value_t)BOOL<<40))
#define UTF8_TAG     (QNAN|((value_t)UTF8<<40))
#define PRIM_TAG     (QNAN|((value_t)PRIMITIVE<<40))
#define NUL_TAG      (QNAN|((value_t)NUL<<40))

#define EOS_VAL      ((value_t)EOF|UTF8_TAG)
#define NUL_VAL      ((value_t)0  |NUL_TAG)
#define TRUE_VAL     ((value_t)1  |BOOL_TAG)
#define FALSE_VAL    ((value_t)0  |BOOL_TAG)

#define TMASK        OBJECT_TAG
#define STMASK       (TMASK|((value_t)255<<40))
#define GCMASK       ((value_t)3<<48)
#define PMASK        (~(TMASK|GCMASK))
#define IMASK        (~STMASK)
#define FLMASK       ((value_t)15)
#define NPMASK       (PMASK&(~FLMASK))

// typedefs
typedef double    real_t;
typedef int64_t   int_t;
typedef char      char_t;
typedef bool      bool_t;
typedef uint16_t  opcode_t;
typedef uint16_t  primitive_t;

// basic object types
typedef struct object_t           object_t;
typedef struct symbol_t           symbol_t;
typedef struct cons_t             cons_t;
typedef struct tuple_t            tuple_t;
typedef struct vector_t           vector_t;
typedef struct table_t            table_t;
typedef struct slots_t            slots_t;
typedef struct set_t              set_t;
typedef struct stream_t           stream_t;
typedef struct function_t         function_t;
typedef struct method_t           method_t;
typedef struct method_table_t     method_table_t;
typedef struct control_t          control_t;
typedef struct bytecode_t         bytecode_t;
typedef struct type_t             type_t;
typedef struct data_type_t        data_type_t;
typedef struct union_type_t       union_type_t;
typedef struct heap_t             heap_t;
typedef struct vm_t               vm_t;

// C function typedefs
typedef value_t (*native_fn_t)(value_t *args, size_t n);
typedef void    (*reader_fn_t)(stream_t *stream, char32_t dispatch);
typedef void    (*printer_fn_t)(stream_t *stream, value_t value);
typedef void    (*mark_fn_t)(object_t *obj);
typedef void    (*free_fn_t)(object_t *obj);
typedef size_t  (*size_fn_t)(object_t *obj);

// union types
typedef union
{
  real_t      as_real;
  value_t     as_val;
  void       *as_ptr;
  object_t   *as_obj;
} val_data_t;

// utility types
typedef struct val_alist_t
{
  size_t   length;
  size_t   capacity;
  value_t *data;
} val_alist_t;

typedef struct val_stack_t
{
  size_t   length;
  size_t   capacity;
  value_t *data;
} val_stack_t;

// globals
extern type_t *ImmediateTypes[NUM_IMM_TYPES];

// forward declarations
data_type_t *typeof_value(value_t value);
data_type_t *typeof_object(object_t *object);
size_t       sizeof_value(value_t value);
size_t       sizeof_object(object_t *object);

#define rtypeof(val)				\
  _Generic((val),				\
	   value_t:typeof_value,		\
	   object_t*:typeof_object)((val))

#define rsizeof(val)				\
  _Generic((val),				\
	   value_t:sizeof_value,		\
	   object_t*:sizeof_object)((val))

// mostly just forward declarations and utility macros
#define isa(value, type) (rtypeof(value)==(type))
#define is_nul(value)    ((value)==NUL)

#define as_val(val)       (((val_data_t)(val)).as_val)
#define as_slot_spec(val) (((val_data_t)(val)).as_slot_spec)
#define as_prim_spec(val) (((val_data_t)(val)).as_prim_spec)
#define as_ptr(val)						\
  _Generic((val),						\
	   value_t:((void*)(as_val((value_t)(val))&PMASK)),	\
	   default:(typeof(val))(val))

#define tag_ptr(val,tag) ((((value_t)(val))&PMASK)|(tag))
#define tag_int(val,tag) ((((value_t)(val))|(tag)))

// forward declarations
void mark_value( value_t value );
void mark_values( value_t *values, size_t n );
void free_value( value_t value );

val_alist_t *new_val_alist(void);
void         init_val_alist(val_alist_t *val_alist);
void         resize_val_alist(val_alist_t *val_alist, size_t newl);
void         clear_val_alist(val_alist_t *val_alist);
size_t       val_alist_push(val_alist_t *val_alist, value_t value);
size_t       val_alist_write(val_alist_t *val_alist, value_t *src, size_t n);
size_t       val_alist_append(val_alist_t *val_alist, size_t n, ...);
bool         val_alist_pop(val_alist_t *val_alist, value_t *buf );

val_stack_t *new_val_stack(void);
void         init_val_stack(val_stack_t *val_stack);
void         init_val_stack(val_stack_t *val_stack);
void         resize_val_stack(val_stack_t *val_stack, size_t newl);
void         clear_val_stack(val_stack_t *val_stack);
size_t       val_stack_push(val_stack_t *val_stack, value_t value);
size_t       val_stack_write(val_stack_t *val_stack, value_t *src, size_t n);
size_t       val_stack_append(val_stack_t *val_stack, size_t n, ...);
bool         val_stack_pop(val_stack_t *val_stack, value_t *buf );

#endif
