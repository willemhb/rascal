#include "core.h"

#include "template/table.h"
#include "template/arr.h"
#include "template/str.h"

#include "utils/str.h"
#include "utils/num.h"
#include "utils/arr.h"
#include "utils/table.h"

#include <uchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>


// type declarations
typedef uintptr_t  val_t;

typedef uint32_t   type_t;
typedef uint16_t   opcode_t;

typedef struct obj_t   obj_t;

typedef struct cons_t  cons_t;
typedef struct cons_t  upval_t;
typedef struct entry_t entry_t;
typedef struct var_t   var_t;

typedef struct atom_t  atom_t;

typedef struct func_t  func_t;
typedef struct cntl_t  cntl_t;
typedef struct utype_t utype_t;
typedef struct dtype_t dtype_t;

typedef struct arr_t   arr_t;
typedef struct bin_t   bin_t;
typedef struct vec_t   vec_t;
typedef struct bin_t   str_t;
typedef struct bin_t   instr_t;

typedef struct map_t   map_t;

typedef struct map_t   dict_t;
typedef struct map_t   set_t;

// internal types
typedef struct code_t  code_t;
typedef struct clo_t   clo_t;
typedef struct envt_t  envt_t;
typedef struct meth_t  meth_t;

// vm types
typedef struct iobuf_t iobuf_t;
typedef struct sig_t   sig_t;
typedef struct symt_t  symt_t;
typedef struct vm_t    vm_t;
typedef struct heap_t  heap_t;
typedef struct metht_t metht_t;

// function pointer types
typedef hash_t  (*hash_fn_t)(val_t val);
typedef int     (*ord_fn_t)(val_t x, val_t y);
typedef arity_t (*prin_fn_t)(obj_t *io, val_t x);
typedef val_t   (*call_fn_t)(val_t f, val_t *args, arity_t n);
typedef int     (*init_fn_t)(void *spc, val_t val);
typedef void    (*finalize_fn_t)(obj_t **buf);
typedef void    (*mark_fn_t)(obj_t *obj);
typedef obj_t  *(*resize_fn_t)(obj_t *obj, arity_t n);

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

  struct
  {
    int_t as_int : 48;
    int_t        : 16;
  };
} val_data_t;

// forward declarations
static inline int64_t as_int(val_t val);
static inline type_t  typeof_val(val_t val);
static inline bool    is_obj_type( val_t x, type_t t );
static inline bool    is_proper(val_t x);
static inline bool    is_list(val_t x);


size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc);
size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc);
void   resize_array(arr_t *array, size_t newl, size_t elsize, size_t minc);


// tags
#define QNAN      0x7ffc000000000000ul
#define SIGN      0x8000000000000000ul

#define OBJ       0x7ffc000000000000ul
#define NUL       0x7ffd000000000000ul
#define CHR       0x7ffe000000000000ul
#define BOOL      0x7fff000000000000ul
#define TYPE      0xfffc000000000000ul
#define INT       0xfffd000000000000ul
#define PTR       0xfffe000000000000ul
#define ARITY     0xffff000000000000ul

#define FALSE     (BOOL|0ul)
#define TRUE      (BOOL|1ul)
#define EOS       (CHR|EOF)
#define ZERO      (INT|0ul)
#define ONE       (INT|1ul)
#define NULPTR    (PTR|0ul)

// builtin types

// value types
enum
  {
    // root types
    none_type=1, any_type,

    // immediate types
    nul_type, int_type, real_type, type_type, bool_type, chr_type,

    // pair types
    cons_type, upval_type, entry_type, var_type,

    // atom & namespace types
    atom_type, symt_type, envt_type, clo_type,

    // function and method types
    func_type, code_type, meth_type, metht_type, cntl_type,

    // array types
    vec_type, stack_type, instr_type, str_type, buffer_type,

    // set and mapping types
    dict_type, set_type,

    // io types
    port_type,

    // vm types
    vm_type, heap_type,

    num_val_types
  };

// bit manipulation macros
#define val_tag(val)      ((val)&ARITY)
#define tag_val(val, tag) ((((val_data_t)(val)).as_val)|(tag))

