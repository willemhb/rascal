#ifndef rascal_core_h
#define rascal_core_h

#include "common.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

typedef struct object_t {
  value_t type;
  uchar_t space[0];
} object_t;

typedef struct symbol_t   symbol_t;
typedef struct pair_t     pair_t;
typedef struct tuple_t    tuple_t;
typedef struct string_t   string_t;
typedef struct binary_t   binary_t;
typedef struct table_t    table_t;
typedef struct function_t function_t;
typedef struct port_t     port_t;
typedef struct error_t    error_t;

typedef ushort_t builtin_t;
typedef uint_t   integer_t;
typedef bool_t   boolean_t;
typedef char_t   character_t;

typedef pair_t   cons_t;

typedef value_t (*Cbuiltin_t)(value_t *args, arity_t nargs);

#define tag_object    0x00
#define tag_immediate 0x01
#define tag_moved     0x02
#define tag_header    0x03

typedef enum {
  /* fucked up types */
  type_null      = 0x00 | tag_object,
  type_none      = 0x00 | tag_immediate,

  /* non-literal types */
  type_symbol    = 0x04 | tag_header,
  type_pair      = 0x08 | tag_header,
  type_cons      = 0x0c | tag_header,

  /* literal types */
  type_table     = 0x10 | tag_header,
  type_string    = 0x14 | tag_header,
  type_binary    = 0x18 | tag_header,
  type_tuple     = 0x1c | tag_header,
  type_function  = 0x20 | tag_header,
  type_port      = 0x24 | tag_header,
  type_error     = 0x28 | tag_header,

  /* immediate types */
  type_boolean   = 0x04 | tag_immediate,
  type_character = 0x08 | tag_immediate,
  type_builtin   = 0x0c | tag_immediate,

  /* numeric types */
  type_integer   = 0x10 | tag_immediate,
  type_fixnum    = 0x14 | tag_immediate,
  type_arity     = 0x18 | tag_immediate,
  type_index     = 0x1c | tag_immediate,
  type_idno      = 0x20 | tag_immediate
} type_t;

#define rnull   ((value_t)type_null)
#define rnone   (((value_t)0xffffffff00000000ul)|type_none)
#define rtrue   (((value_t)0x0000000100000000ul)|type_boolean)
#define rfalse  ((value_t)type_boolean)

// globals --------------------------------------------------------------------
#define N_STACK 8192
#define N_HEAP  N_STACK

extern ulong_t   Symcnt;
extern object_t *Symbols, *Globals, *Error, *Ins, *Outs, *Errs;

extern value_t Stack[N_STACK], Dump[N_STACK], Function;

extern index_t Sp, Bp, Fp, Dp;

extern uchar_t *Heap, *Reserve, *Free, *HeapMap, *ReserveMap;
extern size_t  NHeap, HeapUsed, NReserve, ReserveUsed;
extern bool_t  Collecting, Grow, Grew;
extern float_t Collectf, Resizef, Growf;

extern Cbuiltin_t  Builtins[];

extern char_t     *BuiltinNames[],
                  *TypeNames[];

extern size_t      TypeSizes[];

// utility macros -------------------------------------------------------------
#define tag(x)     ((x)&3)
#define wtag(x)    ((x)&255)
#define ishift(x)  ((x)>>32)
#define wishift(x) ((x)>>8)
#define tagv(x,t)  (((value_t)(x))|(t))
#define ptr(x)     ((void_t*)(((value_t)(x))&~3ul))
#define obhead(x)  (*((value_t*)ptr(x)))
#define obtype(x)  (obhead(x)&255)
#define obtag(x)   (obhead(x)&3)
#define obsize(x)  (obhead(x)>>8)
#define imtype(x)  (wtag(x))

#define objectp(x)    (tag(x)==tag_object)
#define immediatep(x) (tag(x)==tag_immediate)
#define nullp(x)      ((x)==rnull)
#define nonep(x)      ((x)==rnone)
#define truep(x)      ((x)==rtrue)
#define falsep(x)     ((x)==rfalse)

#endif
