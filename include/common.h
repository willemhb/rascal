#ifndef rascal_common_h
#define rascal_common_h

#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <uchar.h>

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
typedef sint_t         order_t;
typedef uint_t         flags_t;
typedef uint_t         arity_t;
typedef uint_t         index_t;
typedef bool           bool_t;

/* enum types */
typedef enum {
  C_sint8, C_uint8, C_ascii, C_latin1, C_utf8,

  C_sint16, C_uint16, C_utf16,

  C_sint32, C_uint32, C_utf32, C_float,

  C_sint64, C_uint64, C_pointer, C_double
} Ctype_t;

/* a tagged lisp value */
typedef uintptr_t value_t;


// tags -----------------------------------------------------------------------
#define QNAN        0x7ffc000000000000ul
#define SIGN        0x8000000000000000ul

typedef enum {
  /* values that can be derived from the tag that unambiguously identify
     the object's internal representation.

     the first 32 reprs are object types (to fit the heap bitmap).

     the next 32 reprs are reserved for pseudo-object singletons and sentinels
     derived from object reprs, and are notionally related to the types
     that share their 5 low bits.

     reprs >= 64 are for immediate types.

     reprs are related to types, but not identical. They are intended to:

     * decouple the semantics of types from runtime runtime representations.

     * provide a quick way for internal methods to dispatch on a small, fixed
       set of keys.
       
     * provide a quick way for the types of builtin objects to be looked up.

     reprs are also related to tags. They compress the width of the tag range to
     about 128, making dispatch tables for internal methods more practicable.
  */
  /* true objects */
  REPR_SYMBOL       =  0,
  REPR_CONS         =  1,
  REPR_PAIR         =  2,

  REPR_COMPILED     =  3,
  REPR_LAMBDA       =  4,
  REPR_NATIVE       =  5,
  REPR_BUILTIN      =  6,
  REPR_GENERIC      =  7,
  REPR_CONTROL      =  8,

  REPR_ENTRY        =  9,
  REPR_CENTRY       = 10,
  REPR_HAMT         = 11,

  REPR_DICT         = 12,
  REPR_SET          = 13,
  REPR_VECTOR       = 14,
  REPR_TUPLE        = 15,

  REPR_STRING       = 16,
  REPR_BINARY       = 17,
  REPR_BIGINT       = 18,
  REPR_RATIO        = 19,
  REPR_COMPLEX      = 20,

  REPR_PORT         = 21,
  REPR_IOBUFFER     = 22,

  REPR_MODULE       = 23,
  REPR_ENVIRONMENT  = 24,
  REPR_UPVALUE      = 25,

  REPR_STACK        = 26,
  REPR_HEAP         = 27,
  REPR_PROCESS      = 28,

  REPR_EFFECT       = 29,
  REPR_METHODS      = 30,

  REPR_RECORD       = 31,

  /* pseudo-objects (empty singletons and other sentinels) */
  REPR_UNBOUND      = 32 | REPR_SYMBOL,
  REPR_NIL          = 32 | REPR_CONS,
  REPR_EHAMT        = 32 | REPR_HAMT,
  REPR_EDICT        = 32 | REPR_DICT,
  REPR_ESET         = 32 | REPR_SET,
  REPR_EVECTOR      = 32 | REPR_VECTOR,
  REPR_ETUPLE       = 32 | REPR_TUPLE,
  REPR_ESTRING      = 32 | REPR_STRING,
  REPR_EBINARY      = 32 | REPR_BINARY,
  REPR_EOS          = 32 | REPR_PORT,
  REPR_EMODULE      = 32 | REPR_MODULE,
  REPR_EENVIRONMENT = 32 | REPR_ENVIRONMENT,
  REPR_EUPVAULE     = 32 | REPR_UPVALUE,
  REPR_NONE         = 32 | REPR_RECORD,

  /* other immediates */
  REPR_POINTER      = 64,
  REPR_BOOLEAN      = 65,
  REPR_CHARACTER    = 66,
  REPR_INTEGER      = 67,
  REPR_FLONUM       = 68
} repr_t;

#define REPR_PAD 128

// tags -----------------------------------------------------------------------
// true object tags -----------------------------------------------------------
#define OBJECT0      0x7ffc000000000000ul
#define OBJECT1      0x7ffd000000000000ul

