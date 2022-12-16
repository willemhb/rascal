#include "util/string.h"

#include "vm/error.h"
#include "vm/value.h"

#include "rl/load.h"



/* commentary */

/* C types */
typedef enum load_file_type_t
  {
    load_file_type_invalid=-1,
    load_file_type_source = 0,
    load_file_type_object = 1
  } load_file_type_t;

/* internal functions */
load_file_type_t load_file_type( const string_t fname );
value_t          load_from_source_file( const string_t fname );
value_t          load_from_object_file( const string_t fname );

load_file_type_t load_file_type( const string_t fname )
{
  if ( str_ends_with(fname, ".rl") )
    return load_file_type_source;

  if ( str_ends_with(fname, ".rl.o") )
    return load_file_type_object;

  return load_file_type_invalid;
}

value_t load_from_source_file( const string_t fname )
{
  
}

/* globals */

/* API */
value_t load( const string_t fname )
{
  load_file_type_t lf_type = load_file_type(fname);

  if ( lf_type == load_file_type_source )
    return load_from_source_file(fname);

  if ( lf_type == load_file_type_object )
    return load_from_object_file(fname);

  panic("Unkown file type for '%s'", fname);
  return NUL;
}

/* runtime dispatch */
void rl_rl_load_init( void ) {}
void rl_rl_load_mark( void ) {}
void rl_rl_load_cleanup( void ) {}
