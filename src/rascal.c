// system includes
#include <uchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <wctype.h>

// local includes
#include "common.h"
#include "utils.h"

// rascal type declarations
// base types
typedef uintptr_t      val_t;
typedef struct obj_t   obj_t;
typedef struct arr_t   arr_t;
typedef struct map_t   map_t;
typedef struct pair_t  pair_t;

typedef double         real_t;
typedef intptr_t       int_t;
typedef char           char_t;
typedef bool           bool_t;
typedef void          *ptr_t;

// pair types
typedef struct cons_t  cons_t;
typedef struct cons_t  upval_t;

// atom and reader types
typedef struct atom_t  atom_t;

// array types
typedef struct vec_t    vec_t;
typedef struct str_t    str_t;
typedef struct tuple_t  tuple_t;
typedef struct types_t  types_t;
typedef struct instr_t  instr_t;

// function & execution types
typedef uint16_t       op_t;
typedef uint32_t       type_t;

typedef struct func_t  func_t;
typedef struct utype_t utype_t;
typedef struct dtype_t dtype_t;
typedef struct cntl_t  cntl_t;

// method and method table types
typedef struct meth_t  meth_t;
typedef struct metht_t metht_t;

// mapping types & mapentry types
typedef struct dict_t   dict_t;
typedef struct set_t    set_t;
typedef struct dentry_t dentry_t;
typedef struct sentry_t sentry_t;


// io types
typedef struct port_t   port_t;

// closure and namespace types
typedef struct code_t  code_t;
typedef struct clo_t   clo_t;
typedef struct envt_t  envt_t;
typedef struct var_t   var_t;

// vm types & internal types
typedef struct vm_t     vm_t;
typedef struct heap_t   heap_t;
typedef struct symt_t   symt_t;
typedef struct readt_t  readt_t;
typedef struct rentry_t rentry_t;

// function pointer types
typedef hash_t  (*hash_fn_t)(val_t val);
typedef int     (*ord_fn_t)(val_t x, val_t y);
typedef arity_t (*prin_fn_t)(obj_t *io, val_t x);
typedef val_t   (*call_fn_t)(val_t f, val_t *args, arity_t n);
typedef int     (*init_fn_t)(void *spc, val_t val);
typedef void    (*finalize_fn_t)(obj_t **buf);
typedef void    (*mark_fn_t)(obj_t *obj);
typedef obj_t  *(*resize_fn_t)(obj_t *obj, arity_t n);
typedef size_t  (*pad_fn_t)(size_t oldl, size_t newl, size_t oldc, size_t minc);

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

// value accessor type
typedef union
{
  val_t   as_val;
  real_t  as_real;
  obj_t  *as_obj;
  ptr_t   as_ptr;
  int64_t as_int;
} val_data_t;

// forward declarations
int64_t as_int(val_t val);
type_t  typeof_val(val_t val);
bool    is_obj_type( val_t x, type_t t );

// array/map generics
void   resize_array(arr_t *array, size_t newl );
void   resize_map(map_t *map, size_t newl );

// forward declarations & generics
void *alloc(size_t n);
void  dealloc(void *ptr, size_t n);
void *resize( void *spc, size_t old, size_t new );
void  copy( void *dst, void *src, size_t n );
void *duplicate( void *ptr, size_t n );
void  finalize( obj_t **buffer );
void  trace_obj(obj_t *obj);
void  trace_val(val_t val);
void  trace_objs(obj_t **objs, arity_t n);
void  trace_vals(val_t *vals, arity_t n);
void  trace_noop(void *ptr, arity_t n);
void  mark_val(val_t  val);
void  mark_obj(obj_t *obj);
void  collect_garbage( void );

// method table api
bool get_method(func_t *func, arity_t n, val_t *args, meth_t **buf );
bool put_method(func_t *func, arity_t n, val_t *args, meth_t **buf );
bool pop_method(func_t *func, arity_t n, val_t *args, meth_t **buf );

// initialization
void mem_init( void );

// tags
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul
#define ISIGN     0x0000800000000000ul

#define OBJ       0x7ffc000000000000ul
#define NUL       0x7ffd000000000000ul
#define CHR       0x7ffe000000000000ul
#define BOOL      0x7fff000000000000ul
#define TYPE      0xfffc000000000000ul
#define OP        0xfffd000000000000ul
#define PTR       0xfffe000000000000ul
#define INT       0xffff000000000000ul

// value types
enum
  {
    // immediate types
    nul_type=1, chr_type, bool_type, type_type, op_type, ptr_type,
    
    // number types
    int_type, real_type,
    
    // root types
    none_type, any_type,

    // pair types
    cons_type, upval_type,

    // atom types
    atom_type,

    // function types
    func_type, macro_type, utype_type, dtype_type, cntl_type,
    
    // method and method table types
    meth_type, metht_type,
    
    // closure, code, and namespace types
    clo_type, code_type, envt_type, var_type,
    
    // array types
    vec_type, str_type, tuple_type, types_type, instr_type,

    // map types
    dict_type, set_type, dentry_type, sentry_type,
    
    // io types
    port_type,
    
    // vm types (each of these is a singleton)
    vm_type, heap_type, symt_type, readt_type, rentry_type,

    num_val_types
  };

// bit manipulation macros
#define val_tag(val)      ((val)&INT)
#define tag_val(val, tag) ((((val_data_t)(val)).as_val)|(tag))

