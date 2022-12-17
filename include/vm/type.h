#ifndef rl_vm_type_h
#define rl_vm_type_h

#include "rascal.h"

/* commentary 

   Internal type representation (within the language types are represented by
   their constructors). 

   notes:
   
   - internal type representations are hash-consed for fast  */

/* C types */
typedef struct type_t type_t;
typedef struct data_t data_t;
typedef struct union_t union_t;
typedef struct alias_t alias_t;
typedef enum kind_t kind_t;
typedef enum runtime_t runtime_t;

enum kind_t { no_kind=-1, none_kind, data_kind, alias_kind, union_kind, any_kind };

/* dispatch key for fundamental data type */
enum runtime_t {
  stream_runtime, native_runtime, nul_runtime, bool_runtime, glyph_runtime,
  small_runtime, fixnum_runtime, real_runtime,

  symbol_runtime, function_runtime, cons_runtime, string_runtime, vector_runtime,
  dict_runtime, set_runtime, u16_array_runtime, record_runtime, control_runtime,
  closure_runtime, bytecode_runtime, namespace_runtime, variable_runtime
};

#define TYPE_HEADER                             \
  rl_function_t *constructor;                   \
  ulong hash, idno;                             \
  kind_t kind;                                  \
  ushort builtin

struct type_t
{
  TYPE_HEADER;
};

struct data_t
{
  TYPE_HEADER;
  ushort runtime_type;
  size_t value_size;
  size_t object_size;

  /* slots (record types only) */
  rl_slots_t *slots;
};

struct union_t
{
  TYPE_HEADER;

  type_t *left;
  type_t *right;
};

struct alias_t
{
  TYPE_HEADER;

  type_t *aliased; // aliased type
  type_t *base; // if aliased is also an alias, thoe root (non-aliased) type
};

/* globals */
/* API */
type_t    *user_type( rl_value_t value );
data_t    *data_type( rl_value_t value );
rl_type_t *constructor( rl_value_t value );

/* runtime dispatch */
void rl_vm_type_init( void );
void rl_vm_type_mark( void );
void rl_vm_type_cleanup( void );

/* convenience */

#endif
