/*
  femtoLisp
  a minimal interpreter for a minimal lisp dialect
  this lisp dialect uses lexical scope and self-evaluating lambda.
  it supports 30-bit integers, symbols, conses, and full macros.
  it is case-sensitive.
  it features a simple compacting copying garbage collector.
  it uses a Scheme-style evaluation rule where any expression may appear in
    head position as long as it evaluates to a function.
  it uses Scheme-style varargs (dotted formal argument lists)
  lambdas can have only 1 body expression; use (progn ...) for multiple
    expressions. this is due to the closure representation
    (lambda args body . env)
  lispf is a fork that provides an #ifdef FLOAT option to use single-precision
  floating point numbers instead of integers, albeit with even less precision
  than usual---only 21 significant mantissa bits!
  it is now also being used to test a tail-recursive evaluator.
  by Jeff Bezanson
  Public Domain
*/

#include "core.h"

#include "templates/table.h"
#include "templates/arr.h"
#include "templates/str.h"

#include "utils/str.h"
#include "utils/num.h"
#include "utils/arr.h"
#include "utils/table.h"

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
#define INT       0x7ffd000000000000ul
#define CHR       0x7ffe000000000000ul
#define BOOL      0xffff000000000000ul
#define PRIM      0xfffc000000000000ul
#define NUL       0xfffd000000000000ul
#define PTR       0xfffe000000000000ul
#define ARITY     0xffff000000000000ul

#define val_tag(x)  ((x)&ARITY)



typedef uintptr_t  val_t;

typedef uint32_t   type_t;
typedef uint16_t   opcode_t;

typedef struct obj_t   obj_t;

typedef struct cons_t  cons_t;
typedef struct entry_t entry_t;
typedef struct 

typedef struct atom_t  atom_t;
typedef struct func_t  func_t;
typedef struct cntl_t  cntl_t;
typedef struct vec_t   vec_t;
typedef struct map_t   map_t;
typedef struct bin_t   bin_t;

typedef struct 

// function pointer types
typedef hash_t  (*hash_fn_t)(val_t val);
typedef int     (*ord_fn_t)(val_t x, val_t y);
typedef arity_t (*prin_fn_t)(obj_t *io, val_t x);
typedef val_t   (*call_fn_t)(val_t f, val_t *args, arity_t n);

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
    symt_type,
    code_type,
    clo_type,
    envt_type,
    meth_type,
    cntl_type,
    entry_type,
    var_type,

    num_val_types
  };

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

struct atom_t
{
  OBJ_HEAD
  hash_t  hash;
  idno_t  idno;
  str_t  *name;
};

struct func_t
{
  OBJ_HEAD

  str_t  *name;
  envt_t *ns;

  union
  {
    opcode_t primitive;
    
  };
};

struct vec_t
{
  ARRAY_SLOTS(OBJ_HEAD, val_t);
};

typedef vec_t stack_t;

struct map_t
{
  ORDERED_TABLE_SLOTS(OBJ_HEAD, obj_t*);
};

typedef struct map_t dict_t;
typedef struct map_t set_t;
typedef struct map_t symt_t;

// internal types


// globals

struct
{
  stack_t   stack;

  code_t   *code;
  envt_t   *envt;

  opcode_t *ip;
  arity_t   bp;    // arguments
  arity_t   cp;    // 
} Vm;