#define as_char(val)       ((int)((val)&UINT32_MAX))
#define as_bool(val)       ((int)((val)&1ul))
#define as_float(val)      (float_bits((uint32_t)as_small(val)))
#define as_real(val)       (((val_data_t)(val)).as_real)

#define as_ptr(val)							\
  _Generic((val),							\
	   val_t:((val_data_t)(((val_t)(val))&~INT)).as_ptr,		\
	   default:(val))

int64_t as_int(val_t val)
{
  return (int64_t)((val&ISIGN) ? val : val&~INT);
}

#define is_obj(val)   (val_tag(val)==OBJ)
#define is_nul(val)   ((val)==NUL)
#define is_chr(val)   (val_tag(val)==CHR)
#define is_bool(val)  (((val)|1ul)==TRUE)
#define is_type(val)  (val_tag(val)==TYPE)
#define is_int(val)   (val_tag(val)==INT)
#define is_ptr(val)   (val_tag(val)==PTR)
#define is_arity(val) (val_tag(val)==ARITY)
#define is_real(val)  (((val)&QNAN)!=QNAN)

#define asa(t, v)   ((t##_t*)as_ptr(v))
#define isa(t, v)    (is_obj_type(v, t##_type))
#define getf(t, v, f)   (asa(t, v)->f)

// memory helpers
#define scrub(spc, n)              memset((spc), 0, (n))
#define scrubv(vec, n, type)       scrub((vec), (n) * sizeof(type))
#define allocv(n, type)            alloc((n) * sizeof(type))
#define deallocv(ptr, n, type)     dealloc((ptr), (n) * sizeof(type))
#define duplicatev(ptr, n, type)   duplicate((ptr), (n) * sizeof(type))
#define resizev(ptr, on, nn, type) resize((ptr), (on) * sizeof(type), (nn) * sizeof(type))
#define copyv(dst, src, n, type)   copy((dst), (src), (n) * sizeof(type))

#define safe_alloc(func, ...)						\
  ({									\
    void *_ptr_ = func( __VA_ARGS__ );					\
    if ( _ptr_ == NULL )						\
      {									\
	printf("allocation failed at %s:%d:%s.\n",			\
	       __FILE__,						\
	       __LINE__,						\
	       __func__);						\
	exit(1);							\
      }									\
    _ptr_;								\
  })

