#ifndef rascal_types_h
#define rascal_types_h

#include <stdio.h>

#include "strutils.h"
#include "numutils.h"

#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

#define bits64

#define tag_fixnum    0x00 // *000
#define tag_immediate 0x01 // *001
#define tag_system    0x02 // *010
#define tag_function  0x03 // *011
#define tag_bignum    0x04 // 0100

#ifdef bits64

#define tag_binary    0x05 // 0101
#define tag_string    0x06 // 0110
#define tag_vector    0x07 // 0111
#define tag_dict      0x0c // 1100
#define tag_set       0x0d // 1101
#define tag_cons      0x0e // 1110
#define tag_symbol    0x0f // 1111

#else

#define tag_table     0x05
#define tag_cons      0x06
#define tag_symbol    0x07

#endif

static const value_t tag_character = tag_immediate << 24 | tag_immediate;
static const value_t tag_bool      = (0x10|tag_immediate) << 24 | tag_immediate;
static const value_t tag_nil       = (0x10|tag_cons) << 24 | tag_immediate;

typedef enum
  {
    type_fixnum    = 0x00|tag_fixnum,
    type_binary    = 0x00|tag_binary,
    type_string    = 0x00|tag_string,
    type_vector    = 0x00|tag_vector,
    type_dict      = 0x00|tag_dict,
    type_set       = 0x00|tag_set,
    type_symbol    = 0x00|tag_symbol,

    type_character = 0x00|tag_immediate,
    type_bool      = 0x08|tag_immediate,

    type_builtin   = 0x00|tag_function,
    type_closure   = 0x08|tag_function,

    type_port      = 0x00|tag_system,
    type_error     = 0x08|tag_system,

    type_double    = 0x00|tag_bignum,

    type_cons      = 0x00|tag_cons,
    type_nil       = 0x10|tag_cons
  } type_t;


static const value_t val_true  = tag_bool | 0x8;
static const value_t val_false = tag_bool;
static const value_t val_nil   = tag_nil;

static const value_t val_fptr    = tag_cons;
static const value_t val_unbound = tag_symbol;

static const value_t val_EOF        = (type_port << 24) | tag_immediate;
static const value_t val_empty_str  = (type_string << 24) | tag_immediate;
static const value_t val_empty_vec  = (type_vector << 24) | tag_immediate;
static const value_t val_empty_set  = (type_set << 24) | tag_immediate;
static const value_t val_empty_dict = (type_dict << 24) | tag_immediate;

typedef struct {
  bool boxed;
  int  flags;

  value_t *data;
  size_t length, size;
} vector_t;

typedef struct {
  encoding_t encoding;
  int flags;
  char *data;
  size_t length, size;
} string_t;

typedef struct {
  Ctype_t ctype;
  int flags;
  
  void *data;
  size_t length, size;
} binary_t;

typedef struct {
  value_t key, left, right;
  hash_t hash;
} set_t;

typedef struct {
  value_t entry, left, right;
  hash_t hash;
} dict_t;

typedef struct {
  value_t bind;
  char *name;
  ulong hash, idno;
} symbol_t;

typedef struct {
  value_t name;
  value_t envt;
  value_t vals;
  value_t code;
} closure_t;

#define type_pad 256

typedef long fixnum_t;
typedef bool boolean_t;
typedef void (*builtin_t)( size_t n );

typedef struct {
  value_t car;
  value_t cdr;
} cons_t;

typedef cons_t pair_t;

typedef struct symbols_t {
  symbol_t base;
  struct symbols_t *left, *right;
} symbols_t;

#define uval(x)       ((x)>>3)
#define ival(x)       ((long)uval(x))
#define cval(x)       (uval(x)&0xfffffful)
#define pval(x)       ((void*)(((value_t)(x))&~7ul))

#define tag(x)        ((x)&7)
#define tagp(p, t)    ((((value_t)(p))&~7ul)|(t))
#define tagl(l, t)    ((((value_t)(l))<<3)|(t))

#endif
