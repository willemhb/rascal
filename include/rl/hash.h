#ifndef rascal_rl_hash_h
#define rascal_rl_hash_h

#include "obj/hash.h"
#include "describe.h"

/* Implementation of the general value hash used in rascal.

   Because hashing is required for efficient multiple dispatch, 
   a builtin hashing function that works for arbitrary rascal values
   is required. Reference cycles and the complexity of the type system
   make this a non-trivial task, hence it goes in its own module.

   I'm not yet sure how to handle potential user extension of `(hash x)`.
   For now I'm simply marking the function as final, but I would like to find
   a way around that.
 */

// external api
hash_t hash_obj( object_t *obj );
hash_t hash_val( value_t val );

// convenience
#define rl_hash ( x ) GENERIC_2( hash, x )

#endif
