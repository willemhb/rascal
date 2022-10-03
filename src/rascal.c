// system includes
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>

// local includes
#include "common.h"
#include "utils.h"
#include "templates.h"

// typedefs
// representation types
typedef uintptr_t    val_t;
typedef struct obj_t obj_t;

// immediate types
typedef uint64_t  type_t;
typedef double    real_t;
typedef int64_t   int_t;
typedef char      char_t;

// object types
typedef struct atom_t atom_t;
typedef struct cons_t cons_t;
typedef struct port_t port_t;

// vm & envt types
typedef struct envt_t envt_t;
typedef struct var_t  var_t;

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
} val_data_t;

// base object type
// common object flags
typedef enum
  {
    obj_fl_traversed=0x8000,
  } obj_fl_t;

typedef struct obj_t
{
  obj_t    *next;
  flags16_t flags;
  flags8_t  gray;
  flags8_t  black;
  type_t    type;
} obj_t;

// symbol and symbol table
struct atom_t
{
  obj_t obj;
  ENTRY_SLOTS(char*, name, idno64_t, idno);
};

struct symt_t
{
  obj_t obj;
  ARRAY_SLOTS(atom_t*);
  idno64_t idno;
};

// pair types
typedef enum
  {
   cons_fl_proper=0x001,
   cons_fl_typed =0x002,
  } cons_fl_t;

struct cons_t
{
  obj_t   obj;

  val_t   car;
  val_t   cdr;

  arity32_t len;
  type_t    eltype;
};

// vm & envt types
struct var_t
{
  obj_t obj;
  ENTRY_SLOTS(atom_t*, name, val_t, bind); 
};

struct envt_t
{
  obj_t obj;
  ARRAY_SLOTS(var_t*);
};

// array types
struct alist_t
{
  obj_t obj;
  ARRAY_SLOTS(val_t);
};

struct stack_t
{
  obj_t obj;
  ARRAY_SLOTS(val_t);
};

// read table and reader entry
typedef struct rentry_t
{
  obj_t      obj;
  ENTRY_SLOTS(char32_t, dispatch, reader_fn_t, handler);
} rentry_t;

typedef struct readt_t
{
  obj_t obj;
  ARRAY_SLOTS(rentry_t*);
} readt_t;

// io types
typedef struct buffer_t
{
  obj_t obj;
  ARRAY_SLOTS(char);
} buffer_t;

typedef enum port_fl_t
  {
    port_fl_inport  =0x040,
    port_fl_outport =0x080,
    port_fl_lispport=0x100,
    port_fl_ready   =0x200,
  } port_fl_t;

struct port_t
{
  obj_t      obj;

  FILE      *stream;
  buffer_t  *buffer;

  val_t      value;
  val_t      temp;
};

// vm-internal types
struct heap_t
{
  obj_t    obj;
  obj_t   *objects;

  stack_t *grays;
  stack_t *saved;

  size_t   allocated;
  size_t   alloccap;
};

// globals
// vm core objects
heap_t  Heap;
symt_t  Symbols;
readt_t Reader;
envt_t  Toplevel;

// standard streams
port_t  Ins, Outs, Errs;

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
    CONS   =0x14,
    ATOM   =0x15,
    SYMT   =0x16,
    RENTRY =0x17,
    READT  =0x18,
    PORT   =0x19,
    BUFFER =0x1a,
    ALIST  =0x1b,
    STACK  =0x1c,
    
    ANY    =0x20,
    NONE   =0x21,
    N_TYPES
  };

// bit manipulation & tag utilities
#define as_real(val)     (((val_data_t)(val)).as_real)
#define as_chr(val)      ((int32_t)((((val_data_t)(val)).as_val)&(val_t)UINT32_MAX))
#define as_val(val)      (((val_data_t)(val)).as_val)
#define as_ptr(val)      ((void*)(as_val(val)&PMASK))
#define tag_ptr(val,tag) ((((val_t)(val))&PMASK)|(tag))
#define as_obj(val)      ((obj_t*)as_ptr(val))
#define as_atom(val)     ((atom_t*)as_ptr(val))
#define as_cons(val)     ((cons_t*)as_ptr(val))