#define SYMBOL       (OBJECT0| 0)
#define CONS         (OBJECT0| 1)
#define PAIR         (OBJECT0| 2)
#define COMPILED     (OBJECT0| 3)
#define LAMBDA       (OBJECT0| 4)
#define NATIVE       (OBJECT0| 5)
#define BUILTIN      (OBJECT0| 6)
#define GENERIC      (OBJECT0| 7)
#define CONTROL      (OBJECT0| 8)
#define ENTRY        (OBJECT0| 9)
#define CENTRY       (OBJECT0|10)
#define HAMT         (OBJECT0|11)
#define DICT         (OBJECT0|12)
#define SET          (OBJECT0|13)
#define VECTOR       (OBJECT0|14)
#define TUPLE        (OBJECT0|15)
#define STRING       (OBJECT1| 0)
#define BINARY       (OBJECT1| 1)
#define BIGINT       (OBJECT1| 2)
#define RATIO        (OBJECT1| 3)
#define COMPLEX      (OBJECT1| 4)
#define PORT         (OBJECT1| 5)
#define IOBUFFER     (OBJECT1| 6)
#define MODULE       (OBJECT1| 7)
#define ENVIRONMENT  (OBJECT1| 8)
#define UPVALUE      (OBJECT1| 9)
#define STACK        (OBJECT1|10)
#define HEAP         (OBJECT1|11)
#define PROCESS      (OBJECT1|12)
#define EFFECT       (OBJECT1|13)
#define METHODS      (OBJECT1|14)
#define RECORD       (OBJECT1|15)

// pseudo-object tags ---------------------------------------------------------
#define OBJECT2     0x7ffe000000000000ul

#define UNBOUND      (OBJECT2|SYMBOL)
#define NIL          (OBJECT2|CONS)
#define EHAMT        (OBJECT2|HAMT)
#define EDICT        (OBJECT2|DICT)
#define ESET         (OBJECT2|SET)
#define EVECTOR      (OBJECT2|VECTOR)
#define ETUPLE       (OBJECT2|TUPLE)
#define ESTRING      (OBJECT2|STRING)
#define EBINARY      (OBJECT2|BINARY)
#define EOS          (OBJECT2|PORT)
#define EMODULE      (OBJECT2|MODULE)
#define EENVIRONMENT (OBJECT2|ENVIRONMENT)
#define EUPVALUE     (OBJECT2|UPVALUE)
#define NONE         (OBJECT2|RECORD)

// other immediate tags -------------------------------------------------------
#define POINTER     0xfffc000000000000ul /* pointer to data outside of heap */
#define BOOLEAN     0xfffd000000000000ul /* true | false */
#define CHARACTER   0xfffe000000000000ul /* up to 32-bit character (encoding included) */
#define INTEGER     0xffff000000000000ul /* 48-bit signed integer */

/* pseudo-object tags (empty singletons and other sentinels) */

/* other constants */
#define TRUE        (BOOLEAN|1)
#define FALSE       (BOOLEAN|0)
#define PNAN        0x7ff8000000000000ul
#define MNAN        0xfff8000000000000ul
#define PINF        0x7ff0000000000000ul
#define MINF        0xfff0000000000000ul

// immediate typedefs ---------------------------------------------------------
typedef intptr_t               integer_t;
typedef double                 flonum_t;
typedef char                   character_t;
typedef bool                   boolean_t;
typedef void*                  pointer_t;

// interpreter types ----------------------------------------------------------
// non-trival s-expression types ----------------------------------------------
typedef struct symbol_t        symbol_t;
typedef struct pair_t          pair_t; // an improper cons cell
typedef struct pair_t          cons_t; // a proper cons cell
typedef struct pair_t          cell_t; // any cons cell
typedef struct pair_t*         list_t; // nil | cons

// function types -------------------------------------------------------------
typedef struct compiled_t      compiled_t;
typedef struct lambda_t        lambda_t;
typedef struct generic_t       generic_t;
typedef struct native_t        native_t;
typedef struct builtin_t       builtin_t;
typedef struct control_t       control_t;

// table entry types ----------------------------------------------------------
typedef struct entry_t         entry_t;
typedef struct entry_t         centry_t;

