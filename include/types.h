#ifndef rascal_types_h
#define rascal_types_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "common.h"

// tags -----------------------------------------------------------------------
enum
  {
    tag_atom     =0,
    tag_list     =1,
    tag_function =2,
    tag_port     =3,
    tag_binary   =4,
    tag_vector   =5,
    tag_table    =6,
    tag_character=7,
    tag_real     =8
  };

#define QNAN      0x7ffc000000000000ul
#define SIGN_BIT  0x8000000000000000ul

#define LIST      0x7ffc000000000000ul
#define ATOM      0x7ffd000000000000ul
#define FUNCTION  0x7ffe000000000000ul
#define PORT      0x7fff000000000000ul
#define BINARY    0xfffc000000000000ul
#define VECTOR    0xfffd000000000000ul
#define TABLE     0xfffe000000000000ul
#define CHARACTER 0xffff000000000000ul
#define ESCAPE    0xffff000000000000ul

#define TMASK     0xffff000000000000ul
#define PMASK     0x0000fffffffffffful
#define LMASK     0x0000fffffffffffful
#define CMASK     0x00000000fffffffful

#define NIL        LIST
#define TRUE       (ATOM|1)
#define FALSE      (ATOM|0)
#define EOS        PORT
#define EMPTYSTR   (BINARY|C_ascii)
#define EMPTYVEC   VECTOR
#define EMPTYTABLE TABLE

// C types --------------------------------------------------------------------
typedef uintptr_t value_t;
typedef struct object_t object_t;

// immediate types ------------------------------------------------------------
typedef double real_t;
typedef char   character_t;
typedef bool   boolean_t;

// object types ---------------------------------------------------------------
typedef struct cons_t     cons_t;
typedef struct symbol_t   symbol_t;
typedef struct function_t function_t;
typedef struct port_t     port_t;
typedef struct binary_t   binary_t;
typedef struct vector_t   vector_t;
typedef struct table_t    table_t;

// macros & utils -------------------------------------------------------------
#define HEADER object_t base

#define tag(x)  ((x)&TMASK)
#define pval(x) ((void*)((x)&PMASK))
#define dval(x) (((ieee64_t)(x)).fp)
#define cval(x) ((int)((x)&CMASK))
#define cflg(x) (((x)>>32)&UINT16_MAX)

#define asa(ctype, x, cnvt)          ((ctype)cnvt(x))
#define getf(ctype, x, field)        (asa(ctype, x, pval)->field)
#define getf_s(type, x, field, func) (to##type(x, func)->field)

#define tag_p(type, TAG)      bool is_##type(value_t x) { return tag(x) == TAG; }
#define value_p(value, VALUE) bool is_##value(value_t x) { return x == VALUE; }
#define non_empty_tag_p(type, TAG)			\
  bool is_##type(value_t x)				\
  {							\
    return tag(x) == TAG && !(x&7) && !!(x&PMASK);	\
  }

#define ob_flag_p(flag, TAG, FLAG)			\
  bool is_##flag(value_t x)				\
  {							\
    return is_object(x)					\
      && tag(x) == TAG					\
      && !!(ob_flags(x)&FLAG);				\
  }

#define im_flag_p(flag, TAG, FLAG)		\
  bool is_##flag(value_t x)			\
  {						\
    return is_immediate(x)			\
      && tag(x) == TAG				\
      && !!(cflg(x)&FLAG);			\
  }

#define flag_p(flag, TAG, FLAG)			\
  bool is_##flag(value_t x)			\
  {						\
    if (tag(x) != TAG)				\
      return false;				\
    else if (is_immediate(x))			\
      return !!(cflg(x)&FLAG);			\
    else					\
      return !!(ob_flags(x)&FLAG);		\
  }

#endif
