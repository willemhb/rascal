#include <assert.h>
#include <string.h>

#include "repr.h"

#include "obj/object.h"
#include "obj/type.h"
#include "obj/stream.h"

#include "vm/memory.h"

#include "util/string.h"

/* commentary */

/* C types */

/* globals */

/* API */
object_t *make_object( type_t *type, size_t n )
{
  if ( type->make_fn )
    return type->make_fn(type, n);

  object_t *out = alloc(type->ob_size);

  *out = (object_t) OBJINIT(type, type->ob_size, true);

  return out;
}

void init_object( object_t *object, size_t n, void *ini )
{
  if ( object->type->init_fn )
    object->type->init_fn(object, n, ini);

  else
    {
      type_t  *type   = object->type;

      assert( n == type->n_fields );

      if ( ini )
	memcpy(object->space, ini, type->ob_size - sizeof(object_t));
    }
}

void free_object( object_t *object )
{
  if ( !object->allocated )
    return;

  type_t *type      = object->type;

  if ( type->free_fn )
    {
      type->free_fn(object);
      return;
    }

  field_t *fields   = type->fields;
  size_t   tmp_size = 0;
  void    *tmp_ptr  = NULL;

  for (size_t i=0; i<type->n_fields;i++)
    {
      field_t field = fields[i];
      
      switch ( field.repr )
	{
	case repr_sptr:
	  tmp_ptr   = *(void**)(object->space + field.offset);
	  
	  fclose(tmp_ptr);
	  break;

	case repr_vals:
	  tmp_ptr   = *(void**)(object->space + field.offset);
	  tmp_size  = *(size_t*)(object->space + field.offset + sizeof(value_t*));

	  dealloc_array(tmp_ptr, tmp_size, sizeof(value_t));
	  break;

	case repr_objs:
	  tmp_ptr  = *(void**)(object->space + field.offset);
	  tmp_size = *(size_t*)(object->space + field.offset + sizeof(object_t**));

	  dealloc_array(tmp_ptr, tmp_size, sizeof(object_t*));
	  break;

	case repr_astr:
	case repr_l1str:
	case repr_u8str:
	  tmp_ptr = *(void**)(object->space + field.offset);

	  if ( field.write )
	    tmp_size = *(size_t*)(object->space + field.offset + sizeof(char*));

	  else
	    tmp_size = strlen8(tmp_ptr)+1;

	  dealloc_array(tmp_ptr, tmp_size, sizeof(char));
	  break;

	case repr_u16str:
	  tmp_ptr = *(void**)(object->space + field.offset);

	  if ( field.write )
	    tmp_size = *(size_t*)(object->space + field.offset + sizeof(char16_t*));

	  else
	    tmp_size = strlen16(tmp_ptr)+1;

	  dealloc_array(tmp_ptr, tmp_size, sizeof(char16_t));
	  break;

	case repr_u32str:
	  tmp_ptr = *(void**)(object->space + field.offset);

	  if ( field.write )
	    tmp_size = *(size_t*)(object->space + field.offset + sizeof(char32_t*));

	  else
	    tmp_size = strlen32(tmp_ptr)+1;

	  dealloc_array(tmp_ptr, tmp_size, sizeof(char32_t));
	  break;

	case repr_bptr:
	  tmp_ptr  = *(void**)(object->space + field.offset);
	  tmp_size = *(size_t*)(object->space + field.offset + sizeof(uchar*));

	  dealloc_array(tmp_ptr, tmp_size, type->el_size);
	  break;

	default:
	  break;
	}
    }

  dealloc(object, type->ob_size);
}

extern void mark_value( value_t x );
extern void mark_object( object_t *object );

void trace_object( object_t *object )
{
  if ( object->type->trace_fn )
    {
      object->type->trace_fn(object);
      return;
    }

  object->gray    = false;
  field_t *fields = object->type->fields;

  for ( size_t i=0; i < object->type->n_fields; i++ )
    {
      field_t field = fields[i];

      switch ( field.repr )
	{
	case repr_val:
	  {
	    value_t val = *(value_t*)(object->space + field.offset);

	    mark_value(val);
	    break;
	  }

	case repr_vals:
	  {
	    value_t *vals  = *(value_t**)(object->space + field.offset);
	    size_t   count = *(size_t*)(object->space + field.offset + sizeof(value_t*));

	    for ( size_t j=0; j<count; j++ )
	      mark_value(vals[j]);
	    break;
	  }

	case repr_obj:
	  {
	    object_t *obj   = *(object_t**)(object->space + field.offset);

	    mark_object(obj);
	    break;
	  }

	case repr_objs:
	  {
	    object_t **objs  = *(object_t***)(object->space + field.offset);
	    size_t     count = *(size_t*)(object->space + field.offset + sizeof(object_t**));

	    for ( size_t j=0; j<count; j++ )
	      mark_object(objs[j]);
	    break;
	  }

	default:
	  break;
	}
    }
}

/* runtime */
void rl_obj_object_init( void ) {}
void rl_obj_object_mark( void ) {}