// basic descriptive macros
#define OFLAGP(F, val)					\
  _Generic((val),					\
	   val_t:is_val_##F,				\
	   obj_t*:is_obj_##F)((val))

#define IMPL_OFLAGP(F)				\
  bool is_val_##F(val_t val)			\
  {						\
    return is_obj(val)				\
      && is_obj_##F(as_obj(val));		\
  }						\
  bool is_obj_##F(obj_t *obj)			\
  {						\
    return obj					\
      && flag_p(obj->flags, obj_fl_##F);	\
  }

#define FLAGP(T, F, val)				\
  _Generic((val),					\
	   val_t:is_val_##F,				\
	   obj_t*:is_obj_##F,				\
	   T##_t*:is_##T##_##F)((val))

#define IMPL_FLAGP(T, F)			\
  bool is_val_##F(val_t val)			\
  {						\
    return is_obj(val)				\
      && is_obj_##F(as_obj(val));		\
  }						\
  bool is_obj_##F(obj_t *obj)			\
  {						\
    return obj					\
      && obj->type == T##_type			\
      && flag_p(obj->flags, T##_fl_##F );	\
  }						\
  bool is_##T##_##F(T##_t *T)			\
  {						\
    return T					\
      && flag_p(T->obj.flags, T##_fl_##F );	\
  }

#define IMPL_RTYPE_P(T, L, H)			\
  bool is_val_##T(val_t val);			\
  bool is_obj_##T(obj_t *obj);			\
  						\
  bool is_val_##T(val_t val)			\
  {						\
    return is_obj(val)				\
      && is_obj_##T(as_obj(val));		\
  }						\
  bool is_obj_##T(obj_t *obj)			\
  {						\
    return obj					\
      && obj_type(obj) >= L			\
      && obj_type(obj) <= H;			\
  }

#define IMPL_UTYPE_P(T, N, ...)			\
  bool is_val_##T(val_t val)			\
  {						\
    static type_t members[N] = { __VA_ARGS__ };	\
    type_t t = typeof_val(val);			\
    for (size_t i=0;i<N;i++)			\
      {						\
	if (t == members[i])			\
	  return true;				\
      }						\
    return false;				\
  }						\
  bool is_obj_##T(obj_t *obj)			\
  {						\
    static type_t members[N] = { __VA_ARGS__ };	\
    type_t t = obj_type(obj);			\
    for (size_t i=0; i<N; i++)			\
      {						\
	if (t == members[i])			\
	  {					\
	    return true;			\
	  }					\
      }						\
    return false;				\
  }

#define TYPE_P(T, val)				\
  _Generic((val),				\
	   val_t:is_val_##T,			\
	   obj_t*:is_obj_##T)(val)


// type definitions
// base types
// object type
typedef enum
  {
    obj_fl_frozen=0x8000,
    obj_fl_array =0x4000,
    obj_fl_map   =0x2000,
    obj_fl_pair  =0x1000,
  } obj_fl_t;

struct obj_t
{
  obj_t      *next;
  type_t      type;
  uint16_t    flags;
  uint8_t     black;
  uint8_t     gray;
};

#define OBJ_HEAD				\
  obj_t obj;

#define as_obj(val)    asa(obj, val)

#define obj_type(val)  getf(obj,val,type)
#define obj_next(val)  getf(obj,val,next)
#define obj_black(val) getf(obj,val,black)
#define obj_gray(val)  getf(obj,val,gray)
#define obj_flags(val) getf(obj,val,flags)

// array type
struct arr_t
{
  OBJ_HEAD
  void    *data;
  arity_t  len;
  arity_t  cap;
};

#define arr_data(val) getf(arr,val,data)
#define arr_len(val)  getf(arr,val,len)
#define arr_cap(val)  getf(arr,val,cap)

struct map_t
{
  OBJ_HEAD
  obj_t   **data;
  arity_t   len;
  arity_t   cap;

  union
  {
    int8_t  *o8;
    int16_t *o16;
    int32_t *o32;
  };

  arity_t ocap;
  arity_t osize;
};

#define map_data(val)  getf(map,val,data)
#define map_len(val)   getf(map,val,len)
#define map_cap(val)   getf(map,val,ocap)
#define map_o8(val)    getf(map,val,o8)
#define map_o16(val)   getf(map,val,o16)
#define map_o32(val)   getf(map,val,o32)
#define map_ocap(val)  getf(map,val,ocap)
#define map_osize(val) getf(map,val,osize)

struct pair_t
{
  OBJ_HEAD
  val_t car;

  union
  {
    val_t   cdr;
    pair_t *tail;
  };
};

#define pair_car(val)   getf(pair,val,data)
#define pair_cdr(val)   getf(pair,val,cdr)
#define pair_tail(val)  getf(pair,val,tail)

IMPL_OFLAGP(frozen);
IMPL_OFLAGP(arr);
IMPL_OFLAGP(map);
IMPL_OFLAGP(pair);

#define is_frozen(val) OFLAGP(frozen, val)
#define is_arr(val)    OFLAGP(arr, val)
#define is_map(val)    OFLAGP(map, val)
#define is_pair(val)   OFLAGP(pair, val)

bool  is_obj_type( val_t x, type_t t )
{
  return is_obj(x)
    && as_obj(x)->type == t;
}

// cons types and macros
typedef enum
  {
    cons_fl_proper    =0x0001,
    upval_fl_closed   =0x0002,
  } cons_fl_t;

struct cons_t
{
  pair_t  pair;

  arity_t len;
  bool    proper;
};

struct upval_t
{
  pair_t  pair;
  arity_t index;
  bool    closed;
};

#define as_cons(val)     asa(cons, val)
#define is_cons(val)     isa(cons, val)

#define cons_car(val)    (getf(cons,val,pair).car)
#define cons_cdr(val)    (getf(cons,val,pair).cdr)
#define cons_tail(val)   (getf(cons,val,pair).tail)
#define cons_len(val)    getf(cons,val,len)
#define cons_type(val)   getf(cons,val,type)

IMPL_FLAGP(cons, proper);
IMPL_UTYPE_P(list, 2, nul_type, cons_type);

#define is_proper(val)    FLAGP(cons, proper, val)
#define is_list(val)      TYPE_P(list, val)

#define as_upval(val)     asa(upval,val)
#define is_upval(val)     isa(upval,val)

#define upv_val(val)      getf(upval,val,car)
#define upv_next(val)     getf(upval,val,cdr)
#define upv_index(val)    getf(upval,val,len)

IMPL_FLAGP(upval, closed);

#define is_closed(val) FLAGP(upval, closed, val)

// atom typedefs and macros
typedef enum
  {
    atom_fl_gensym =1,
    atom_fl_keyword=2,
  } atom_fl_t;

struct atom_t
{
  OBJ_HEAD
  str_t  *name;
  idno_t  idno;
  hash_t  hash;
};

#define as_atom(val)   asa(atom, val)
#define is_atom(val)   isa(atom, val)

#define atom_name(val) getf(atom,val,name)
#define atom_hash(val) getf(atom,val,hash)
#define atom_idno(val) getf(atom,val,idno)

IMPL_FLAGP(atom, gensym);
IMPL_FLAGP(atom, keyword);

#define is_gensym(val)  FLAGP(atom, gensym, val)
#define is_keyword(val) FLAGP(atom, keyword, val)

// array typedefs, macros, & globals
#define ARRAY_SLOTS(V)				\
  V *data;					\
  arity_t len;					\
  arity_t cap

#define STRING_SLOTS(V)				\
  ARRAY_SLOTS(V);				\
  hash_t hash

// array typedefs, macros, & globals
size_t MinCap[num_val_types] =
  {
    [vec_type]    = 8,   [str_type]   = 0,   [tuple_type]  = 0,
    [types_type]  = 1,   [instr_type] = 8,

    [dict_type]   = 8,   [set_type]   = 8,
    [symt_type]   = 256, [meth_type]  = 8,   [metht_type]  = 8,
    [readt_type]  = 256
  };

size_t ElSize[num_val_types] =
  {
    [vec_type]    = sizeof(val_t),   [str_type]    = sizeof(char_t),
    [tuple_type]  = sizeof(val_t),
    [stack_type]  = sizeof(val_t),   [obarr_type]  = sizeof(obj_t*),
    [types_type]  = sizeof(type_t),  [instr_type]  = sizeof(op_t),
    [ordarr_type] = sizeof(int32_t)
  };

pad_fn_t PadSize[num_val_types] =
{
  [vec_type]=pad_alist_size,  [str_type]=pad_alist_size,  [tuple_type]=pad_alist_size,

  [iobuf_type]=pad_stack_size, [stack_type]=pad_stack_size, [obarr_type]=pad_alist_size,
  [types_type]=pad_alist_size, [instr_type]=pad_stack_size, [ordarr_type]=pad_table_size,
};

typedef enum
  {
    arr_fl_sint8     =0x0001,
    arr_fl_uint8     =0x0002,
    arr_fl_sint16    =0x0003,
    arr_fl_uint16    =0x0004,
    arr_fl_sint32    =0x0005,
    arr_fl_uint32    =0x0006,
    arr_fl_float32   =0x0007,
    arr_fl_sint64    =0x0008,
    arr_fl_uint64    =0x0009,
    arr_fl_pointer   =0x000a,
    arr_fl_float64   =0x000b,

    arr_Ctype_mask   =0x000f,

    arr_fl_unencoded =0x0000,
    arr_fl_ascii     =0x0011,
    arr_fl_latin1    =0x0012,
    arr_fl_utf8      =0x0021,
    arr_fl_utf16     =0x0013,
    arr_fl_utf32     =0x0015,

    arr_encoding_mask=0x003f,

    arr_fl_hashed    =0x0040,

    arr_fl_alist     =0x0100,
    arr_fl_stack     =0x0200,
    arr_fl_map       =0x0300,
    arr_fl_fixed     =0x0400,
  } arr_fl_t;

typedef enum
  {
    arr_err_ok       =0x0000,
    arr_err_underflow=0x0001,
    arr_err_overflow =0x0002,
    arr_err_frozen   =0x0003,
    arr_err_anull    =0x0004,
    arr_err_atype    =0x0005,
    arr_err_vtype    =0x0006,
  } arr_err_t;

struct arr_t    { OBJ_HEAD ARRAY_SLOTS(void); };
struct vec_t    { OBJ_HEAD ARRAY_SLOTS(val_t); };
struct str_t    { OBJ_HEAD STRING_SLOTS(char_t); };
struct tuple_t  { OBJ_HEAD ARRAY_SLOTS(val_t); obj_t *spec; };
struct types_t  { OBJ_HEAD STRING_SLOTS(type_t); };
struct instr_t  { OBJ_HEAD ARRAY_SLOTS(uint16_t); };

#define as_arr(val)          asa(arr, val)
#define as_vec(val)          asa(vec, val)
#define as_str(val)          asa(str, val)
#define as_tuple(val)        asa(tuple, val)
#define as_types(val)        asa(types, val)
#define as_instr(val)        asa(instr, val)

IMPL_RTYPE_P(arr, vec_type, str_type);

#define is_arr(val)          TYPE_P(arr, val)
#define is_vec(val)          isa(vec, val)
#define is_str(val)          isa(str, val)
#define is_tuple(val)        isa(tuple, val)
#define is_types(val)        isa(types, val)
#define is_instr(val)        isa(instr, val)

#define arr_flags(val)      (getf(arr,val,obj).flags)
#define arr_Ctype(val)      (getf(arr,val,obj).flags&arr_Ctype_mask)
#define arr_encoding(val)   (getf(arr,val,obj).flags&arr_encoding_mask)
#define arr_frozen(val)     (getf(arr,val,obj).flags&arr_fl_frozen)
#define arr_hashed(val)     (getf(arr,val,obj).flags&arr_fl_hashed)

#define arr_data(val)        getf(arr,val,data)
#define arr_len(val)         getf(arr,val,len)
#define arr_cap(val)         getf(arr,val,cap)

#define vec_flags(val)      (getf(vec,val,obj).flags)
#define vec_frozen(val)     (getf(vec,val,obj).flags&arr_fl_frozen)
#define vec_data(val)        getf(vec,val,data)
#define vec_len(val)         getf(vec,val,len)
#define vec_cap(val)         getf(vec,val,cap)

#define str_flags(val)      (getf(str,val,obj).flags)
#define str_encoding(val)   (getf(str,val,obj).flags&arr_encoding_mask)
#define str_Ctype(val)      (getf(str,val,obj).flags&arr_Ctype_mask)
#define str_hashed(val)     (getf(str,val,obj).flags&arr_fl_hashed)
#define str_data(val)        getf(str,val,data)
#define str_len(val)         getf(str,val,len)
#define str_cap(val)         getf(str,val,cap)
#define str_hash(val)        getf(str,val,hash)

#define tuple_flags(val)    (getf(tuple,val,obj).flags)
#define tuple_data(val)      getf(tuple,val,data)
#define tuple_len(val)       getf(tuple,val,len)
#define tuple_cap(val)       getf(tuple,val,cap)
#define tuple_hash(val)      getf(tuple,val,hash)
#define tuple_spec(val)      getf(tuple,val,spec)

#define iobuf_flags(val)    (getf(iobuf,val,obj).flags)
#define iobuf_encoding(val) (getf(iobuf,val,obj).flags&arr_Ctype_mask)
#define iobuf_Ctype(val)    (getf(iobuf,val,obj).flags&arr_encoding_mask)
#define iobuf_data(val)      getf(iobuf,val,data)
#define iobuf_len(val)       getf(iobuf,val,len)
#define iobuf_cap(val)       getf(iobuf,val,cap)

#define iobuf_data(val)      getf(iobuf,val,data)
#define iobuf_len(val)       getf(iobuf,val,len)
#define iobuf_cap(val)       getf(iobuf,val,cap)

#define stack_flags(val)    (getf(stack,val,obj).flags)
#define stack_data(val)      getf(stack,val,data)
#define stack_len(val)       getf(stack,val,len)
#define stack_cap(val)       getf(stack,val,cap)

#define obarr_flags(val)    (getf(obarr,val,obj).flags)
#define obarr_frozen(val)   (getf(obarr,val,obj).flags&arr_fl_frozen)
#define obarr_data(val)      getf(obarr,val,data)
#define obarr_len(val)       getf(obarr,val,len)
#define obarr_cap(val)       getf(obarr,val,cap)

#define types_flags(val)    (getf(types,val,obj).flags)
#define types_hashed(val)   (getf(types,val,obj).flags&arr_fl_hashed)
#define types_data(val)      getf(types,val,data)
#define types_len(val)       getf(types,val,len)
#define types_cap(val)       getf(types,val,cap)
#define types_hash(val)      getf(types,val,hash)

#define instr_flags(val)    (getf(instr,val,obj).flags)
#define instr_frozen(val)   (getf(instr,val,obj).flags&arr_fl_frozen)
#define instr_data(val)      getf(instr,val,data)
#define instr_len(val)       getf(instr,val,len)
#define instr_cap(val)       getf(instr,val,cap)

#define ordarr_flags(val)    (getf(ordarr,val,obj).flags)
#define ordarr_data(val)      getf(ordarr,val,data)
#define ordarr_len(val)       getf(ordarr,val,len)
#define ordarr_cap(val)       getf(ordarr,val,cap)

// array description and generic macros
#define AGENERIC(op, val)				\
  _Generic((val),					\
	   vec_t*:vec_##op,				\
	   str_t*:str_##op,				\
	   tuple_t*:tuple_##op,				\
	   iobuf_t*:iobuf_##op,				\
	   stack_t*:stack_##op,				\
	   obarr_t*:obarr_##op,				\
	   types_t*:types_##op,				\
	   instr_t*:instr_##op,				\
	   ordarr_t*:ordarr_##op)

#define AIMPL(OP, ...)				\
  OP(vec, val_t);				\
  OP(str, char_t);				\
  OP(tuple, val_t);				\
  OP(iobuf, char_t);				\
  OP(stack, val_t);				\
  OP(obarr, obj_t*);				\
  OP(types, type_t);				\
  OP(instr, op_t);				\
  OP(ordarr, order_t)

#define AREF(T, V)							\
  arr_err_t T##_ref(T##_t *arr, order_t n, V *buf)			\
  {									\
    if (n < 0)								\
      {									\
	n += arr->len;							\
	if (n < 0)							\
	  return arr_err_underflow;					\
      }									\
    if ((arity_t)n >= arr->len)						\
      return arr_err_overflow;						\
    if (buf)								\
      *buf = arr->data[n];						\
    return arr_err_ok;							\
  }

#define ASET(T, V)						\
  arr_err_t T##_set(T##_t *arr, order_t n, V v, V *buf )	\
  {								\
    if (n < 0)							\
      n += arr->len;						\
    if (n < 0)							\
      return arr_err_underflow;					\
    if ((arity_t)n >= arr->len)					\
      return arr_err_overflow;					\
    if (flag_p(arr->obj.flags, arr_fl_frozen ))			\
      return arr_err_frozen;					\
    if (buf)							\
      *buf = arr->data[n];					\
    arr->data[n] = v;						\
    return arr_err_ok;						\
  }

#define APUSH(T, V)							\
  arr_err_t T##_push(T##_t *arr, V v, arity_t *loc)			\
  {									\
    if (flag_p(arr->obj.flags, arr_fl_frozen))				\
      return arr_err_frozen;						\
    if (loc)								\
      *loc = arr->len;							\
    arr->data[arr->len] = v;						\
    resize_array((arr_t*)arr, arr->len+1 );				\
    return arr_err_ok;							\
 }

#define APUSHN(T, V)						       \
  arr_err_t T##_pushn(T##_t *arr, arity_t n, arity_t *loc)	       \
  {								       \
    if (flag_p(arr->obj.flags, arr_fl_frozen))			       \
      return arr_err_frozen;					       \
    if (loc)							       \
      *loc = arr->len;						       \
    resize_array((arr_t*)arr, arr->len+n);			       \
    return arr_err_ok;						       \
  }

#define APOP(T, V)							\
  arr_err_t T##_pop(T##_t *arr, V *buf, arity_t *loc)			\
  {									\
    if (flag_p(arr->obj.flags, arr_fl_frozen))				\
      return arr_err_frozen;						\
    if (arr->len == 0)							\
      return arr_err_underflow;						\
    if (buf)								\
      *buf = arr->data[arr->len-1];					\
    if (loc)								\
      *loc = arr->len-1;						\
    resize_array((arr_t*)arr, arr->len-1);				\
    return arr_err_ok;							\
  }

#define APOPN(T, V)							\
  arr_err_t T##_popn(T##_t *arr, arity_t n, V *buf, arity_t *loc)	\
  {									\
    if (flag_p(arr->obj.flags, arr_fl_frozen))				\
      return arr_err_frozen;						\
    if (arr->len < n)							\
      return arr_err_underflow;						\
    if (buf)								\
      *buf = arr->data[arr->len-1];					\
    if (loc)								\
      *loc = arr->len-n;						\
    resize_array((arr_t*)arr, arr->len-n);				\
    return arr_err_ok;							\
  }

#define AREADN(T, V)						\
  arr_err_t T##_readn( T##_t *arr, V *src, arity_t n)		\
  {								\
    if (flag_p(arr->obj.flags, arr_fl_frozen))			\
      return arr_err_frozen;					\
    arity_t offset = arr->len;					\
    resize_array((arr_t*)arr, arr->len+n );			\
    memcpy( arr->data + offset, src, n * sizeof(V) );		\
    return arr_err_ok;						\
  }

#define AWRITEN(T, V)						\
  arity_t T##_writen( V *dst, T##_t *src, arity_t n)		\
  {			 					\
    n = min(n, src->len);					\
    memcpy( dst, src->data, n * sizeof(V) );			\
    return arr_err_ok;						\
  }

AIMPL(AREF);
AIMPL(ASET);
AIMPL(APUSH);
AIMPL(APUSHN);
AIMPL(APOP);
AIMPL(APOPN);
AIMPL(AREADN);
AIMPL(AWRITEN);

#define aref(val, n, buf)       AGENERIC(val)((val), (n), (buf))
#define aset(val, n, v, buf)    AGENERIC(val)((val), (n), (v), (buf))
#define apush(val, v, buf)      AGENERIC(val)((val), (v), (buf))
#define apushn(val, n, loc)     AGENERIC(val)((val), (n), (loc))
#define apop(val, buf, loc)     AGENERIC(val)((val), (buf), (loc))
#define apopn(val, n, buf, loc) AGENERIC(val)((val), (n), (buf), (loc))
#define areadn(val, src, n)     AGENERIC(val)((val), (src), (n))
#define awriten(dst, src, n)    AGENERIC(src)((dst), (src), (n))

void resize_array( arr_t *array, size_t newl)
{
  type_t atype       = array->obj.type;
  pad_fn_t adjust    = PadSize[atype];
  size_t oldl        = array->len, oldc = array->cap, minc = MinCap[atype];
  size_t newc        = adjust(oldl, newl, oldc, minc);

  if (newc != oldc)
    {
      size_t elsize = ElSize[atype];
      array->data   = resize(array->data, oldc*elsize, newc*elsize);
    }

  array->len = newl;
}

// mapping typedefs, macros, and globals

typedef union
{
  int8_t  *o8;
  int16_t *o16;
  int32_t *o32;
} map_order_t;

#define MAP_SLOTS(V)				\
  V           *data;				\
  arity_t      len;				\
  arity_t      cap;				\
  map_order_t  order;				\
  arity_t      ocap;				\
  arity_t      osize


#define ENTRY_SLOTS(K, key, V, val)		\
  K key;					\
  V val;					\
  hash_t hash

struct map_t  { OBJ_HEAD MAP_SLOTS(obj_t*); };
struct dict_t { OBJ_HEAD MAP_SLOTS(obj_t*); };
struct set_t  { OBJ_HEAD MAP_SLOTS(obj_t*); };
struct symt_t { OBJ_HEAD MAP_SLOTS(atom_t*); idno_t idno; };

#define map_entries(val) (getf(map,val,obarr).data)
#define map_order(val)    getf(map,val,data)
#define map_len(val)      getf(map,val,len)
#define map_cap(val)      getf(map,val,cap)

void resize_map( map_t *map, size_t newl )
{
  size_t oldl = map->;
  resize_array((arr_t*)&map->entries, newl );
  resi
}

// function typedefs and macros
typedef enum
  {
    // function type discriminator
    func_fl_common  =0x000, // common function with usual semantics
    func_fl_script  =0x001, // function is a module body
    func_fl_macro   =0x002, // a syntax expander
    func_fl_dtype   =0x003,
    func_fl_utype   =0x004,
    func_fl_cntl    =0x005,
  } func_fl_t;

struct func_t
{
  OBJ_HEAD

  func_t  *module;
  str_t   *name;
  metht_t *methods;
};

struct cntl_t
{
  OBJ_HEAD
  
};

struct utype_t
{
  func_t   func;
  types_t *members;
};

struct dtype_t
{
  func_t   func;
  types_t *signature;
  dict_t  *slots;
};

#define as_func(val)     asa(func, val)
#define is_func(val)     isa(func, val)

#define func_name(val)    getf(func,val,name)
#define func_module(val)  getf(func,val,module)
#define func_methods(val) getf(func,val,methods)

// function implementation types
typedef enum
  {
    meth_fl_none     = 0x000, // no method matching the current signature
    meth_fl_primitive= 0x001,
    meth_fl_native   = 0x002,
    meth_fl_bytecode = 0x003,
    meth_fl_vargs    = 0x004
  } meth_fl_t;


#define as_meth(val)   asa(meth, val)
#define is_meth(val)   isa(meth, val)

#define as_metht(val)  asa(metht, val)
#define is_metht(val)  isa(metht, val)

#define as_mcache(val) ((mcache_t*)as_obj(val))
#define as_mlevel(val) ((mlevel_t*)as_obj(val))

// vec, dict, & set typedefs and macros
// internal types
struct envt_t
{
  OBJ_HEAD
  envt_t *next;
  
  set_t  *locals;
  set_t  *upvals;

  dict_t *imports;
  dict_t *module;
  dict_t *syntax;
};

struct code_t
{
  OBJ_HEAD
  func_t  *module;
  vec_t   *vals;
  instr_t *code;
};

// utility array types
// vm types
struct vm_t
{
  stack_t   stack;

  code_t   *code;
  envt_t   *envt;

  op_t     *ip;
  arity_t   bp;    // arguments
  arity_t   cp;    // control pointer (index of nearest enclosing continuation prompt)

  vec_t    *upvals;
  cons_t   *openupv;
};

struct heap_t
{
  obj_t    *objects;
  obarr_t  *gray_stack;

  size_t    allocated;
  size_t    next_gc;

  bool_t collecting;
  float  heap_grow_factor;
};

// globals
vm_t Vm;
heap_t Heap;
symt_t Symbols;
envt_t Toplevel;

  
// static utilities
type_t typeof_val(val_t x)
{
  switch (val_tag(x))
    {
    case OBJ:           return obj_type(x);
    case NUL:           return nul_type;
    case CHR:           return chr_type;
    case BOOL:          return bool_type;
    case TYPE:          return type_type;
    case PTR ... INT:   return int_type;
    default:            return real_type;
    }
}

// memory
typedef enum
  {
    mem_fl_sharing  =0x8000,
    mem_fl_protected=0x4000,
    mem_fl_static   =0x2000,
  } mem_fl_t;

#define mark(val)				\
  _Generic((val),				\
	   val_t:mark_val,			\
	   obj_t*:mark_obj)(val)

#define trace(val, ...)						\
  _Generic((val),						\
	   obj_t*:trace_obj,					\
	   val_t:trace_val,					\
	   obj_t**:trace_objs,					\
	   val_t*:trace_vals,					\
	   default:trace_noop)((val) __VA_OPT__(,) __VA_ARGS__)

// memory implementation
// methods & dispatch tables
void mark_cons(obj_t *obj);
void mark_upval(obj_t *obj);

void mark_atom(obj_t *obj);

void mark_func(obj_t *obj);
void mark_utype(obj_t *obj);

void mark_vec(obj_t *obj);
void mark_map(obj_t *obj);
void mark_port(obj_t *obj);
void mark_code(obj_t *obj);
void mark_clo(obj_t *obj);
void mark_envt(obj_t *obj);
void mark_meth(obj_t *obj);
void mark_cntl(obj_t *obj);
void mark_entry(obj_t *obj);
void mark_var(obj_t *obj);
void mark_symt(obj_t *obj);
void mark_metht(obj_t *obj);
void mark_vm(obj_t *obj);
void mark_heap(obj_t *obj);

#define NOMARK(type) [type##_type] = NULL
#define MARK(type) [type##_type] = mark_##type

mark_fn_t Mark[num_val_types] =
  {
    NOMARK(none), NOMARK(any), NOMARK(nul), NOMARK(int), NOMARK(real),

    NOMARK(type), NOMARK(bool), NOMARK(chr),
    
    MARK(cons), MARK(upval),

    MARK(atom),

    
  };

// static helpers -------------------------------------------------------------
static void guard_gc( size_t n )
{
  if (Heap.allocated + n >= Heap.next_gc)
    collect_garbage();
}

// core allocation functions --------------------------------------------------
void *alloc(size_t n)
{
  assert( n > 0 );
  guard_gc( n );
  void* out = safe_alloc( malloc, n );
  return out;
}

void dealloc(void *ptr, size_t n)
{
  assert( n > 0 );
  Heap.allocated -= n;
  free(ptr);
}

void *resize_bytes( void *ptr, size_t old_n, size_t new_n )
{
  bool   grew = new_n > old_n;
  size_t diff = grew ? new_n - old_n : old_n - new_n;
  
  if (grew)
    guard_gc( diff );

  ptr = safe_alloc( realloc, ptr, new_n );

  if (grew)
    Heap.allocated += diff;

  else
    Heap.allocated -= diff;

  return ptr;
}

void copy(void *dst, void *src, size_t n)
{
  memcpy( dst, src, n );
}


// memory management ----------------------------------------------------------
void mark_val(val_t val)
{
  if (val_tag(val) == OBJ)
    mark_obj(as_obj(val));
}

void mark_obj(obj_t *obj)
{
  if (obj == NULL)
    return;

  if (obj->black)
    return;

  obj->black = true;

  if (Mark[obj->type])
      push( Heap.gray_stack, obj );

 else
   obj->gray = false;
}

void trace_val(val_t val)
{
  trace_obj(as_obj(val));
}

void trace_vals(val_t *vals, arity_t n)
{
  for (arity_t i=0; i<n; i++)
    mark(vals[i]);
}

void trace_objs(obj_t **objs, arity_t n)
{
  for (arity_t i=0; i<n; i++)
    mark(objs[i]);
}

void trace_noop(void *spc, arity_t n)
{
  (void)spc;
  (void)n;
}

// gc toplevel
static void gc_roots( void );
static void gc_mark( void );
static void gc_sweep( void );
static const float gc_load_factor = 0.75;

void collect_garbage( void )
{
  Heap.collecting = true;

  gc_roots();
  gc_mark();
  gc_sweep();

  Heap.collecting = false;

  if (Heap.allocated >= Heap.next_gc * gc_load_factor)
    Heap.next_gc *= 2;
}

static void gc_roots( void )
{
  // mark heap
  atom_mark();
}

static void gc_mark( void )
{
  while ((Heap.gray_stack)->len > 0)
    {
      obj_t  *obj  = pop( Heap.gray_stack );
      type_t type  = obj->type;
      obj->gray    = true;
    }
}

static void gc_sweep( void )
{
  obj_t **buffer = &Heap.objects;

  while (*buffer != NULL)
    {
      if ((*buffer)->black)
	{
	  (*buffer)->black = false;
	  (*buffer)->gray  = true;
	  buffer           = &((*buffer)->next);
	}

      else
	finalize( buffer );
    }
}

// objs_t implementation
// implementation and fallback methods for object types
void init_obj(obj_t *obj, type_t type, flags_t fl)
{
  obj->type  = type;
  obj->gray  = true;
  obj->black = false;
  obj->flags = fl&UINT16_MAX;

  if (flag_p(fl, mem_fl_static))
    obj->next = NULL;

  else
    {
      obj->next     = Heap.objects;
      Heap.objects = obj;
    }
}

// object apis
// pending
static const arity_t  symt_min_cap = 256;
static const char    *gsfmt        = "%s#%zu";
static const char    *gsfallback   = "symbol";

// api
atom_t *new_atom( char *name, hash_t h, bool hp, bool gensym )
{
  hash_t self_hash;
  
  if (name == NULL)
    {
      assert(gensym);
      name = (char*)fallback;
    }

  if (!hp)
    {
      h = hash_string(name);
    }

  self_hash = h;
  self_hash = mix_hashes( Symbols.counter, self_hash );
  self_hash = mix_hashes( atom_type, self_hash );
  
  arity_t n  = strlen(name);
  flags_t fl = 0 | atom_fl_gensym*gensym | atom_fl_keyword*(name[0]==':');

  str_t *name_str = new_str( enc_ascii, n, name );

  atom_t *out = alloc(sizeof(atom_t));

  init_obj((obj_t*)out, atom_type, fl );

  out->name = name_str;
  out->hash = self_hash;
  out->idno = Symbols.counter++;

  return out;
}

val_t intern( char *name )
{
  val_t string = tag_val((void*)name, PTR );
  kv_t *buffer;

  table_put( &Symbols.table, string, (obj_t**)&buffer );
}

// pair types

// api
void    init_cons( cons_t *new, val_t hd, val_t tl, type_t eltype )
{
  init_obj((obj_t*)new,
	   cons_type,
	   (tl == NUL || is_proper(tl))*cons_fl_proper );

  new->car     = hd;
  new->cdr     = tl;
  new->eltype = eltype;

  if (is_cons(tl))
    new->len = as_cons(tl)->len + 1;

  else
    new->len = 1;
}

void init_kv( kv_t *new, val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  init_cons((cons_t*)new, k, b, t );
  new->cons.len = i;
  new->hash = h;
}

void init_var( var_t *new, val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  init_kv( (kv_t*)new, k, b, t, i, h );
  new->doc  = empty_str;
  new->meta = (obj_t*)empty_table;
}

cons_t *new_cons( val_t hd, val_t tl )
{
  cons_t *out = alloc( sizeof(cons_t) );
  init_cons( out, hd, tl, any_type );
  return out;
}

kv_t *new_kv( val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  kv_t *out = alloc(sizeof(kv_t));
  init_kv( out, k, b, t, i, h );
  return out;
}

var_t *new_var( val_t k, val_t b, type_t t, arity_t i, hash_t h )
{
  var_t *out = alloc(sizeof(var_t));
  init_var( out, k, b, t, i, h );
  return out;
}

// utilities
val_t ncat(val_t xs, val_t ys)
{
  if (xs == NUL)
    return ys;

  if (ys == NUL)
    return xs;

  arity_t inc = as_cons(ys)->len;

  for (val_t tmp=xs;;tmp=as_cons(xs)->cdr)
    {
      as_cons(tmp)->len += inc;

      if (!is_cons(as_cons(tmp)->cdr))
	{
	  as_cons(tmp)->cdr = ys;
	  break;
	}
    }
  return xs;
}

val_t xar( val_t xs, val_t v )
{
  if (!!(as_obj(xs)->flags & 0x1e))
    {
      cons_t *copy = new_cons(v, as_cons(xs)->cdr);
      xs = tag_val((obj_t*)copy, OBJ);
    }

  else
    as_cons(xs)->car = v;

  return xs;
}

val_t xdr( val_t xs, val_t v )
{
  if (is_proper(xs))
    {
      cons_t *copy = new_cons(as_cons(xs)->car, v);
      xs = tag_val((obj_t*)copy, OBJ);
    }

  else
    {
      as_cons(xs)->cdr = v;
      as_cons(xs)->len = 1;

      if (is_cons(v))
	as_cons(xs)->len += as_cons(v)->len;

    }
  return xs;
}

val_t nrev(val_t xs)
{
  if (is_cons(xs) && as_cons(xs)->len > 1)
    {
      val_t last=NUL;
      arity_t len = as_cons(xs)->len;

      for (arity_t i=1; i <= len; i++)
      	{
	  as_cons(xs)->len = i;
	  val_t tmp        = as_cons(xs)->cdr;
	  as_cons(xs)->cdr  = last;
	  last             = xs;
	  xs               = tmp;
	}
    }

  return xs;
}
