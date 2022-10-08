#ifndef rascal_val_h
#define rascal_val_h

#include "common.h"

// most typedefs, a few macros, and some globals are defined here
// representation types

// tags
#define QNAN        0x7ffc000000000000ul
#define SIGN        0x8000000000000000ul

#define SMALL_TAG    0x7ffc000000000000ul
#define POINTER_TAG  0x7ffd000000000000ul
#define SYMBOL_TAG   0x7ffe000000000000ul
#define LIST_TAG     0x7fff000000000000ul
#define FUNCTION_TAG 0xfffc000000000000ul
#define VECTOR_TAG   0xfffd000000000000ul
#define TABLE_TAG    0xfffe000000000000ul
#define CVALUE_TAG   0xffff000000000000ul

#define REAL_TAG     0x7ffc000000000000ul
#define INT_TAG      0x7ffc010000000000ul
#define CHAR_TAG     0x7ffc020000000000ul
#define BOOL_TAG     0x7ffc030000000000ul
#define PRIM_TAG     0x7ffc040000000000ul
#define TYPE_TAG     0x7ffc050000000000ul
#define CTYPE_TAG    0x7ffc060000000000ul

#define EOS_VAL   ((value_t)EOF|TAG_CHAR)
#define NUL_VAL   ((value_t)  0|TAG_LIST)

#define PMASK        0x0000fffffffffffful
#define TMASK        0xffff000000000000ul
#define NPMASK       0x0000fffffffffff0ul

// builtin types
enum
  {
   REAL, INTEGER, CHARACTER, BOOLEAN, PRIMITIVE, TYPE, CTYPE, POINTER,

   SYMBOL, SYMBOL_TABLE,

   NUL, CONS, VECTOR,

   FUNCTION, CONTROL,

   BYTECODE, INSTRUCTIONS, CLOSURE, ENVT, VARIABLE,

   DICT, DICT_ENTRY, SET, SET_ENTRY,

   STREAM, BUFFER, READER, READER_ENTRY,

   DATATYPE, UNIONTYPE, HEAP, VM,
  };

// typedefs
typedef uintptr_t value_t;
typedef double    real_t;
typedef char      char_t;
typedef bool      bool_t;
typedef int64_t   int_t;
typedef void     *ptr_t;
typedef uint32_t  type_t;
typedef uint32_t  Ctype_t;
typedef uint16_t  opcode_t;

// base object types
typedef struct object_t           object_t;
typedef struct symbol_t           symbol_t;
typedef struct cons_t             cons_t;
typedef struct vector_t           vector_t;
typedef struct function_t         function_t;
typedef struct cvalue_t           cvalue_t;

typedef struct table_t            table_t;
typedef struct table_t            dict_t;
typedef struct dict_entry_t       dict_entry_t; 
typedef struct table_t            set_t;
typedef struct set_entry_t        set_entry_t;
typedef struct environment_t      environment_t;
typedef struct variable_t         variable_t;
typedef struct slots_t            slots_t;
typedef struct slot_t             slot_t;
typedef struct read_table_t       read_table_t;
typedef struct read_table_entry_t read_table_entry_t;
typedef struct symbol_table_t     symbol_table_t;

typedef struct stream_t           stream_t;
typedef struct buffer_t           buffer_t;

typedef struct control_t          control_t;
typedef struct opcodes_t          opcodes_t;
typedef struct bytecode_t         bytecode_t;
typedef struct closure_t          closure_t;

typedef struct vm_t               vm_t;
typedef struct heap_t             heap_t;
typedef struct datatype_t         datatype_t;
typedef struct uniontype_t        uniontype_t;

// C function typedefs
typedef value_t (*native_fn_t)(value_t *args, arity_t n);
typedef void    (*reader_fn_t)(stream_t *stream, char32_t dispatch);
typedef void    (*printer_fn_t)(stream_t *stream, value_t value);
typedef void    (*mark_fn_t)(object_t *obj);
typedef void    (*free_fn_t)(object_t *obj);
typedef value_t (*native_fn_t)(value_t *args, arity_t n);

typedef union
{
  opcode_t     primitive;
  native_fn_t  native;
  bytecode_t  *bytecode;
} invoke_t;

// union types
typedef union
{
  real_t  as_real;
  value_t   as_val;
  void   *as_ptr;
  object_t  *as_obj;
} val_data_t;

// forward declarations
type_t typeof_value(value_t value);
type_t typeof_object(object_t *obj);

#define rtypeof(val)				\
  _Generic((val),				\
	   value_t:typeof_value,		\
	   object_t*:typeof_object)((val))

// mostly just forward declarations and utility macros
#define isa(val, type)	 (rtypeof(val)==(type))
#define is_nul(val)      ((val)==NUL)

#define as_val(val)      (((val_data_t)(val)).as_val)
#define as_ptr(val)					\
  _Generic((val),					\
	   value_t:((void*)(as_val((value_t)(val))&PMASK)),	\
	   default:(typeof(val))(val))
#define tag_ptr(val,tag) ((((value_t)(val))&PMASK)|(tag))

// forward declarations
void mark_value( value_t value );
void mark_values( value_t *values, size_t n );
void free_value( value_t value );

#endif