#define as_char(val)       ((int)((val)&UINT32_MAX))
#define as_bool(val)       ((int)((val)&1ul))
#define as_float(val)      (float_bits((uint32_t)as_small(val)))
#define as_real(val)       (((val_data_t)(val)).as_real)

#define as_ptr(val)							\
  _Generic((val),							\
	   val_t:((val_data_t)(((val_t)(val))&~ARITY)).as_ptr,		\
	   default:(val))

static inline int64_t as_int(val_t val) { return ((val_data_t)(val)).as_int; }

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

// object header
struct obj_t
{
  obj_t      *next;
  type_t      type;
  uint16_t    flags;
  uint8_t     black;
  uint8_t     gray;
};

#define as_obj(val)  asa(obj, val)

#define obj_type(val)  getf(obj,val,type)
#define obj_next(val)  getf(obj,val,next)
#define obj_black(val) getf(obj,val,black)
#define obj_gray(val)  getf(obj,val,gray)
#define obj_flags(val) getf(obj,val,flags)

// convenience macros
#define OBJ_HEAD				\
  obj_t object;

static inline bool  is_obj_type( val_t x, type_t t )
{
  return is_obj(x)
    && as_obj(x)->type == t;
}

// cons types and macros
typedef enum
  {
    cons_fl_proper    =0x0001,
    upval_fl_captured =0x0002,
  } cons_fl_t;

struct cons_t
{
  OBJ_HEAD
  val_t   car;
  val_t   cdr;
  arity_t len;
  type_t  eltype;
};

#define as_cons(val)     asa(cons, val)
#define is_cons(val)     isa(cons, val)

#define cons_car(val)    getf(cons,val,car)
#define cons_cdr(val)    getf(cons,val,cdr)
#define cons_len(val)    getf(cons,val,len)
#define cons_type(val)   getf(cons,val,type)


static inline bool is_proper(val_t x)
{
  return is_cons(x)
    && flag_p(as_obj(x)->flags, cons_fl_proper);
}


static inline bool is_list(val_t x)
{
  return x == NUL || is_cons(x);
}


struct entry_t
{
  cons_t cons;
  hash_t hash;
};

#define as_entry(val)    asa(entry, val)
#define is_entry(val)    isa(entry, val)

#define entry_key(val)   getf(cons,val,car)
#define entry_bind(val)  getf(cons,val,cdr)
#define entry_order(val) getf(cons,val,len)
#define entry_hash(val)  getf(entry,val,hash)

struct var_t
{
  entry_t entry;
  cons_t *props;
  envt_t *ns;
};

#define as_var(val)      asa(var, val)
#define is_var(val)      isa(var, val)

#define var_key(val)     getf(cons,val,car)
#define var_bind(val)    getf(cons,val,cdr)
#define var_order(val)   getf(cons,val,len)
#define var_hash(val)    getf(entry,val,hash)
#define var_props(val)   getf(var,val,props)
#define var_ns(val)      getf(var,val,ns)

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
  hash_t  hash;
  idno_t  idno;
};

#define as_atom(val)   asa(atom, val)
#define is_atom(val)   isa(atom, val)

#define atom_name(val) getf(atom,val,name)
#define atom_hash(val) getf(atom,val,hash)
#define atom_idno(val) getf(atom,val,idno)

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
  obj_t   *type;
  metht_t *methods;
};

#define as_func(val)     asa(func, val)
#define is_func(val)     isa(func, val)

#define func_name(val)   getf(func,val,name)
#define func_ns(val)     getf(func,val,ns)
#define func_prim(val)   getf(func,val,primitive)
#define func_meth(val)   getf(func,val,methods)
#define func_native(val) getf(func,val,native)

#define as_vec(val)    asa(vec, val)
#define as_dict(val)   asa(dict, val)
#define as_set(val)    asa(set, val)
#define as_symt(val)   asa(symt, val)
#define as_meth(val)   asa(meth, val)

// array typedefs, macros, & globals
size_t MinC[num_val_types] =
  {
    [vec_type]   = 1,   [str_type]   = 8, [buffer_type] = 32,
    [stack_type] = 256, [dict_type]  = 8, [set_type]    = 8,
    [symt_type]  = 256, [meth_type]  = 8, [metht_type]  = 8,
    [instr_type] = 64,
  };