// table types ----------------------------------------------------------------
typedef struct hamt_t          hamt_t;
typedef struct table_t         table_t;
typedef struct table_t         dict_t;
typedef struct table_t         set_t;
typedef struct table_t         vector_t;

// array types ----------------------------------------------------------------
typedef struct tuple_t         tuple_t;
typedef struct binary_t        binary_t;
typedef struct binary_t        string_t;

// large numeric types --------------------------------------------------------
typedef struct bigint_t        bigint_t;
typedef struct ratio_t         ratio_t;
typedef struct complex_t       complex_t;

// io types -------------------------------------------------------------------
typedef struct port_t          port_t;
typedef struct io_buffer_t     io_buffer_t;

// namespace types ------------------------------------------------------------
typedef struct module_t        module_t;
typedef struct environment_t   environment_t;
typedef struct upvalue_t       upvalue_t;

// runtime support types ------------------------------------------------------
typedef struct stack_t         stack_t;
typedef struct heap_t          heap_t;
typedef struct process_t       process_t;
typedef struct effect_t        effect_t;
typedef struct methods_t       methods_t;

// user types -----------------------------------------------------------------
typedef struct record_t        record_t;

// internal typedefs ----------------------------------------------------------
typedef struct gc_frame_t {
  size_t      length;
  struct gc_frame_t *next;
  value_t    *saved[];
} gc_frame_t;

typedef union {
  double    fval;
  uintptr_t uval;
  pointer_t pval;

  struct {
    slong_t lval : 48;
    slong_t _pad : 16;
  };
} C_data_t;

// globals --------------------------------------------------------------------
// internal methods dispatch --------------------------------------------------
extern size_t  BaseSizes[REPR_PAD];
extern Ctype_t Ctypes[REPR_PAD];
extern bool_t  MightCycle[REPR_PAD];
extern size_t  (*Sizeof[REPR_PAD])(value_t x);
extern void   *(*Allocate[REPR_PAD])(size_t n);
extern value_t (*Relocate[REPR_PAD])(value_t x);
extern void    (*Finalize[REPR_PAD])(value_t x);
extern hash_t  (*Hash[REPR_PAD])(value_t x);
extern sint_t  (*Order[REPR_PAD])(value_t x, value_t y);
extern size_t  (*Print[REPR_PAD])(value_t x, port_t *ios);

// core VM objects ------------------------------------------------------------
extern value_t Stack, Heap, Module, Modules,
  BuiltinTypes, TypeCount, Symbols, SymCount;

extern gc_frame_t *Saved;
extern jmp_buf     Toplevel;

// form names -----------------------------------------------------------------
extern value_t sym_quote, sym_do, sym_cntl, sym_hndl;
extern value_t sym_cond, sym_case, sym_if, sym_and, sym_or;
extern value_t sym_val, sym_fun, sym_mac, sym_typ, sym_let, sym_labl;

// utilities ------------------------------------------------------------------
#define rsp_aligned  __attribute__((aligned(sizeof(pair_t))))

#define rsp_unlikely(c) __builtin_expect((c), 0)

#define ALCMASK    0xfffd000000000000ul
#define HTMASK     0xffff000000000000ul
#define DTMASK     0x0003000000000000ul
#define OTMASK     0xffff00000000000ful
#define LTMASK     0x000000000000000ful

#define LMASK       0x0000fffffffffffful
#define IMASK       0x00000000fffffffful
#define PMASK       0x0000fffffffffffful
#define OMASK       0x0000fffffffffff0ul

#define hitag(x)    ((x)&HTMASK)
#define dsctag(x)   ((x)&DTMASK)
#define lotag(x)    ((x)&LTMASK)
#define obtag(x)    ((x)&OTMASK)
#define tagval(x,t) (((value_t)(x))|(t))

#define as_type(ctype, cnvt, x) ((ctype)cnvt(x))

#define getf(type, x, field)    (as_##type(x)->field)

#define ht_pred(t, T)  bool_t is_##t(value_t x) { return hitag(x) == T; }
#define ot_pred(t, T)  bool_t is_##t(value_t x) { return obtag(x) == T; }
#define val_pred(v, V) bool_t is_##v(value_t x) { return x == V; }

#define tag_hash(type, T)			\
  hash_t type##_hash(value_t x)			\
  {						\
    (void)x;					\
    return int64hash(T);			\
  }

#endif
