#ifndef rascal_common_h
#define rascal_common_h

#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>
#include <stdio.h>

/* making common C types style compliant */
typedef signed char    schar;
typedef unsigned char  uchar;
typedef char           char_t;
typedef signed char    schar_t;
typedef unsigned char  uchar_t;

typedef signed short   sshort;
typedef unsigned short ushort;
typedef short          short_t;
typedef signed short   sshort_t;
typedef unsigned short ushort_t;

typedef signed int     sint;
typedef unsigned int   uint;
typedef int            int_t;
typedef signed int     sint_t;
typedef unsigned int   uint_t;

typedef signed long    slong;
typedef unsigned long  ulong;
typedef long           long_t;
typedef signed long    slong_t;
typedef unsigned long  ulong_t;

/* utility typedefs */
typedef uint64_t       hash_t;
typedef uint64_t       idno_t;
typedef sint_t         order_t;
typedef uint_t         flags_t;
typedef uint_t         arity_t;
typedef uint_t         index_t;

/* enum types */
typedef enum
  {
    C_pointer, C_uint64, C_sint64, C_double,

    C_utf32, C_uint32, C_sint32, C_float,
    
    C_utf16, C_uint16, C_sint16,

    C_utf8,  C_uint8, C_sint8, C_ascii, C_latin1
} Ctype_t;

typedef enum {
  type_port=0,
  type_nil,
  type_boolean,
  type_flonum,
  type_integer,

  type_binary,
  type_tuple,
  type_map,

  type_builtin=8,
  type_closure,

  type_cons,
  type_symbol
} type_t;

#define TYPE_PAD 32

typedef enum
  {
    // constructors -----------------------------------------------------------
    F_NIL=type_nil, F_BOOL, F_FLO, F_BIN, F_TUP, F_MAP, F_CLO, F_CONS=type_cons, F_SYM,

    // type predicates --------------------------------------------------------
    F_NIL_P, F_BOOL_P, F_FLO_P, F_BIN_P, F_TUP_P, F_MAP_P=17, F_CLO_P, F_CONS_P, F_SYM_P,

    // type-ish predicates ----------------------------------------------------
    F_KW_P, F_GEN_P, F_FUN_P, F_TYPE_P=25,

    // other booleans ---------------------------------------------------------
    F_ID_P, F_NOT,

    // other utilities --------------------------------------------------------
    F_ORD, F_HASH, F_SIZE,

    // arithmetic -------------------------------------------------------------
    F_INC, F_DEC=33, F_ADD, F_SUB, F_MUL,  F_DIV, F_MOD,

    // arithmetic predicates --------------------------------------------------
    F_EQP, F_LTP=41,
    
    // accessors --------------------------------------------------------------
    F_CAR, F_CDR, F_NTH, F_REF,

    // sequence generics ------------------------------------------------------
    F_LEN,

    // io ---------------------------------------------------------------------
    F_OPEN, F_CLOSE=49, F_EOFP, F_READ, F_PRIN, F_LOAD,

    // interpreter ------------------------------------------------------------
    F_APPLY, F_BOUND_P, F_LOOKUP=57, F_COMPILE, F_EXEC,

    // runtime ----------------------------------------------------------------
    F_ERROR, F_SYS, F_GETENV, F_EXIT,

    // sentinel
    NUM_BUILTINS
} builtin_t;

/* a tagged lisp value */
typedef uintptr_t value_t;

// tags -----------------------------------------------------------------------
#define QNAN        0x7ffc000000000000ul
#define SIGN        0x8000000000000000ul

#define IMMEDIATE   0x7ffc000000000000ul
#define INTEGER     0x7ffd000000000000ul
#define FUNCTION    0x7ffe000000000000ul
#define STRING      0x7fff000000000000ul
#define TUPLE       0xfffc000000000000ul
#define MAP         0xfffd000000000000ul
#define SYMBOL      0xfffe000000000000ul
#define LIST        0xffff000000000000ul

#define LOHEADER    0x7ffd

#define PORT        (IMMEDIATE|((value_t)type_port<<32))
#define BOOL        (IMMEDIATE|((value_t)type_boolean<<32))
#define UPVALUE     (IMMEDIATE|((value_t)type_integer<<32))

// special constants ----------------------------------------------------------
#define UNBOUND     (SYMBOL|0)
#define NIL         (LIST|0)
#define MOVED       (FUNCTION|0)
#define EMPTYSTR    (BINARY|0)
#define EMPTYTUP    (TUPLE|0)
#define EMPTYMAP    (MAP|0)
#define ZERO        (INTEGER|0)
#define ONE         (INTEGER|1)

#define INS         (PORT|0)
#define OUTS        (PORT|1)
#define ERRS        (PORT|2)
#define TRUE        (BOOL|1)
#define FALSE       (BOOL|0)

// immediate typedefs ---------------------------------------------------------
typedef double           flonum_t;
typedef slong            integer_t;      
typedef bool             boolean_t;
typedef sint_t           port_t;

typedef struct symbol_t  symbol_t;
typedef struct cons_t    cons_t;
typedef struct closure_t closure_t;
typedef struct tuple_t   tuple_t;
typedef struct map_t     map_t;
typedef struct binary_t  binary_t;
typedef struct binary_t  string_t;
typedef struct binary_t  bytecode_t;

// internal typedefs ----------------------------------------------------------
typedef struct stack_t        stack_t;
typedef struct heap_t         heap_t;
typedef struct table_t        table_t;
typedef struct ios_t          ios_t;
typedef struct ios_map_t      ios_map_t;
typedef struct gc_frame_t     gc_frame_t;

typedef union {
  double    fval;
  uintptr_t uval;

  struct {
    slong_t _pad : 16;
    slong_t lval : 48;
  };

  struct {
    sint_t ival;
    uint_t meta;
  };
} C_data_t;

// globals --------------------------------------------------------------------
#define Sp      (Eval->sp)
#define Fp      (Cntl->sp)
#define Frame   (&(Cntl->data[Fp-3]))
#define Stack   (Eval->data)
#define Tos     (Eval->data[Eval->sp-1])
#define Peek(n) (Eval->data[Eval->sp-1-(n)])
#define Sref(n) (Eval->data[(n)])

// core VM objects ------------------------------------------------------------
extern stack_t *Eval, *Cntl;
extern heap_t *Heap;
extern table_t *Symbols;
extern ios_map_t *Ports;

extern gc_frame_t *Saved;
extern jmp_buf Toplevel;

// form names -----------------------------------------------------------------
extern value_t sym_quote, sym_do;
extern value_t sym_if, sym_and, sym_or;
extern value_t sym_val, sym_fun, sym_let;

// keywords -------------------------------------------------------------------
extern value_t kw_vargs;                                  // closure options
extern value_t kw_intern;                                 // symbol options
extern value_t kw_text, kw_lisp, kw_bin, kw_ins, kw_outs; // io options

// other syntax markers -------------------------------------------------------
extern value_t stx_amp;

// global dispatch ------------------------------------------------------------
extern char *BuiltinNames[NUM_BUILTINS];

// utilities ------------------------------------------------------------------
#define rsp_aligned  __attribute__((aligned(sizeof(pair_t))))
#define rsp_unlikely(c) __builtin_expect((c), 0)

#define HITAG      0xffff000000000000ul
#define WIDETAG    0xffff00ff00000000ul

#define IMASK      0x00000000fffffffful
#define PMASK      0x0000fffffffffffful

#define hitag(x)    ((x)&HITAG)
#define widetag(x)  ((x)&WIDETAG)
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