type_t typeof_obj(obj_t *obj)
{
  return obj->type;
}

type_t typeof_val(val_t val)
{
  switch ((val&TMASK))
    {
    case OBJ:    return as_obj(val)->type;
    case CHRTAG: return CHRTYPE;
    case NULTAG: return NULTYPE;
    default:     return REAL;
    }
}

#define rtypeof(val)				\
  _Generic((val),				\
	   val_t:typeof_val,			\
	   obj_t*:typeof_obj)((val))

#define isa(val, type)				\
  (rtypeof(val)==(type))

#define is_real(val)     ((val)&QNAN != QNAN)
#define is_obj(val)      (((val)&TMASK)==OBJ)
#define is_chr(val)      (((val)&TMASK)==CHRTAG)
#define is_nul(val)      ((val)==NUL)
#define is_atom(val)     isa(val, ATOM)
#define is_cons(val)     isa(val, CONS)
#define is_port(val)     isa(val, PORT)

#define cons_car(val)    (as_cons(val)->car)
#define cons_cdr(val)    (as_cons(val)->cdr)

// memory management
// forward declarations
void  *alloc(size_t n);
void   dealloc(void *p, size_t n);
void   collect_garbage(void);
void   mark_obj(obj_t *obj);
void   mark_val(val_t val);
void   free_obj(obj_t *obj);

#define safe_alloc(allcfun, ...)		\
  ({						\
    void *out = allcfun( __VA_ARGS__ );		\
    if (out == NULL)				\
      {						\
	printf("error at %s:%d:%s: OOM.",	\
	       __FILE__,			\
	       __LINE__,			\
	       __func__);			\
	exit(1);				\
      }						\
    out;					\
  })

#define malloc_s(n)        safe_alloc(malloc, (n))
#define calloc_s(n, s)     safe_alloc(calloc, (n), (s))
#define realloc_s(p, n)    safe_alloc(realloc, (p), (n))

#define resize_vec(p, o, n, t) resize((p), (o)*sizeof(t), (n)*sizeof(t))
#define alloc_vec(n, t)   alloc((n)*sizeof(t))
#define dealloc_vec(p, n, t) dealloc((p), (n)*sizeof(t))

#define duplicate(p, ...)						\
  _Generic((p),								\
	   char*:duplicate_string,					\
	   default:duplicate_bytes)((p) __VA_OPT__(,) __VA_ARGS__)

// forward declarations for object apis
void      prin_real(port_t *port, val_t val);

void      prin_chr(port_t *port, val_t val);

atom_t   *new_atom(void);
void      init_atom(atom_t *atom, symt_t *symt, char *name);
void      free_atom(obj_t *obj);
void      prin_atom(port_t *port, val_t val);

cons_t   *new_cons(void);
cons_t   *new_conses(arity32_t n);
void      init_conses(cons_t *cons, val_t *args, arity32_t n);
void      init_cons(cons_t *cons, val_t car, val_t cdr);
void      trace_cons(obj_t *obj);
void      prin_list(port_t *port, val_t val);

port_t   *new_port(void);
void      init_port(port_t *port, FILE *stream, flags16_t flags);
void      trace_port(obj_t *obj);
void      free_port(obj_t *obj);
void      reset_port(port_t *port);
void      port_close(port_t *port);

// IO predicates
bool      port_eosp(port_t *port);
bool      port_insp(port_t *port);
bool      port_outsp(port_t *port);
bool      port_readyp(port_t *port);

// low level IO
int32_t   port_prinf(port_t *port, char *fmt, ...);
int32_t   port_princ(port_t *port, int32_t ch);
int32_t   port_readc(port_t *port);
int32_t   port_ungetc(port_t *port, int32_t ch);
int32_t   port_peekc(port_t *port);

