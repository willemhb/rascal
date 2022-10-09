#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"
#include "utils.h"

// base object type
struct object_t
{
  union
  {
    value_t next;
    struct
    {
      value_t flags      :  4;
      value_t            : 44;
      value_t gray       :  1;
      value_t black      :  1;
      value_t            : 14;
    };

    struct // cons flags
    {
      value_t proper     :  1;
      value_t            : 63;
    };

    struct // symbol flags
    {
      value_t  gensym    :  1;
      value_t  keyword   :  1;
      value_t            : 62;
    };

    struct // type flags
    {
      value_t  no_trace  :  1;
      value_t  inlined   :  1; // whether array data is stored immediately or in a pointer
      value_t  hashed    :  1; // indicates whether the hash is initialized
      value_t            : 61;
    };

    struct // table flags
    {
      value_t ord_size :  4;
      value_t          : 60;
    };

    struct // slot flags
    {
      value_t boxed    :  1;
      value_t read     :  1;
      value_t write    :  1;
      value_t trace    :  1;
      value_t          : 60;
    };
  };

  data_type_t *data_type;
};

typedef struct type_t
{
  object_t    object;
  value_t     signature;
  hash_t      hash;
  symbol_t   *name;
  function_t *constructor;
} type_t;

struct data_type_t
{
  type_t       type;

  slots_t     *slots;
  size_t       base_size;
  data_type_t *eltype;     // array types only

  bool         fits_imm;
  bool         fits_word;

  mark_fn_t    mark;
  free_fn_t    free;
};

struct union_type_t
{
  type_t   type;
  set_t   *members;
};

typedef struct slot_init_t
{
  size_t  offset;
  char   *name;
  type_t *type;

  bool    unboxed;
  bool    read;
  bool    write;
  bool    trace;
} slot_init_t;

typedef struct slots_init_t
{
  size_t        n_slots;
  slot_init_t  *slot_specs;
} slots_init_t;

// type conveniences
#define as_type(val)          ((type_t*)as_ptr(val))
#define as_data_type(val)     ((data_type_t*)as_ptr(val))
#define as_union_type(val)    ((union_type_t*)as_ptr(val))

#define type_no_trace(val)    (as_object(val)->no_trace)
#define type_inlined(val)     (as_object(val)->inlined)
#define type_hashed(val)      (as_object(val)->hashed)
#define type_signature(val)   (as_type(val)->signature)
#define type_hash(val)        (as_type(val)->hash)
#define type_name(val)        (as_type(val)->name)
#define type_constructor(val) (as_type(val)->constructor)

#define type_slots(val)       (as_data_type(val)->slots)
#define type_base_size(val)   (as_data_type(val)->base_size)
#define type_eltype(val)      (as_data_type(val)->eltype)
#define type_fits_imm(val)    (as_data_type(val)->fits_imm)
#define type_fits_word(val)   (as_data_type(val)->fits_word)
#define type_mark(val)        (as_data_type(val)->mark)
#define type_free(val)        (as_data_type(val)->free)

#define type_members(val)     (as_union_type(val)->members)


#define is_data_type(val)   isa(val, DataTypeType)
#define is_union_type(val)  isa(val, UnionTypeType)
#define is_type(val)        isa(val, TypeType)

// convenience
#define as_object(val)      ((object_t*)as_ptr(val))
#define is_object(val)      (((val)&TMASK)==OBJECT_TAG)

#define obj_next(val)       (as_object(val)->next)
#define obj_gray(val)       (as_object(val)->gray)
#define obj_black(val)      (as_object(val)->black)
#define obj_flags(val)      (as_object(val)->flags)
#define obj_type(val)       (as_object(val)->data_type)

// internal utility types
// dynamic array of unboxed objects
typedef struct obj_alist_t
{
  size_t     length;
  size_t     capacity;
  object_t **data;
} obj_alist_t;

typedef struct obj_stack_t
{
  size_t     length;
  size_t     capacity;
  object_t **data;
} obj_stack_t;

// globals
extern type_t  *DataType, *UnionType, *AnyType, *NoneType, *Type;

#define for_slots(dtype, x)				\
  for (size_t i=0; i<table_length(type_slots(type)) && ((x=table_data(type_slots(type))[i])||1); i++)

#define for_members(utype, x)						\
  for (size_t i=0; i<table_length(type_members(type)) && ((x=table_data(type_members(type))[i])||1); i++)

// describe macros
#define OBJ_NEW(T)				\
  T##_t *new_##T(void)				\
  {						\
    return alloc(sizeof(T##_t));		\
  }

// forward declarations
// root object methods
void init_object(object_t *object, data_type_t *data_type, flags_t flags);
void mark_object(object_t *object);
void mark_objects(object_t **objects, size_t n);
void free_object(object_t *object);
void trace_object(object_t *object);

// data types
data_type_t  *new_data_type( void );
void          init_data_type( data_type_t *data_type, symbol_t *name, function_t *constructor, size_t base_size, data_type_t *eltype, bool fit_imm, slots_init_t *slots, mark_fn_t mark, free_fn_t free, bool inlined);
void          mark_data_type(object_t *object);

union_type_t *new_union_type( void );
void          init_union_type( union_type_t *union_type, symbol_t *name, function_t *constructor, size_t n_members, type_t **members );
void          mark_union_type(object_t *object);

bool          isa_p(value_t val, type_t *type);

// utility array types
obj_alist_t *new_obj_alist(void);
void         init_obj_alist(obj_alist_t *obj_alist);
void         resize_obj_alist(obj_alist_t *obj_alist, size_t newl);
void         clear_obj_alist(obj_alist_t *obj_alist);
size_t       obj_alist_push(obj_alist_t *obj_alist, object_t *object );
size_t       obj_alist_write(obj_alist_t *obj_alist, object_t **src, size_t n);
size_t       obj_alist_append(obj_alist_t *obj_alist, size_t n, ...);
bool         obj_alist_pop(obj_alist_t *obj_alist, object_t **buf );

obj_stack_t *new_obj_stack(void);
void         init_obj_stack(obj_stack_t *obj_stack);
void         init_obj_stack(obj_stack_t *obj_stack);
void         resize_obj_stack(obj_stack_t *obj_stack, size_t newl);
void         clear_obj_stack(obj_stack_t *obj_stack);
size_t       obj_stack_push(obj_stack_t *obj_stack, object_t *object);
size_t       obj_stack_write(obj_stack_t *obj_stack, object_t **src, size_t n);
size_t       obj_stack_append(obj_stack_t *obj_stack, size_t n, ...);
bool         obj_stack_pop(obj_stack_t *obj_stack, object_t **buf );

// convenience
#define DATA_TYPE_SLOTS(name, n, spec...)		\
  slots_init_t name = { .n_slots=(n) };			\
  slot_init_t __init_for_##name##__[] = { spec };	\
  name.slot_specs = __init_for_##name##__

#define UNION_TYPE_MEMBERS(name, n, types...)		\
  union_init_t name = { .n_members=(n) };		\
  type_t *__init_for_##name##__[] = { types };	\
  name.members = __init_for_##name##__

static inline Ctype_t type_Ctype(type_t *type)
{
  if (type->object.data_type == (data_type_t*)UnionType)
    return Ctype_uint64;

  if (is_object(type->signature))
    return Ctype_pointer;

  return type->signature&15;
}

#endif
