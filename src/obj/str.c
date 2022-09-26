#include <string.h>
#include <stdio.h>

#include "obj/str.h"
#include "obj/port.h"

#include "mem.h"

// globals
str_t *empty_str, empty_str_obj;

// object api]
PAD_TEXT_SIZE(str)
NEW_STRING(str, char_t)
INIT_STRING(str, char_t)
FINALIZE_STRING(str, char_t)

// api implementations
int ord_str( val_t x, val_t y )
{
  return strcmp( as_str(x)->data, as_str(y)->data );
}

hash_t hash_str( val_t x )
{
  str_t *str = as_str(x);

  if (!flag_p(str->object.flags, str_fl_hashed))
    {
      hash_t h  = hash_string( str->data );
      str->hash = mix_hashes( str_type, h );
      str->object.flags |= str_fl_hashed;
    }

  return str->hash;
}

arity_t prin_str( obj_t *io, val_t val )
{
  str_t  *str    = as_str(val);
  FILE   *stream = ((port_t*)io)->stream;
  arity_t out    = fprintf( stream, "\"%s\"", str->data );

  return out;
}

// initialization
void str_init( void )
{
  // initialize empty string object
  empty_str = &empty_str_obj;

  // the magic number is the initial value for the FNV64 algorithm
  hash_t empty_str_hash = mix_hashes( str_type, 0xcbf29ce484222325ul );

  empty_str_obj = (str_t)
    {
      {
	.next  = NULL,
	.type  = str_type,
	.flags = mem_fl_static | mem_fl_protected | str_fl_hashed | enc_ascii,
	.black = true,
	.gray  = false
      },

      .len  = 0,
      .data = "",
      .hash = empty_str_hash
    };

  // initialize API table
  val_api_t *StrApi = &ValApis[str_type];

  StrApi->prin = prin_str;
  StrApi->hash = hash_str;
  StrApi->ord  = ord_str;
}