// lisp IO
val_t     port_prin(port_t *port, val_t val);
val_t     port_read(port_t *port);

alist_t  *new_alist(void);
void      init_alist(alist_t *alist);
void      trace_alist(obj_t *obj);
void      free_alist(obj_t *obj);
void      resize_alist(alist_t *alist, size_t newl);
arity32_t alist_push(alist_t *alist, val_t val);
arity32_t alist_write(alist_t *alist, val_t *src, arity32_t n);
bool      alist_pop(alist_t *alist, val_t *buf);

stack_t  *new_stack(void);
void      trace_stack(obj_t *obj);
void      init_stack(stack_t *stack);
void      free_stack(obj_t *obj);
void      resize_stack(stack_t *stack, size_t newl);
arity32_t stack_push(stack_t *stack, val_t val);
arity32_t stack_write(stack_t *stack, val_t *src, arity32_t n);
bool      stack_pop(stack_t *stack, val_t *buf);

buffer_t *new_buffer(void);
void      init_buffer(buffer_t *buffer);
void      free_buffer(obj_t *obj);
void      clear_buffer(buffer_t *buffer);
void      reset_buffer(buffer_t *buffer);
void      resize_buffer(buffer_t *buffer, size_t newl);
arity32_t buffer_push(buffer_t *buffer, char val);
arity32_t buffer_write(buffer_t *buffer, char *src, arity32_t n);
bool      buffer_pop(buffer_t *buffer, char *buf);

void      trace_symt(obj_t *obj);
void      init_symt(symt_t *symt);
void      free_symt(obj_t *obj);
void      clear_symt(symt_t *symt);
void      resize_symt(symt_t *symt, size_t newl);
void      rehash_symt(atom_t **old,size_t oldc,atom_t **new,size_t newc);
bool      symt_put(symt_t *symt, char *name, atom_t **buf);

void      trace_readt(obj_t *obj);
void      init_readt(readt_t *readt);
void      free_readt(obj_t *obj);
void      clear_readt(readt_t *readt);
void      resize_readt(readt_t *readt, size_t newl);
void      rehash_readt(rentry_t**old,size_t oldc,rentry_t**new,size_t newc);
bool      readt_put(readt_t *readt,char32_t dispatch, rentry_t **buf);
bool      readt_get(readt_t *readt,char32_t dispatch, rentry_t **buf);

void      init_heap(heap_t *heap);
void      free_heap(obj_t *obj);

// dispatch tables
const char  *TypeNames[N_TYPES] =
  {
    [REAL]    = "real",        [CHRTYPE] = "chr",   [NULTYPE] = "nul",
    [ATOM]    = "atom",        [SYMT]    = "symt",  [READT]   = "readt",
    [RENTRY]  = "readt-entry", [PORT]    = "port",  [CONS]    = "cons",
    [ALIST]   = "alist",       [STACK]   = "stack",
  };

printer_fn_t Print[N_TYPES] =
  {
   [REAL] = prin_real, [ATOM]   = prin_atom, [CHRTYPE] = prin_chr,
   [CONS] = prin_list, [NULTYPE]= prin_list,
  };

const size_t BaseSizes[N_TYPES] =
  {
   [REAL]  =sizeof(real_t),  [CHRTYPE]=sizeof(char_t),

   [CONS]  =sizeof(cons_t),
   [ATOM]  =sizeof(atom_t),  [SYMT]   =sizeof(symt_t),
   [READT] =sizeof(readt_t), [RENTRY] =sizeof(rentry_t), [PORT]  =sizeof(port_t),
   [ALIST] =sizeof(alist_t), [STACK]  =sizeof(stack_t),  [BUFFER]=sizeof(buffer_t),
  };

