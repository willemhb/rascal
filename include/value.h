#ifndef rascal_value_h
#define rascal_value_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here

// representation types
typedef uintptr_t       value_t;
typedef struct box_t    box_t;
typedef struct object_t object_t;
typedef struct varobj_t varobj_t;

// typedefs
typedef double    real_t;
typedef int64_t   int_t;
typedef char      char_t;
typedef bool      bool_t;
typedef uint16_t  opcode_t;
typedef uint16_t  primitive_t;

typedef struct symbol_t symbol_t;
typedef struct cons_t   cons_t;
typedef struct func_t   func_t;
typedef struct cntl_t   cntl_t;

typedef struct type_t   type_t;
typedef struct stream_t stream_t;
typedef struct tuple_t  tuple_t;

typedef value_t (*native_fn_t)(value_t *args, size_t n);
typedef void    (*reader_fn_t)(stream_t *stream, char32_t dispatch);
typedef void    (*printer_fn_t)(stream_t *stream, value_t value);
typedef void    (*mark_fn_t)(object_t *obj);
typedef void    (*free_fn_t)(object_t *obj);
typedef size_t  (*size_fn_t)(object_t *obj);


// tags
#define QNAN_BITS 32764
#define SIGN_BIT  32768

#define QNAN ((value_t)QNAN_BITS<<48)
#define SIGN ((value_t)SIGN_BIT<<48)

#define SMALL_TAG ((value_t)(QNAN_BITS|0)<<48)
#define INT_TAG   ((value_t)(QNAN_BITS|1)<<48)
#define FUNC_TAG  ((value_t)(QNAN_BITS|2)<<48)
#define CNTL_TAG  ((value_t)(QNAN|BITS|3)<<48)

#define ATOM_TAG  ((value_t)(SIGN_BIT|QNAN_BITS|0)<<48)
#define CONS_TAG  ((value_t)(SIGN_BIT|QNAN_BITS|1)<<48)
#define FOBJ_TAG  ((value_t)(SIGN_BIT|QNAN_BITS|2)<<48)
#define VOBJ_TAG  ((value_t)(SIGN_BIT|QNAN_BITS|3)<<48)

#define REAL_TAG  (SMALL_TAG|((value_t)0<<32))
#define CHAR_TAG  (SMALL_TAG|((value_t)1<<32))
#define BOOL_TAG  (SMALL_TAG|((value_t)2<<32))
#define PRIM_TAG  (SMALL_TAG|((value_t)3<<32))
#define SLOT_TAG  (SMALL_TAG|((value_t)4<<32))

struct box_t
{
  union
  {
    value_t next;

    struct                     // generic flags
    {
      value_t flags :  2;
      value_t black :  1;
      value_t gray  :  1;

      value_t       : 44;
      value_t obtag : 16;
    };
  };
};

#define BOX_HEAD  box_t box;
#define OBJ_HEAD  BOX_HEAD object_t *type;
#define VOBJ_HEAD OBJ_HEAD size_t    len;

#define NUL_TAG      (QNAN|((value_t)NUL<<40))

#define EOS_VAL      ((value_t)EOF|CHAR_TAG)
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
