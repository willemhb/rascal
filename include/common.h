#ifndef rascal_common_h
#define rascal_common_h

#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>
#include <stdio.h>

/* making common C types style compliant */
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

/* utility typedefs */
typedef uint64_t hash_t;
typedef uint64_t idno_t;
typedef int      order_t;
typedef uint     flags_t;
typedef uint     arity_t;
typedef uint     index_t;

/* builtin types */
typedef enum {
  type_port=0,
  type_boolean,
  type_character,
  type_integer,
  type_flonum,
  type_nil,

  type_cons,
  type_symbol,

  type_builtin=8,
  type_closure,

  type_string,
  type_vector,
  type_table
} type_t;

#define TYPE_PAD 32

/* a tagged lisp value */
typedef uintptr_t value_t;

// tags -----------------------------------------------------------------------
#define QNAN        0x7ffc000000000000ul
#define SIGN        0x8000000000000000ul

#define IMMEDIATE   0x7ffc000000000000ul
#define PORT        0x7ffd000000000000ul
#define FUNCTION    0x7ffe000000000000ul
#define STRING      0x7fff000000000000ul
#define VECTOR      0xfffc000000000000ul
#define TABLE       0xfffd000000000000ul
#define SYMBOL      0xfffe000000000000ul
#define LIST        0xffff000000000000ul

#define BOOLEAN     (IMMEDIATE|((value_t)type_boolean<<40))
#define INTEGER     (IMMEDIATE|((value_t)type_integer<<40))
#define CHARACTER   (IMMEDIATE|((value_t)type_character<<40))

// special constants ----------------------------------------------------------
#define UNBOUND     (SYMBOL|0)
#define NIL         (LIST|0)
#define ZERO        (INTEGER|0)
#define ONE         (INTEGER|1)
#define TRUE        (BOOL|1)
#define FALSE       (BOOL|0)

// immediate typedefs ---------------------------------------------------------
typedef double flonum_t;
typedef long   integer_t;      
typedef bool   boolean_t;
typedef char   character_t;
typedef FILE   port_t;

typedef struct symbol_t   symbol_t;
typedef struct cons_t     cons_t;
typedef struct function_t function_t;
typedef struct vector_t   vector_t;
typedef struct table_t    table_t;
typedef struct string_t   string_t;

// internal typedefs ----------------------------------------------------------
typedef struct stack_t stack_t;
typedef struct heap_t  heap_t;
typedef struct gc_frame_t gc_frame_t;

typedef union {
  double    fval;
  uintptr_t uval;

  struct {
    long pad  : 24;
    long lval : 40;
  };
} C_data_t;

// globals --------------------------------------------------------------------
// core VM objects ------------------------------------------------------------
extern stack_t *Stack;
extern heap_t *Heap;
extern table_t *Symbols;

extern gc_frame_t *Saved;
extern jmp_buf Toplevel;

// VM registers ---------------------------------------------------------------
extern uint Arg, Ctl;
extern value_t Fun, Val;

// misc -----------------------------------------------------------------------
extern value_t Open;

// utilities ------------------------------------------------------------------
#define rsp_aligned  __attribute__((aligned(sizeof(pair_t))))
#define rsp_unlikely(c) __builtin_expect((c), 0)

#define HITAG      0xffff000000000000ul
#define WIDETAG    0xffff00ff00000000ul

#define IMASK      0x00000000fffffffful
#define PMASK      0x0000fffffffffffful

#define hitag(x)    ((x)&HITAG)
#define widetag(x)  ((x)&WIDETAG)
#define ibits(x)    ((x)&IMASK)
#define pbits(x)    ((x)&PMASK)

#define tagptr(x,t) ((((value_t)(x))&PMASK)|(t))
#define tagint(x,t) ((((value_t)(x))&IMASK)|(t))

#define as_type(ctype, cnvt, x) ((ctype)cnvt(x))

#define fval(x)    (((C_data_t)(x)).fval)
#define uval(x)    ((x)&PMASK)
#define ival(x)    ((sint_t)((x)&IMASK))
#define lval(x)    (((C_data_t)(x)).lval)
#define pval(x)    ((void*)uval(x))

#define tag_hash(type, T)			\
  hash_t type##_hash(value_t x)			\
  {						\
    (void)x;					\
    return int64hash(T);			\
  }

#define for_t_cons(c, x)						\
  for (;is_cons(*c) && ((x=car(*c))||1) && is_cons(cdr(*c));*c=cdr(*c))

#define for_cons(c, x)					\
  for (;is_cons(*c) && ((x=car(*c))||1); *c = cdr(*c))

#endif