const size_t Mincs[N_TYPES] =
  {
    [ALIST]  = 8,   [SYMT]  = 256, [READT] = 256,
    [BUFFER] = 128, [STACK] = 64,
  };

trace_fn_t Trace[N_TYPES] =
  {
    [READT] = trace_readt, [SYMT]  = trace_symt,
    [STACK] = trace_stack, [ALIST] = trace_alist,
    [PORT]  = trace_port,  [CONS]  = trace_cons,
  };

free_fn_t Free[N_TYPES] =
  {
   [ATOM]  =free_atom,   [PORT]  =free_port,
   [SYMT]  =free_symt,   [READT] =free_readt,
   [STACK] =free_stack,  [ALIST] =free_alist,
   [BUFFER]=free_buffer,
  };

// memory implementation
static const size_t heap_inic     = 8192*sizeof(alist_t);
static const real_t heap_pressure = 0.75;

void init_heap(heap_t *heap)
{
  // initialize heap by hand, since it bypasses the usual protocol
  heap->obj.next  = NULL;
  heap->obj.type  = HEAP;
  heap->obj.gray  = false;
  heap->obj.black = true;
  heap->obj.flags = 0;
  
  heap->allocated = 0;
  heap->alloccap  = heap_inic;

  heap->objects   = NULL;
  stack_t *gray   = malloc_s( sizeof(alist_t) );
  heap->grays     = gray;

  // do the same for the initial part of heap->grays
  gray->obj.next  = NULL;
  gray->obj.type  = ALIST;
  gray->obj.gray  = false;
  gray->obj.black = true;
  gray->obj.flags = Ctype_pointer;

  gray->len       = 0;
  gray->cap       = Mincs[STACK];
  gray->data      = alloc_vec(  gray->cap, val_t );

  // saved can be initialized the easy way
  heap->saved     = new_stack();
  init_stack(heap->saved);
}

void free_heap(obj_t *obj)
{
  heap_t *heap = (heap_t*)obj;

  for (obj_t *obj=heap->objects;obj != NULL;)
    {
      obj_t *tmp = obj;
      obj        = tmp->next;
      free_obj( obj );
    }
  
  free_stack((obj_t*)heap->grays);
  free( heap->grays );
}

void mark_roots( void )
{
  mark_obj((obj_t*)Heap.saved);
  mark_obj((obj_t*)&Symbols);
  mark_obj((obj_t*)&Reader);
  mark_obj((obj_t*)&Ins);
  mark_obj((obj_t*)&Outs);
  mark_obj((obj_t*)&Errs);
}

void collect_garbage(void)
{
  mark_roots();

  val_t buf;
  while (stack_pop(Heap.grays, &buf))
    {
      obj_t *obj       = as_obj(buf);
      type_t type      = obj->type;
      trace_fn_t trace = Trace[type];
      trace(obj);
      obj->gray        = false;
    }

  obj_t **prev = &Heap.objects, *curr = Heap.objects;
  
  while (curr != NULL)
    {
      if (curr->black)
	{
	  curr->black = false;
	  curr->gray  = true;
	  prev        = &curr->next;
	  curr        = curr->next;
	}

      else
	{
	  obj_t *obj  = curr;
	  *prev       = curr->next;
	  curr        = curr->next;

	  free_obj(obj);
	}
    }

  if (Heap.allocated * heap_pressure > Heap.alloccap)
    Heap.alloccap *= 2;
}

void free_obj(obj_t *obj)
{
  if (obj == NULL)
    return;

  type_t type = obj->type;

  if (Free[type])
    Free[type](obj);

  dealloc( obj, BaseSizes[type] );
}

void mark_obj(obj_t *obj)
{
  if (obj == NULL)
    return;

  if (obj->black)
    return;

  type_t t = rtypeof(obj);

  if (Trace[t])
    stack_push( Heap.grays, tag_ptr(obj, OBJ) );

  else
    obj->gray = false;
}
void mark_val(val_t val)
{
  if (is_obj(val))
    mark_obj(as_obj(val));
}

