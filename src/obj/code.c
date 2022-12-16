#include <assert.h>

#include "rl/prin.h"
#include "rl/read.h"

#include "obj/code.h"
#include "obj/type.h"
#include "obj/cons.h"
#include "obj/nul.h"

#include "vm/memory.h"

#include "vm/obj/support/vector.h"
#include "vm/obj/support/namespc.h"

/* commentary */

/* C types */

/* internal functions */
void serialize_constants( vector_t constants, stream_t stream )
{
  fprintf(stream, ";;; Constants\n\n" );

  size_t length = vector_len(constants);
  value_t as_list = vec_to_list(constants, length);

  prin(stream, as_list);
  fprintf(stream, "\n\n");
}

void deserialize_constants( vector_t *constants, stream_t stream )
{
  value_t from_file = read(stream);

  save_values(1, &from_file);

  assert(is_cons(from_file) || is_nul(from_file));

  while (is_cons(from_file))
    {
      value_t head = get_cons_car(as_cons(from_file));
      vector_push(constants, head);
      from_file = get_cons_cdr(as_cons(from_file));
    }
}

void serialize_names( namespc_t *names, stream_t stream )
{
  fprintf(stream, ";;; Namespace\n\n");

  fprintf(stream, "(");

  while ( names )
    {
      fprintf(stream, "(");
      size_t n_locals = get_namespc_locals_count(names);
      value_t buffer[n_locals];
      
      namespc_names_to_buffer(names, buffer, n_locals);

      for (size_t i=0; i<n_locals; i++)
	{
	  prin(stream, buffer[i]);

	  if ( i+1 < n_locals )
	    fprintf(stream, " ");
	}

      fprintf(stream, ")");

      names = get_namespc_parent(names);

      if (names)
	fprintf(stream, " ");
    }

  fprintf(stream, ")\n\n");
}



/* globals */
void init_code(object_t *object);
void trace_code(object_t *object);
void free_code(object_t *object);

datatype_t CodeType =
  {
    {
      .obj=gl_datatype_head,
      .name="code",
      .isa=datatype_isa
    },

    .vmtype=vmtype_objptr,
    .obsize=sizeof(rl_code_t),

    .init=init_code,
    .trace=trace_code,
    .free=free_code
  };

/* API */
/* constructors */
rl_code_t *make_code( lambda_t *function )
{
  rl_code_t *out = (rl_code_t*)make_object(&CodeType);

  out->function = function;

  return out;
}

int serialize_code( rl_code_t *code, stream_t stream )
{
  
}

/* interfaces */

/* runtime dispatch */
void rl_obj_code_init( void ) {}
void rl_obj_code_mark( void ) {}
void rl_obj_code_cleanup( void ) {}