typedef struct arr_t
{
  OBJ_HEAD

  union
  {
    void  *untagged;
    val_t *tagged;
  } data;
  
  arity_t len;
  arity_t cap;
} arr_t;

typedef enum
  {
    arr_fl_stack    =0x0040,
    arr_fl_encoded  =0x0080,
    arr_fl_protected=0x0100,
    arr_fl_hashed   =0x0200,
  } arr_fl_t;

size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  if (oldc >= newl && newl >= (oldc>>1))			
    return oldc;							
  arity_t newc = ((size_t)newl+(newl>>3)+6)&~(size_t)3;	
  if (newl - oldl > newc - oldc)				
    newc = ((size_t)newl+3)&~(size_t)3;

  if (newc < minc)
    newc = minc;

  return newc;
}

size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;
  if (newc < minc)
    newc = minc;
  if (newl > oldl)						
    while (newl > newc)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < (newc >> 1))
      newc >>= 1;
  return newc;
}

void resize_array( arr_t *array, size_t newl, size_t elsize, size_t minc )
{
  size_t oldc = array->cap,
    oldl = array->len,
    newc;

  size_t (*pad)(size_t, size_t, size_t, size_t ) = flag_p(array->object.flags, arr_fl_stack) ?
    pad_stack_size
    : pad_alist_size;
    
  newc = pad( )

  if (newc != oldc)
    array->data = resize( array->data, oldc*elsize, newc*elsize );

  array->len = newl;
}

struct vec_t
{
  OBJ_HEAD
  ALIST_SLOTS(val_t);
};

typedef vec_t stack_t;

struct bin_t
{
  OBJ_HEAD
  union
  {
    uint8_t  *data;
    uint8_t  *u8;
    int8_t   *i8;
    uint16_t *u16;
    int16_t  *i16;
    uint32_t *u32;
    int32_t  *i32;
    float    *f32;
    int64_t  *i64;
    double   *f64;
    void    **p64;
    char     *ascii;
    char     *latin1;
    char     *utf8;
    char16_t *utf16;
    char32_t *utf32;
  };
  arity_t len;
  arity_t cap;
  hash_t  hash;
};

#define as_bin(val)    asa(bin,val)
#define is_bin(val)    isa(bin,val)

#define bin_Ctype(val) (obj_flags(val)&0xf)
#define bin_len(val)   getf(bin,val,len)
#define bin_cap(val)   getf(bin,val,cap)
#define bin_hash(val)  getf(bin,val,hash)

#define bin_data(val)  getf(bin,val,data)
#define bin_u8(val)    getf(bin,val,u8)
#define bin_i8(val)    getf(bin,val,i8)
#define bin_u16(val)   getf(bin,val,u16)
#define bin_i16(val)   getf(bin,val,i16)
#define bin_u32(val)   getf(bin,val,u32)
#define bin_i32(val)   getf(bin,val,i32)
#define bin_f32(val)   getf(bin,val,f32)
#define bin_i64(val)   getf(bin,val,i64)
#define bin_p64(val)   getf(bin,val,p64)

#define as_str(val)     asa(str, val)
#define is_str(val)     isa(str, val)

#define str_enc(val)   (obj_flags(val)&enc_mask)
#define str_data(val)   getf(str,val,ascii)
#define str_ascii(val)  getf(str,val,ascii)
#define str_latin1(val) getf(str,val,latin1)
#define str_utf8(val)   getf(str,val,utf8)
#define str_utf16(val)  getf(str,val,utf16)
#define str_utf32(val)  getf(str,val,utf32)

#define str_len(val)    getf(str,val,len)
#define str_cap(val)    getf(str,val,hash)
#define str_hash(val)   getf(str,val,hash)



// vec, dict, & set typedefs and macros
struct map_t
{
  ORDERED_TABLE_SLOTS(OBJ_HEAD, obj_t*);
};

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
  func_t *module;
  vec_t  *vals;
  bin_t  *code;
};

// utility array types
DECL_ALIST(objs);
ALIST_TYPE(objs, EMPTY_HEAD, obj_t*);
DECL_ALIST_API(objs, obj_t*);