void trace_vals(val_t *vals, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_val(vals[i]);
}

void trace_objs(obj_t **objs, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_obj(objs[i]);
}

void *alloc(size_t n)
{
  if (Heap.allocated + n > Heap.alloccap)
    collect_garbage();
  
  void *out = malloc_s(n);
  memset(out, 0, n);
  return out;
}

void *duplicate_bytes(void *p, size_t n)
{
  void *dup = alloc(n);
  memcpy(dup, p, n);
  return dup;
}

char *duplicate_string(char *chars)
{
  return duplicate_bytes(chars, strlen(chars));
}

void* resize(void *p, size_t o, size_t n)
{
  if (o > n)
    Heap.allocated -= n;

  else
    {
      size_t diff = n - o;

      if (Heap.allocated + diff > Heap.alloccap)
	collect_garbage();

      Heap.allocated += diff;
    }

  return realloc_s(p, n);
}

void dealloc(void *p, size_t n)
{
  Heap.allocated -= n;
  free(p);
}

// object apis & implementations
// primitives
// real
void prin_real(port_t *port, val_t val)
{
  port_prinf( port, "%.3F", as_real(val));
}

// character
#include "chrnames.h"


void prin_chr(port_t *port, val_t val)
{
  if (val == EOS)
    port_prinf( port, "<eos>");

  else
    port_prinf( port, "\\%s", ChrNames[as_chr(val)&128]);
}

// object
void init_obj( obj_t *obj, type_t type, flags16_t flags )
{
  obj->next    = Heap.objects;
  Heap.objects = obj;

  obj->type    = type;
  obj->gray    = true;
  obj->black   = false;
  obj->flags   = flags;
}

// pair types
bool is_proper(val_t val)
{
  return is_nul(val)
    || (is_cons(val)
	&& flag_p(as_obj(val)->flags, cons_fl_proper));
}

bool is_list(val_t val)
{
  return is_nul(val) || is_cons(val);
}

OBJ_NEW(cons);

cons_t *new_conses(arity32_t n)
{
  return alloc_vec(n, cons_t );
}

void init_cons( cons_t *cons, val_t car, val_t cdr )
{
  init_obj(&cons->obj, CONS, cons_fl_proper * is_proper(cdr) );
  cons->car    = car;
  cons->cdr    = cdr;
  cons->eltype = ANY;
  cons->len    = 1;

  if (is_cons(cdr))
    cons->len += as_cons(cdr)->len;
}

void init_conses( cons_t *cons, val_t *args, arity32_t n )
{
  for (arity32_t i=n-1; i>0; i--)
    {
      val_t tail = args[i];
      val_t head = args[i-1];
      init_cons(cons+i-1, head, tail);
      args[i-1] = tail;
    }
}


void prin_list(port_t *port, val_t val)
{
  port_prinf(port, "(" );

  while (is_cons(val))
    {
      port_prin(port, cons_car(val));
      val = cons_cdr(val);

      if (is_cons(val))
	port_prinf(port, " ");

      if (!is_nul(val))
	port_prinf(port, ". ");
    }

  port_prinf(port, ")");
}

// boxed array types
OBJ_NEW(alist);
ARRAY_INIT(alist, val_t, Ctype_uint64, ALIST);
ARRAY_TRACE(alist, val_t);
ARRAY_FREE(alist, val_t);
ARRAY_RESIZE(alist, val_t, ALIST, pad_alist_size);
ARRAY_WRITE(alist, val_t);
ARRAY_PUSH(alist, val_t);
ARRAY_POP(alist, val_t);
ARRAY_CLEAR(alist, val_t, ALIST);

