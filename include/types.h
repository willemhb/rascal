#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>

#include "common.h"
#include "describe/utils.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

#define tag_immediate 0x00 // 000
#define tag_fixnum    0x01 // 001
#define tag_cons      0x02 // 010
#define tag_symbol    0x03 // 011
#define tag_function  0x04 // 100
#define tag_vector    0x05 // 101
#define tag_string    0x06 // 110
#define tag_port      0x07 // 111

#define tag_character (0x0a000000|tag_immediate)

// these live above the valid unicode range
#define val_true      (0x08000000|tag_immediate)
#define val_false     (0x08800000|tag_immediate)
#define val_nil       (0x09000000|tag_immediate)

typedef enum
  {
   type_fixnum=tag_fixnum,
   type_cons,
   type_symbol,
   type_function,
   type_vector,
   type_string,
   type_port,
   type_bool,
   type_nil,
   type_character
} type_t;

typedef struct {
  value_t  flags;
  value_t *data;
  size_t length, size;
} vector_t;

typedef struct {
  value_t flags;
  char *data;
  size_t length, size;
} string_t;

typedef struct {
  value_t bind;
  char *name;
  ulong hash, idno;
} symbol_t;

#define type_pad 16

typedef long fixnum_t;
typedef void (*builtin_t)( size_t n );

typedef struct {
  value_t car;
  value_t cdr;
} cons_t;

typedef struct symbols_t {
  struct symbols_t *left, *right;
  symbol_t base;
} symbols_t;

typedef enum {
  /* 0-argument codes */
  op_noop, op_done,

  /* 1-argument opcodes */
  op_loadc, op_loadg, 
} opcode_t;


#define uval(x)       ((x)>>3)
#define ival(x)       ((long)uval(x))
#define pval(x)       ((void*)(((value_t)(x))&~7ul))

#define tag(x)        ((x)&7)
#define tagp(p, t)    ((((value_t)(p))&~7ul)|(t))
#define tagl(l, t)    ((((value_t)(l))<<3)|(t))

#endif
