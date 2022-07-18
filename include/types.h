#ifndef rascal_types_h
#define rascal_types_h

#include <stdio.h>

#include "strutils.h"
#include "numutils.h"

#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

#define tag_fixnum    0b000
#define tag_immediate 0b001
#define tag_pair      0b010
#define tag_boxed     0b011

// #define flag_gray     0b100
// #define flag_proper   0b100

typedef enum
  {
    type_fixnum    = 0x00|tag_fixnum,

    type_type      = 0x00|tag_immediate,
    type_boolean   = 0x08|tag_immediate,

    type_cons      = 0x00|tag_pair,
    type_nil       = 0x08|tag_pair,

    type_builtin   = 0x00|tag_boxed,
    type_closure   = 0x08|tag_boxed,
    type_symbol    = 0x10|tag_boxed,
    type_vector    = 0x18|tag_boxed,
    type_binary    = 0x20|tag_boxed
  } type_t;

static const value_t tag_type     = (type_type<<24)|tag_immediate;
static const value_t tag_boolean  = (type_boolean<<24)|tag_immediate;

static const value_t tag_builtin  = (type_builtin<<3)|tag_type;
static const value_t tag_closure  = (type_closure<<3)|tag_type;
static const value_t tag_symbol   = (type_symbol<<3)|tag_type;
static const value_t tag_vector   = (type_vector<<3)|tag_type;
static const value_t tag_binary   = (type_binary<<3)|tag_type;

static const value_t val_nil     = (type_nil<<24)|tag_immediate;
static const value_t val_true    = (type_boolean<<24)|(1<<3)|tag_immediate;
static const value_t val_false   = (type_boolean<<24)|tag_immediate;
static const value_t val_evec    = (type_vector<<24)|tag_immediate;

static const value_t val_ebin_s8  = (type_binary<<24)|(C_sint8<<3)|tag_immediate;
static const value_t val_ebin_u8  = (type_binary<<24)|(C_uint8<<3)|tag_immediate;
static const value_t val_ebin_s16  = (type_binary<<24)|(C_sint16<<3)|tag_immediate;
static const value_t val_ebin_u16  = (type_binary<<24)|(C_uint16<<3)|tag_immediate;
static const value_t val_ebin_s32  = (type_binary<<24)|(C_sint32<<3)|tag_immediate;
static const value_t val_ebin_u32  = (type_binary<<24)|(C_uint32<<3)|tag_immediate;
static const value_t val_ebin_s64  = (type_binary<<24)|(C_sint64<<3)|tag_immediate;
static const value_t val_ebin_f64  = (type_binary<<24)|(C_float64<<3)|tag_immediate;

static const value_t val_unbound = (type_symbol<<24)|tag_immediate;
static const value_t val_forward = (type_builtin<<24)|tag_immediate;

typedef struct {
  type_t  type;
  flags_t Ctype         :  3;
  flags_t encoding      :  3;
  flags_t is_array      :  1;
  flags_t is_traversed  :  1;
  flags_t flags         :  8;
  flags_t base_size     : 16;
} header_t;

typedef struct {
  header_t base;
  uchar space[0];
} object_t;

typedef struct {
  header_t base;
  size_t   size;
  value_t *data;
  size_t   length;
} vector_t;

typedef struct {
  header_t base;
  size_t   size;
  uchar   *data;
  size_t   length;
} binary_t;

typedef struct {
  header_t base;
  void (*callback)( size_t n );
} builtin_t;

typedef struct {
  header_t base;
  value_t  envt;
  value_t  vals;
  value_t  code;
} closure_t;

typedef struct {
  header_t base;
  value_t  bind;
  idno_t   idno;
  hash_t   hash;
  char     name[0];
} symbol_t;

typedef long fixnum_t;
typedef bool boolean_t;

typedef struct {
  value_t car;
  value_t cdr;
} cons_t;


typedef struct symbols_t {
  struct symbols_t *left, *right;
  symbol_t base;
} symbols_t;

#define uval(x)       ((x)>>3)
#define ival(x)       ((long)uval(x))
#define cval(x)       (uval(x)&0x1fffff)
#define pval(x)       ((void*)(((value_t)(x))&~7ul))

#define tag(x)        ((x)&7)
#define tagp(p, t)    ((((value_t)(p))&~7ul)|(t))
#define tagi(i, t)    ((((value_t)(i))<<3)|(t))

#define fix_1 8

#endif