OBJ_NEW(stack);
ARRAY_INIT(stack, val_t, Ctype_uint64, STACK );
ARRAY_TRACE(stack, val_t);
ARRAY_FREE(stack, val_t);
ARRAY_RESIZE(stack, val_t, STACK, pad_stack_size);
ARRAY_WRITE(stack, val_t);
ARRAY_PUSH(stack, val_t);
ARRAY_POP(stack, val_t);
ARRAY_CLEAR(stack, val_t, STACK);

// symbol & symbol table
OBJ_NEW(atom);

void free_atom(obj_t *obj)
{
  atom_t *atom = (atom_t*)obj;
  dealloc_vec(atom->name, strlen(atom->name), char );
}

void init_atom(atom_t *atom, symt_t *symt, char *name )
{
  init_obj(&atom->obj, ATOM, 0);

  atom->name = duplicate(name);
  atom->idno = symt->idno++;
}

void prin_atom(port_t *port, val_t val)
{
  port_prinf( port, as_atom(val)->name );
}

TABLE_INIT(symt, atom, SYMT);
TABLE_FREE(symt, atom);
TABLE_TRACE(symt, atom);
TABLE_CLEAR(symt, atom);
TABLE_REHASH(symt, atom);
TABLE_RESIZE(symt, atom, SYMT);
TABLE_PUT(symt, atom, char*, name, hash_string, strcmp);

atom_t *symt_intern(symt_t *symt, char *name)
{
  atom_t *buf;

  if (symt_put(symt, name, &buf))
      init_atom(buf, symt, name);

  return buf;
}

// read table and readtable entries
OBJ_NEW(rentry);

void init_rentry(rentry_t *re, char32_t dispatch, reader_fn_t handler)
{
  re->dispatch = dispatch;
  re->handler  = handler;
}

TABLE_INIT(readt, rentry, READT);
TABLE_FREE(readt, rentry);
TABLE_TRACE(readt, rentry);
TABLE_CLEAR(readt, rentry);
TABLE_REHASH(readt, rentry);
TABLE_RESIZE(readt, rentry, READT);
TABLE_PUT(readt, rentry, char32_t, dispatch, hash_long, cmp);
TABLE_GET(readt, rentry, char32_t, dispatch, hash_long, cmp);

void add_reader_macro(readt_t *readt, char32_t dispatch, reader_fn_t handler)
{

  rentry_t *buf;

  if (readt_put(readt, dispatch, &buf))
	init_rentry(buf, dispatch, handler);

  else
    {
      printf( "You absolute dullard, you used the same dispatch character twice!\n" );
      exit(1);
    }
}

// io types
// buffer
OBJ_NEW(buffer);
ARRAY_INIT(buffer, char, Ctype_ascii, BUFFER);
ARRAY_FREE(buffer, char);
ARRAY_RESIZE(buffer, char, BUFFER, pad_stack_size);
ARRAY_WRITE(buffer, char);
ARRAY_PUSH(buffer, char);
ARRAY_POP(buffer, char);
ARRAY_CLEAR(buffer, char, BUFFER);

// port type
OBJ_NEW(port);

void init_port(port_t *port, FILE *stream, flags16_t flags)
{
  init_obj( &port->obj, PORT, flags );
  port->stream = stream;
  port->value = port->temp = NUL;
  port->buffer = new_buffer();
  init_buffer( port->buffer );
}

void trace_port(obj_t *obj)
{
  port_t *port = (port_t*)obj;

  mark_obj((obj_t*)port->buffer);
  mark_val(port->value);
  mark_val(port->temp);
}

void free_port(obj_t *obj)
{
  port_t *port = (port_t*)obj;

  port_close(port);
  free_obj((obj_t*)port->buffer);
}

void port_close(port_t *port)
{
  FILE *stream = port->stream;

  if (stream == stdin || stream == stdout || stream == stderr)
    return;

  fclose(stream);
}

// IO predicates
bool port_eosp(port_t *port)
{
  
  return feof(port->stream);
}

bool port_insp(port_t *port)
{
  return flag_p(port->obj.flags, port_fl_inport);
}

