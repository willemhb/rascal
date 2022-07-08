#ifndef rascal_types_h
#define rascal_types_h

#include "common.h"
#include "rtypes.h"

#define tag_object    0x00
#define tag_immediate 0x01
#define tag_moved     0x02
#define tag_header    0x03

enum type_t {
  /*
    object tag used for singleton types
   */
  type_null      = 0x00 | tag_object,
  /*
    moved tag used for union types (constructable but no concrete objects with this type)
  */

  type_value     = 0x04 | tag_moved,
  type_list      = 0x08 | tag_moved,
  type_mapping   = 0x0c | tag_moved,
  type_array     = 0x10 | tag_moved,
  type_function  = 0x14 | tag_moved,

  /*
    lowest part of immediate tag used for abstract types
    (not constructable and no concrete instances).
  */

  type_none      = 0x00 | tag_immediate,
  type_any       = 0x04 | tag_immediate,

  /* non-literal types */
  type_symbol    = 0x00 | tag_header,
  type_pair      = 0x04 | tag_header, // a list pair
  type_assc      = 0x08 | tag_header, // a pair storing a key and value
  
  /* sequence & collection types */
  /* array types */
  type_tuple     = 0x0c | tag_header,
  type_vector    = 0x10 | tag_header,
  type_binary    = 0x14 | tag_header,
  type_string    = 0x18 | tag_header,
  type_buffer    = 0x1c | tag_header,

  /* mapping types */
  type_table     = 0x20 | tag_header,
  type_dict      = 0x24 | tag_header,
  type_set       = 0x28 | tag_header,

  /* function object types types */
  type_closure   = 0x2c | tag_header,

  /* misc internal types */
  type_port      = 0x30 | tag_header,
  type_error     = 0x34 | tag_header,
  type_bytecode  = 0x38 | tag_header,
  type_envt      = 0x3c | tag_header,

  /* immediate types */
  /* non-numbers with integer representations */
  type_boolean   = 0x08 | tag_immediate,
  type_character = 0x0c | tag_immediate,

  /* instructions */
  type_builtin   = 0x10 | tag_immediate,
  type_form      = 0x14 | tag_immediate,
  type_opcode    = 0x18 | tag_immediate,

  /* numeric types */
  type_integer   = 0x1c | tag_immediate,
  type_fixnum    = 0x20 | tag_immediate,

  /* higher order types */
  // type_type      = 0xe0 | tag_immediate,
  // type_class     = 0xe4 | tag_immediate,
  // type_effect    = 0xe8 | tag_immediate,

  /* any user defined primitive that fits 32-bits */
  // type_Cprim32   = 0xec | tag_immediate,

  /* internal types (these are fixnums with special tags to hint at their meaning) */
  type_arity     = 0xf0 | tag_immediate,
  type_size      = 0xf4 | tag_immediate,
  type_index     = 0xf8 | tag_immediate,
  type_idno      = 0xfc | tag_immediate
};

#define type_mcons type_pair
#define N_TYPES    256

// structure types for passing arguments through traverse
typedef struct {
  type_t  t;
  flags_t f;
  size_t  n, s;
  value_t i;
  void *spc;
} init_args_t;

typedef struct {
  value_t  *b;
  uchar_t **s;
  uchar_t **m;
  size_t   *u;
} relocate_args_t;

// global dispatch tables -----------------------------------------------------
// type dispatch tables
extern construct_t Construct[N_TYPES];
extern init_t      Init[N_TYPES];
extern relocate_t  Relocate[N_TYPES];
extern print_t     Print[N_TYPES];
extern finalize_t  Finalize[N_TYPES];
extern sizeof_t    Sizeof[N_TYPES];
extern mk_hash_t   Hash[N_TYPES];
extern order_t     Order[N_TYPES];

extern char_t  *TypeNames[N_TYPES];
extern size_t   TypeSizes[N_TYPES];
extern flags_t  TypeFlags[N_TYPES];            // default allocation flags
extern bool_t   TypeMembers[N_TYPES][N_TYPES]; // simplefied subtyping


#endif
