#ifndef rascal_core_h
#define rascal_core_h

#include "common.h"
#include "types.h"
#include "instructions.h"

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

typedef uint_t   integer_t;
typedef bool_t   boolean_t;
typedef char_t   character_t;

typedef pair_t   cons_t;

typedef value_t (*Cbuiltin_t)(value_t *args, arity_t nargs);

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

// builtin dispatch tables
extern Cbuiltin_t    Builtins[form_pad];
extern bool_t      (*Ensure[form_pad])(value_t *base, size_t nargs);

extern char_t     *InstructionNames[num_instructions];

// type dispatch tables
extern void   (*obtracers[N_TYPES])(object_t* ob);
extern void   (*valtracers[N_TYPES])(value_t val);
extern void   (*obprinters[N_TYPES])(object_t *ob);
extern void   (*valprinters[N_TYPES])(value_t val);
extern void   (*finalizers[N_TYPES])(object_t *ob);

extern char_t  *TypeNames[N_TYPES];

extern size_t   TypeSizes[N_TYPES];

extern bool_t   TypeMembers[N_TYPES][N_TYPES]; // simple

static void initmembers(void) {
  for (int i=0; i<N_TYPES; i++)
    for (int j=0; j<N_TYPES; j++) TypeMembers[i][j] = i != type_none && (i == type_any || i == j);
}


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