bool port_outsp(port_t *port)
{
  return flag_p(port->obj.flags, port_fl_outport);
}

bool port_readyp(port_t *port)
{
  return flag_p(port->obj.flags, port_fl_ready);
}

// low level IO
int32_t port_prinf(port_t *port, char *fmt, ...)
{
  assert(port_outsp(port));
  va_list va;
  va_start(va, fmt);
  arity32_t out = vfprintf(port->stream, fmt, va);
  va_end(va);
  return out;
}

int32_t port_princ(port_t *port, int32_t ch)
{
  return fputc( ch, port->stream);
}

int32_t port_readc(port_t *port)
{
  if (port_eosp(port))
    return EOF;

  return fgetc(port->stream);
}

int32_t port_peekc(port_t *port)
{
  if (port_eosp(port))
    return EOF;

  int32_t out = fgetc(port->stream);
  ungetc(out, port->stream);
  return out;
}

int32_t port_ungetc(port_t *port, int32_t ch)
{
  if (port_eosp(port))
    return EOF;

  return ungetc(ch, port->stream);
}

// lisp IO
val_t port_prin(port_t *port, val_t val)
{
  assert(port_outsp(port));
  
  type_t type = rtypeof(val);

  if (Print[type])
    Print[type](port, val);

  else
    port_prinf(port, "<%s>", TypeNames[type] );

  return val;
}

val_t  port_take(port_t *port)
{
  if (port_readyp(port) || port_eosp(port))
    return port->value;
  
  val_t out      = port->value;
  port->value    = NUL;

  port->obj.flags |= port_fl_ready;
  
  clear_buffer(port->buffer);
  return out;
}

void port_give(port_t *port, val_t val)
{
  port->obj.flags &= ~port_fl_ready;
  port->value      = val;
}

static reader_fn_t get_reader(int32_t dispatch)
{
  rentry_t *buf;

  if (!readt_get(&Reader, dispatch, &buf))
    {
	  printf( "No way to read %c, exiting.\n", dispatch );
	  exit( 1 );
    }

  return buf->handler;
}

val_t port_read(port_t *port)
{
  while (port_readyp(port))
    {
      int32_t ch  = port_peekc(port);
      reader_fn_t reader = get_reader(ch);
      reader(port, ch);
    }

  return port->value;
}

// reader
#define VERSION "%d.%d.%d.%c"
#define MAJOR   0
#define MINOR   1
#define PATCH   1
#define DEV     'a'

#define INPROMPT  "<< "
#define OUTPROMPT ">> "
static bool symchrp(int32_t chr)
{
  if (chr == EOF)
    return false;
  
  return iswalnum(chr) || strchr(":?!+-_*/^", chr);
}

static int testrealchr(int32_t chr, char *acc)
{
  if (iswdigit(chr))
    return 0;
  
  if (iswspace(chr))
    return strpbrk(acc, "0123456789") ? 1 : -1;

  if (chr == '.')
    return strchr(acc, '.') ? -1 : 0;

  return -1;
}

static void accumc( port_t *port, char32_t ch )
{
  buffer_push( port->buffer, ch );
}

void read_error(port_t *port, char32_t dispatch)
{
  (void)dispatch;

  atom_t *atom = symt_intern( &Symbols, "error" );
  port_give(port, tag_ptr(atom, OBJ));
}

void read_atom(port_t *port, char32_t dispatch)
{
  dispatch = port_readc(port); // advance

  // printf( "made it into read_atom.\n" );
  while (symchrp(dispatch))
    {
      accumc( port, dispatch );
      dispatch = port_readc(port);
    }

  // printf( "made it through read.\n" );
  atom_t *interned = symt_intern(&Symbols, port->buffer->data);
  port_give(port, tag_ptr(interned, OBJ));
}

