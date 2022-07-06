#ifndef rascal_types_h
#define rascal_types_h

#define tag_object    0x00
#define tag_immediate 0x01
#define tag_moved     0x02
#define tag_header    0x03

typedef enum {
  /*
    object tag used for singleton types
   */
  type_null      = 0x00 | tag_object,
  /*
    moved tag used for union types (constructable but no concrete objects with this type)
   */
  type_list      = 0x04 | tag_moved,

  /*
    lowest part of immediate tag used for abstract types
    (not constructable and no concrete instances).
  */

  type_none      = 0x00 | tag_immediate,
  type_any       = 0x04 | tag_immediate,

  /* non-literal types */
  type_symbol    = 0x04 | tag_header,
  type_pair      = 0x08 | tag_header,
  type_cons      = 0x0c | tag_header,

  /* literal types */
  type_table     = 0x10 | tag_header,
  type_string    = 0x14 | tag_header,
  type_binary    = 0x18 | tag_header,
  type_tuple     = 0x1c | tag_header,
  type_function  = 0x20 | tag_header,
  type_port      = 0x24 | tag_header,
  type_error     = 0x28 | tag_header,

  /* immediate types */
  type_boolean   = 0x08 | tag_immediate,
  type_character = 0x0c | tag_immediate,

  type_builtin   = 0x10 | tag_immediate,
  type_form      = 0x14 | tag_immediate,
  type_opcode    = 0x18 | tag_immediate,

  /* numeric types */
  type_integer   = 0x1c | tag_immediate,
  type_fixnum    = 0x20 | tag_immediate,

  /* higher order types */
  type_type      = 0xe0 | tag_immediate,
  type_class     = 0xe4 | tag_immediate,
  type_effect    = 0xe8 | tag_immediate,

  /* any user defined primitive that fits 32-bits */
  type_Cprim32   = 0xec | tag_immediate,

  /* internal types (these are fixnums with special tags to hint at their meaning) */

  type_arity     = 0xf0 | tag_immediate,
  type_size      = 0xf4 | tag_immediate,
  type_index     = 0xf8 | tag_immediate,
  type_idno      = 0xfc | tag_immediate
} type_t;

#define N_TYPES    256

/* type system basics
   - a type() is a set of values
   - the type system includes a bottom type, none()
   - the type system includes a unit type, null()
   - the type system includes a top type, any()

   - the type system includes four metatypes
     - type()   - the type of all metatypes
     - data()   - the type of all types of kind *
     - class()  - the type of all types of kind _ -> Constraint
     - effect() - the type of all types of kind e

   - the type system includes the following primitive types:
     - symbol(), binding(), envt(), module()
     - pair(), cons(), graph(), list()
     - tuple(), string(), binary(), array()
     - vector(), dict(), set(), table()
     - queue(), task(), port(), process(), error()
     - function(), form(), builtin(), macro(), opcode()
     - ascii(), latin1(), utf8(), utf16(), utf32()
     - int(), float(), frac(), imag(), cplx(), fixnum()
     
   - the type system includes the following primitive classes:
     - 
     
 */

#endif
