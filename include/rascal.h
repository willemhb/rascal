#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

// value types and tags
typedef uword                 value_t;

// core object types are defined here
typedef struct object_t       object_t;

// immediate types
typedef void                 *nul_t;
typedef bool                  bool_t;
typedef double                real_t;
typedef value_t             (*primitive_t)(value_t *args, size_t nargs);

// object types
typedef struct type_t         type_t;
typedef struct cons_t         cons_t;
typedef struct atom_t         atom_t;
typedef struct vector_t       vector_t;
typedef struct string_t       string_t;
typedef struct table_t        table_t;
typedef struct instructions_t instructions_t;
typedef struct lambda_t       lambda_t;
typedef struct control_t      control_t;
typedef struct closure_t      closure_t;
typedef struct environment_t  environment_t;
typedef struct namespace_t    namespace_t;

typedef union
{
  value_t      as_tagged;
  real_t       as_real;
  bool_t       as_bool;
  object_t    *as_object;
  type_t      *as_type;
  cons_t      *as_cons;
  atom_t      *as_atom;
  primitive_t  as_primitive;
  lambda_t    *as_lambda;
  vector_t    *as_vector;
} rl_data_t;

#define QNAN    0x7ff8000000000000ul

#define TMASK   0xffff000000000000ul
#define PMASK   (~TMASK)

#define NUL     0x7ffc000000000000ul
#define TRUE    0x7ffd000000000001ul
#define FALSE   0x7ffd000000000000ul
// #define GLYPH   0x7ffe000000000000ul
// #define FIXNUM  0x7fff000000000000ul
// #define SMINT   0xfffc000000000000ul
#define PFUNC   0xfffd000000000000ul
// #define STREAM  0xfffe000000000000ul
#define OBJECT  0xffff000000000000ul

static inline bool is_real( value_t x )      { return (x&QNAN) != QNAN; }
static inline bool is_nul( value_t x )       { return x == NUL; }
static inline bool is_bool( value_t x )      { return (x&TMASK) == FALSE; }
// static inline bool is_glyph( value_t x )     { return (x&TMASK) == GLYPH; }
// static inline bool is_fixnum( value_t x )    { return (x&TMASK) == FIXNUM; }
// static inline bool is_smint( value_t x )     { return (x&TMASK) == SMINT; }
static inline bool is_pfunc( value_t x )     { return (x&TMASK) == PFUNC; }
// static inline bool is_stream( value_t x )    { return (x&TMASK) == STREAM; }
static inline bool is_obj( value_t x )       { return (x&TMASK) == OBJECT; }

static inline bool rl_to_C_bool( value_t x ) { return !(x == NUL || x == false); }

#define as_value( x )  (((rl_data_t)(x)).as_tagged)
#define as_object( x ) (((rl_data_t)(x)).as_object)

#define as_ptr( x )    ((void*)_Generic((x),				\
					value_t:(((value_t)(x))&PMASK),	\
					default:((typeof(x))(x))))

#define tag_ptr( x, t ) (((value_t)(x))|(t))

#define as_obj( x )  ((object_t*)as_ptr(x))
#define as_pfunc( x ) ((primitive_t)as_ptr(x))

/* globals */
/* builtin types */
extern type_t TypeType, NulType, BoolType, RealType, PrimitiveType, ConsType,
  VectorType, InstructionsType, AtomType, LambdaType, ControlType, ClosureType,
  EnvironmentType;

static inline type_t *rl_typeof( value_t x )
{
  switch (x&TMASK)
    {
    case NUL:    return &NulType;
    case FALSE:  return &BoolType;
    case PFUNC:  return &PrimitiveType;
    case OBJECT: return *(type_t**)as_ptr(x);
    default:     return &RealType;
    }
}

static inline bool value_is_type( value_t x, type_t *type )
{
  return rl_typeof(x) == type;
}

#endif
