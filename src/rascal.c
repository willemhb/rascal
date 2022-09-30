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

#define val_tag(x)  ((x)&ARITY)

#define OBJ_HEAD				\
  obj_t object;


typedef uintptr_t  val_t;

typedef uint32_t   type_t;
typedef uint16_t   opcode_t;

typedef struct obj_t   obj_t;

typedef struct cons_t  cons_t;
typedef struct entry_t entry_t;
typedef struct var_t   var_t;

typedef struct atom_t  atom_t;

typedef struct func_t  func_t;
typedef struct cntl_t  cntl_t;

typedef struct bin_t   bin_t;
typedef struct str_t   str_t;

typedef struct vec_t   vec_t;
typedef struct map_t   map_t;

typedef struct map_t dict_t;
typedef struct map_t set_t;

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

// value types
enum
  {
    none_type=1,
    any_type,

    int_type,
    real_type,
    type_type,
    bool_type,
    chr_type,
    nul_type,

    // core user types
    cons_type,
    atom_type,
    func_type,
    vec_type,
    dict_type,
    set_type,
    str_type,
    bytes_type,
    port_type,

    // internal types
    code_type,
    clo_type,
    envt_type,
    meth_type,
    cntl_type,
    entry_type,
    var_type,

    // vm types
    symt_type,
    metht_type,
    vm_type,
    heap_type,

    num_val_types
  };

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

// builtin types
struct obj_t
{
  obj_t      *next;
  type_t      type;
  uint16_t    flags;
  uint8_t     black;
  uint8_t     gray;
};

struct cons_t
{
  OBJ_HEAD
  val_t   car;
  val_t   cdr;
  arity_t len;
  type_t  type;
};

struct entry_t
{
  cons_t cons;
  hash_t hash;
};

#define entry_key(val)   (as_entry(val)->cons.car)
#define entry_bind(val)  (as_entry(val)->cons.cdr)
#define entry_order(val) (as_entry(val)->cons.arity)
#define entry_hash(val)  (as_entry(val)->hash)

struct var_t
{
  entry_t entry;
  dict_t *props;
  envt_t *ns;
};

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
  };
  arity_t len;
  Ctype_t Ctype;
  hash_t  hash;
};

struct str_t
{
  OBJ_HEAD
  union
  {
    char     *ascii;
    char     *latin1;
    char     *utf8;
    char16_t *utf16;
    char32_t *utf32;
  };

  arity_t     len;
  encoding_t  enc;
  hash_t      hash;
};

struct atom_t
{
  OBJ_HEAD
  str_t  *name;
  hash_t  hash;
  idno_t  idno;
};

struct func_t
{
  OBJ_HEAD

  str_t  *name;
  envt_t *ns;

  union
  {
    opcode_t     primitive;
    meth_t      *methods;
    native_fn_t  native;
  };
};

struct vec_t
{
  OBJ_HEAD
  ALIST_SLOTS(val_t);
};

typedef vec_t stack_t;

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

// convenience macros
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

#define asa(type, val) ((type##_t*)as_ptr(val))
#define isa(type, val) (is_obj_type(val, type_##type))

#define as_obj(val) ((obj_t*)as_ptr(val))
#define is_obj(val) (val_tag(val)==OBJ)

#define obj_type(val)  (as_obj(val)->type)
#define obj_next(val)  (as_obj(val)->next)
#define obj_black(val) (as_obj(val)->black)
#define obj_gray(val)  (as_obj(val)->gray)
#define obj_flags(val) (as_obj(val)->flags)

#define as_cons(val)   asa(cons, val)
#define as_atom(val)   asa(atom, val)
#define as_func(val)   asa(func, val)
#define as_vec(val)    asa(vec, val)
#define as_dict(val)   asa(dict, val)

#define atom_name(val) (as_atom(val)->name)

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
  

static inline bool is_obj_type( val_t x, type_t t ) { return is_obj(x) && obj_type(x) == t; }

static inline int64_t as_int(val_t val) { return ((val_data_t)(val)).as_int; }

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

    [code_type] = mark_code, [clo_type] = mark_clo,
    envt_type,
    meth_type,
    cntl_type,
    entry_type,
    var_type,

    // vm types
    symt_type,
    metht_type,
    vm_type,
    heap_type,			       
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

PAD_STACK_SIZE(objs, obj_t*)
NEW_ARRAY(objs, obj_t*)
INIT_ARRAY(objs, obj_t*)
RESIZE_ARRAY(objs, obj_t*)
FINALIZE_ARRAY(objs, obj_t*)
MARK_ARRAY(objs)

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
  val_t string = tag_val((void*)name, POINTER);
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
