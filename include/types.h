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
	       lowest part of immediate tag used for abstract types
	       (not constructable and no concrete instances).
	     */
	     
	     type_none      = 0x00 | tag_immediate,
	     type_any       = 0x04 | tag_immediate,
	     
	     /* non-literal types */
	     type_symbol    = 0x00 | tag_header,
	     type_cons      = 0x04 | tag_header,
	     
	     /* sequence & collection types */
	     /* array types */
	     type_vector    = 0x08 | tag_header,
	     type_string    = 0x0c | tag_header,
	     type_bytecode  = 0x10 | tag_header,
	     
	     /* mapping types */
	     type_table     = 0x14 | tag_header,
	     
	     /* function object types types */
	     type_closure   = 0x18 | tag_header,
	     
	     /* misc internal types */
	     type_port      = 0x1c | tag_header,
	     type_error     = 0x20 | tag_header,

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
};

#define type_mcons type_pair
#define N_TYPES    256

// global dispatch tables -----------------------------------------------------
extern char_t  *TypeNames[N_TYPES];
extern size_t   TypeSizes[N_TYPES];

#endif
