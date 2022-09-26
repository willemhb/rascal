#include <wchar.h>
#include "obj/multi.h"


bool get_method(multi_t *multi, val_t *args, arity_t n, obj_t **buf)
{
  type_t buffer[n];
  hash_t hashes[n];
  wmemset((int*)buffer, 0, n);
  wmemset((int*)hashes, 0, n*2);
  
  obj_t *obj = (obj_t*)multi;

  while (obj->type == multi_type)
    {
      objs_t *objs= &multi->table;
      arity_t  loc = multi->position;
      
      if (buffer[loc] == 0)
	{
	  buffer[loc] = typeof_val(args[loc]);
	  hashes[loc] = hash_val(buffer[loc]);
	}

      hash_t  h = hashes[loc];
      arity_t i = h & (objs->cap-1);
      
    }
}
