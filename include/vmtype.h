#ifndef rl_vmtype_h
#define rl_vmtype_h

#include "common.h"

/* commentary

   Various definitions and globals that provide a low-level
   description of rascal fields & values.

   Also classifies rascal object types into a small number of
   template types for the purpose of memory management,
   safety, etc.
*/

/* C types */
typedef enum vmtype_t vmtype_t;
typedef enum numtype_t numtype_t;
typedef enum chartype_t chartype_t;
typedef enum objtype_t objtype_t;
typedef struct vmtype_spec_t vmtype_spec_t;

enum vmtype_t
  {
    vmtype_ascii,
    vmtype_latin1,
    vmtype_utf8,
    vmtype_utf16,
    vmtype_utf32,

    vmtype_sint8,
    vmtype_sint16,
    vmtype_sint32,
    vmtype_sint64,

    vmtype_uint8,
    vmtype_uint16,
    vmtype_uint32,
    vmtype_uint64,

    vmtype_flo32,
    vmtype_flo64,

    vmtype_value,

    vmtype_nulptr,
    vmtype_stream,
    vmtype_funptr,

    vmtype_symbol,
    vmtype_cvalue,
    vmtype_array,
    vmtype_string,
    vmtype_table,
    vmtype_record
  };

enum numtype_t
  {
    numtype_void,
    numtype_signed,
    numtype_unsigned,
    numtype_float
  };

enum chartype_t
  {
    chartype_void,
    chartype_ascii,
    chartype_latin1,
    chartype_utf8,
    chartype_utf16,
    chartype_utf32
  };

struct vmtype_spec_t
{
  char *name;
  size_t size;
  size_t align;
  numtype_t numtype;
  chartype_t chartype;
  ulong flags;
};

/* globals */
extern vmtype_spec_t TypeSpecs[];

/* API */

/* convenience */

#endif
