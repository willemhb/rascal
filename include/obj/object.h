#ifndef rascal_object_h
#define rascal_object_h

#include <stdio.h>

#include "rascal.h"

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

/* API */
object_t *make_object( type_t *type );
void init_object( object_t *obj, size_t n, void *ini );
void free_object( object_t *obj );
void trace_object( object_t *obj );

/* convenience & utilities */
// common object head declaration
#define OBJHEAD object_t obj

// apply object tag
#define tag_object( x ) tag_ptr( x, OBJECT )

// initialize object head, fill in default values
#define OBJINIT(type, size, allocated)				\
  { NULL, NUL, (type), (size), true, false, (allocated), 0 }



#endif