void read_list(port_t *port, char32_t dispatch)
{
  port->temp = NUL;
  port_readc(port);     // advance past opening '('.

  arity32_t base = Heap.saved->len;

  while ((dispatch=port_peekc(port)) != ')' && dispatch != '.')
    {
      port_read(port);
      stack_push( Heap.saved, port_take(port) );
    }

  if (dispatch == '.')
    {
      port_readc(port); // advance past '.'
      port_read(port);
      stack_push(Heap.saved, port_take(port) );
    }

  else
    stack_push(Heap.saved, NUL);

  if (dispatch != ')')
    {
      read_error(port, dispatch);
      return;
    }

  // allocate and initialize
  cons_t *space = new_conses( Heap.saved->len-base-1 );
  init_conses(space, Heap.saved->data+base, Heap.saved->len-base );

  // set token value
  port_give(port, tag_ptr(space, OBJ));

  // cleanup
  resize_stack( Heap.saved, base );
  port_readc(port); // advance past terminal ')'
}

void read_real(port_t *port, char32_t dispatch)
{
  accumc(port, dispatch);

  port_readc(port);            // clear the dispatch character

  int test = 0;

  
  while (!(test=testrealchr((dispatch=port_peekc(port)), port->buffer->data)))
    accumc( port, dispatch );

  if (test < 0)
      read_atom(port, dispatch);

  else
    {
      char *buf;
      double out = strtod( port->buffer->data, &buf );

      assert(!*buf);

      port_give(port, as_val(out) );
    }
}

void read_comment(port_t *port, char32_t dispatch)
{
  while ((dispatch=port_readc(port)) != '\n')
    continue;
}

void read_space(port_t *port, char32_t dispatch)
{
  while (iswspace(dispatch))
    dispatch = port_readc(port);

  port_ungetc(port, dispatch);
}

void read_eof(port_t *port, char32_t dispatch)
{
  (void)dispatch;

  port_give(port, EOS);
}

// main
static void init_reader( void )
{
  add_reader_macro( &Reader, EOF, read_eof );    
  
  for (char dispatch='\0'; dispatch < '\x7f'; dispatch++)
    {
      switch (dispatch)
	{
	case '\0' ... '\b':
	case '\x0e' ... '\x1f':
	case '\x7f':
	  break;

	case '\x09' ... '\x0d':
	case ' ':
	  add_reader_macro( &Reader, dispatch, read_space );
	  break;

	case '+': case '-': case '.': case '0' ... '9':
	  add_reader_macro( &Reader, dispatch, read_real );
	  break;

	case ';':
	  add_reader_macro( &Reader, dispatch, read_comment );
	  break;

	case '(':
	  add_reader_macro( &Reader, dispatch, read_list );
	  break;

	case ')':
	  add_reader_macro( &Reader, dispatch, read_error );
	  break;

	default:
	  add_reader_macro( &Reader, dispatch, read_atom );
	  break;
	}
    }
}

static void init_rascal( void )
{
  init_heap(&Heap);
  init_symt(&Symbols);
  init_readt(&Reader);

  init_port(&Ins, stdin, port_fl_inport|port_fl_lispport|Ctype_ascii);
  init_port(&Outs, stdout, port_fl_outport|port_fl_lispport|Ctype_ascii);
  init_port(&Errs, stderr, port_fl_outport|port_fl_lispport|Ctype_ascii);

  init_reader();
}

static void prin_welcome( void )
{
  printf("Welcome to rascal version "VERSION"!\n\n", MAJOR, MINOR, PATCH, DEV );
}

static void repl( void )
{
  for (;;)
    {
      port_take(&Ins);
      printf(INPROMPT);
      val_t val = port_read(&Ins);
      printf("\n"OUTPROMPT);
      port_prin(&Outs, val);
      printf("\n");
    }
}

int main(const int argc, const char *argv[argc])
{
  (void)argv;

  init_rascal();
  prin_welcome();
  repl();
  
  return 0;
}