DECL_ALIST(vals);
ALIST_TYPE(vals, EMPTY_HEAD, val_t);
DECL_ALIST_API(vals, val_t);

// vm types
struct vm_t
{
  stack_t   stack;

  code_t   *code;
  envt_t   *envt;

  opcode_t *ip;
  arity_t   bp;    // arguments
  arity_t   cp;    // control pointer (index of nearest enclosing continuation prompt)

  vec_t    *upvals;
  cons_t   *openupv;
};

struct heap_t
{
  obj_t   *objects;
  objs_t  *gray_stack;

  size_t allocated;
  size_t next_gc;

  bool_t collecting;
  float  heap_grow_factor;
};

struct symt_t
{
  map_t  map;
  idno_t counter;
};

struct iobuf_t
{
  OBJ_HEAD
  union
  {
    uint8_t  *data;
    uint8_t  *u8;
    int8_t   *i8;
    uint16_t *u16;
    int16_t  *i16;
    uint32_t *u32;
    int32_t  *i32;
    float    *f32;
    int64_t  *i64;
    double   *f64;
  };
  arity_t len;
  arity_t cap;
  Ctype_t Ctype;
  
};

// globals
vm_t Vm;
heap_t Heap;
symt_t Symbols;
envt_t Toplevel;

  
// static utilities
static inline type_t typeof_val(val_t x)
{
  switch (val_tag(x))
    {
    case OBJ:           return obj_type(x);
    case NUL:           return nul_type;
    case CHR:           return chr_type;
    case BOOL:          return bool_type;
    case TYPE:          return type_type;
    case INT ... ARITY: return int_type;
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

// initialization
void mem_init( void );


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
void mark_atom(obj_t *obj)
{
  mark_obj((obj_t*)atom_name(obj));
}

void mark_cons(obj_t *obj);
void mark_func(obj_t *obj);
void mark_vec(obj_t *obj);
void mark_map(obj_t *obj);
void mark_port(obj_t *obj);
void mark_code(obj_t *obj);
void mark_clo(obj_t *obj);
void mark_envt(obj_t *obj);
void mark_meth(obj_t *obj);

mark_fn_t Mark[num_val_types] =
  {
   [none_type] = NULL,  [any_type] = NULL, [real_type] = NULL, [type_type] = NULL, [int_type] = NULL, 
   [bool_type] = NULL,  [chr_type] = NULL, [nul_type]  = NULL,
   
   [cons_type] = mark_cons, [atom_type] = mark_atom, [func_type] = mark_func,
   [vec_type]  = mark_vec, [dict_type] = mark_map,   [set_type]  = mark_map,
   
   [str_type] = NULL,      [bytes_type] = NULL,      [port_type] = mark_port,

   [code_type] = mark_code, [clo_type] = mark_clo,  [envt_type]  = mark_envt,
 
    [meth_type] = mark_meth,
    [cntl_type] = mark_cntl,
    [entry_type] = mark_entry,
    [var_type] = mark_var,

    // vm types
    [symt_type] = mark_symt,
    [metht_type] = mark_metht,
    [vm_type] = mark_vm,
    [heap_type] = mark_heap,			       
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


// utility type implementations
// vals
static const size_t vals_min_cap = 8;

PAD_STACK_SIZE(vals, val_t);
NEW_ARRAY(vals, val_t);
INIT_ARRAY(vals, val_t);
MARK_ARRAY(vals);
FINALIZE_ARRAY(vals, val_t);
RESIZE_ARRAY(vals, val_t);

// objs

// objs_t implementation
static const size_t objs_min_cap = 8;

PAD_STACK_SIZE(objs, obj_t*);
NEW_ARRAY(objs, obj_t*);
INIT_ARRAY(objs, obj_t*);
RESIZE_ARRAY(objs, obj_t*);
FINALIZE_ARRAY(objs, obj_t*);
MARK_ARRAY(objs);

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

  return kv_val(buffer);
}

bool is_keyword( val_t val )
{
  return is_atom(val)
    && flag_p(as_obj(val)->flags, atom_fl_keyword );
}

bool is_gensym( val_t val )
{
  return is_atom(val)
    && flag_p(as_obj(val)->flags, atom_fl_gensym );
}
