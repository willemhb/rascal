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
typedef bool           bool_t;

/* enum types */
typedef enum {
  C_sint8, C_uint8, C_ascii, C_latin1, C_utf8,

  C_sint16, C_uint16, C_utf16,

  C_sint32, C_uint32, C_utf32, C_boolean, C_float,

  C_sint64, C_uint64, C_double, C_pointer
} Ctype_t;

typedef enum {
  type_type=0,
  type_none,
  type_any,
  type_nil,

  type_boolean,
  type_character,
  type_integer,
  type_flonum,

  type_builtin=8,
  type_closure,

  type_pair,
  type_cons,

  type_vector,
  type_string,
  type_binary,

  type_symbol,
  
  type_port=16,

  type_environment,

  type_pointer=24,
} type_t;

typedef enum
  {
    // constructors
    F_NONE=type_none, F_ANY,  F_NIL,
    F_BOOL, F_CHAR, F_INT, F_FLO,
    F_CLO=type_closure, F_PAIR, F_CONS, F_VEC, F_STR, F_BIN,
    F_SYM,
    
    // accessors
    F_CAR=17, F_CDR, F_LEN, F_NTH, F_REF,
    
    F_PUT, F_XUT,
    
    F_XAR=25, F_XDR, F_XTH, F_XEF,
    
    // type predicates
    F_TYPEP, F_NONEP, F_ANYP, F_NILP=33, F_BOOLP, F_CHRP, F_INTP, F_FLOP,
    F_PRIMP, F_CLOP,  F_PAIRP=41, F_CONSP, F_VECP, F_STRP, F_BINP, F_SYMP,
    F_PORTP, F_PTRP=49,
    
    // type-ish predicates
    F_FUNCP, F_KEYWORDP, F_GENSYMP, F_CELLP, 
    
    // arithmetic
    F_ADD, F_SUB, F_MUL=57,  F_DIV, F_MOD, F_EQP, F_LTP,
    
    // other predicates & utilities
    F_IDP, F_NOT,
    
    // io
    F_OPEN=65, F_CLOSE, F_EOFP, F_READ, F_PRIN, F_LOAD,
    
    // interpreter
    F_EVAL, F_APPLY=73, F_LOOKUP,
    
    // runtime
    F_ERROR, F_EXIT,
    
    // special forms
    F_QUOTE, F_DO, F_IF, F_AND=80, F_OR, F_VAL, F_FUN, F_MAC, F_LET, F_LABL,
    
    NUM_BUILTINS
} builtin_t;

/* a tagged lisp value */
typedef uintptr_t value_t;

// tags -----------------------------------------------------------------------
#define QNAN        0x7ffc000000000000ul
#define SIGN        0x8000000000000000ul

#define IMMEDIATE   0x7ffc000000000000ul
#define HEADER      0x7ffd000000000000ul
#define FUNCTION    0x7ffe000000000000ul
#define OBJECT      0x7fff000000000000ul
#define POINTER     0xfffc000000000000ul
#define PAIR        0xfffd000000000000ul
#define CONS        0xfffe000000000000ul
#define SYMBOL      0xffff000000000000ul

#define LOHEADER    0x7ffd

#define TYPE        (IMMEDIATE|((value_t)type_type<<32))
#define PORT        (IMMEDIATE|((value_t)type_port<<32))
#define BOOLEAN     (IMMEDIATE|((value_t)type_boolean<<32))
#define INTEGER     (IMMEDIATE|((value_t)type_integer<<32))
#define CHARACTER   (IMMEDIATE|((value_t)type_character<<32))
#define CLOSURE      (HEADER|((value_t)type_closure<<32))
#define ENVIRONMENT  (HEADER|((value_t)type_environment<<32))
#define VECTOR       (HEADER|((value_t)type_vector<<32))
#define STRING       (HEADER|((value_t)type_string<<32))
#define BINARY       (HEADER|((value_t)type_binary<<32))

// empty singletons -----------------------------------------------------------
#define EMPTYBIN    (IMMEDIATE|((value_t)type_binary<<32))
#define EMPTYSTR    (IMMEDIATE|((value_t)type_string<<32))
#define EMPTYVEC    (IMMEDIATE|((value_t)type_vector<<32))
#define NIL         (IMMEDIATE|((value_t)type_nil<<32))

// special constants ----------------------------------------------------------
#define NONE        (IMMEDIATE|((value_t)type_none<<32))
#define TOPLEVEL    (IMMEDIATE|((value_t)type_environment<<32))
#define UNBOUND     (IMMEDIATE|((value_t)type_symbol<<32))
#define INS         (PORT|0)
#define OUTS        (PORT|1)
#define ERRS        (PORT|2)
#define TRUE        (BOOLEAN|1)
#define FALSE       (BOOLEAN|0)

// immediate typedefs ---------------------------------------------------------
typedef sint_t                 integer_t;
typedef bool                   boolean_t;
typedef sint_t                 port_t;

typedef struct symbol_t        symbol_t;
typedef struct cons_t          cons_t;
typedef struct closure_t       closure_t;
typedef struct vector_t        vector_t;
typedef struct binary_t        binary_t;
typedef struct binary_t        string_t;
typedef struct environment_t   environment_t;

// internal typedefs ----------------------------------------------------------
typedef struct stack_t        stack_t;
typedef struct heap_t         heap_t;
typedef struct symbol_table_t symbol_table_t;
typedef struct ios_t          ios_t;
typedef struct ios_map_t      ios_map_t;
typedef struct gc_frame_t     gc_frame_t;

typedef union {
  double    fval;
  uintptr_t uval;

  struct {
    sint_t ival;
    uint_t meta;
  };
} C_data_t;

// globals --------------------------------------------------------------------
// core VM objects ------------------------------------------------------------
extern stack_t        *Stack;
extern heap_t         *Heap;
extern symbol_table_t *Symbols;
extern ios_map_t      *Ports;

extern gc_frame_t     *Saved;
extern jmp_buf         Toplevel;

// form names -----------------------------------------------------------------
extern value_t sym_quote, sym_do;
extern value_t sym_if, sym_and, sym_or;
extern value_t sym_val, sym_fun, sym_mac, sym_let, sym_labl;

// keywords -------------------------------------------------------------------
extern value_t kw_macro, kw_vargs;

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
#define pval(x)    ((void*)uval(x))

#define getf(type, x, field)            (as_##type(x)->field)
#define getf_s(type, x, field, fname)   (to##type(x, fname)->field)

#define ht_pred(t, T)  bool_t is_##t(value_t x) { return hitag(x) == T; }
#define wt_pred(t, T)  bool_t is_##t(value_t x) { return widetag(x) == T; }
#define ot_pred(t, T)  bool_t is_##t(value_t x) { return hitag(x) == OBJECT && widetag(car(x)) == T; }
#define val_pred(v, V) bool_t is_##v(value_t x) { return x == V; }

#define tag_hash(type, T)			\
  hash_t type##_hash(value_t x)			\
  {						\
    (void)x;					\
    return int64hash(T);			\
  }

#define for_cons(cx, x)						\
  for (;is_cons(*cx) && ((x=car(*cx))||1); *cx = cdr(*cx))

#endif
