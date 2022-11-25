#ifndef rl_obj_object_h
#define rl_obj_object_h

#include "vm/value.h"

/* commentary 

   Definition of the common object type.

   All values tagged OBJECT have a struct object_t as their first member.

   struct object_t contains:

     - pointer to type
     - live object pointer (might be null)
     - metadata list
     - size of total memory belonging to object
     - memory flags
     - discretionary flags
 */

/* C types */
struct object_t
{
  object_t *next;           // live object list
  value_t   _meta;          // association list of object metadata
  type_t   *type;           // this object's type
  uint      size;           // object's total size
  uchar     gray;
  uchar     black;
  uchar     allocated;
  uchar     flags;
  uchar     space[0];     // beginning of object's own data
};

/* globals */

/* API */
object_t *make_object( type_t *type, size_t n );
void      init_object( object_t *object, size_t n, bool safe, void *ini );
void      free_object( object_t *object );
void      trace_object( object_t *object );

/* runtime */
void rl_obj_object_init( void );
void rl_obj_object_mark( void );

/* convenience */
// common object head declaration
#define OBJHEAD object_t obj

// apply object tag
#define tag_object( x ) set_tag( x, OBJECT )

static inline object_t *as_object( value_t x ) { return (object_t*)as_pointer(x); }
static inline bool      is_object( value_t x ) { return get_tag(x) == OBJECT; }

// initialize object head, fill in default values
#define OBJINIT(type, size, allocated)				\
  { NULL, NUL, (type), (size), true, false, (allocated), 0 }



#endif
