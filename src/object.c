#include <stddef.h>
#include <stdarg.h>

#include "object.h"
#include "memory.h"
#include "array.h"
#include "table.h"
#include "symbol.h"
#include "number.h"
#include "function.h"

// object
void init_object( object_t *object, data_type_t *data_type, flags_t flags )
{
  object->next    = tag_ptr( Heap.objects, OBJECT_TAG );
  Heap.objects    = object;

  object->data_type = data_type;
  object->gray      = true;
  object->black     = false;
  object->flags     = flags;
}

void free_object(object_t *object)
{
  if (object == NULL)
    return;

  data_type_t  *data_type = object->data_type;

  if (data_type->free)
    data_type->free(object);

  else
    dealloc(object, data_type->base_size);
}

void mark_objects(object_t **objects, size_t n)
{
  for (size_t i=0; i<n; i++)
    mark_object(objects[i]);
}

void mark_object(object_t *object)
{
  if (object == NULL)
    return;

  if (object->black)
    return;

  data_type_t *data_type = object->data_type;

  if (data_type->mark)
    obj_stack_push( &Heap.grays, object );

  else
    object->gray = false;
}

void trace_object(object_t *object)
{
  object->data_type->mark(object);
  object->gray = false;
}

// utility array types
OBJ_NEW(obj_alist);

ARRAY_INIT(obj_alist, object_t*, ALIST_MINC);
ARRAY_RESIZE(obj_alist, object_t*, pad_alist_size, ALIST_MINC);
ARRAY_CLEAR(obj_alist, object_t*, ALIST_MINC);
ARRAY_WRITE(obj_alist, object_t*);
ARRAY_APPEND(obj_alist, object_t*);
ARRAY_POP(obj_alist, object_t*);
ARRAY_PUSH(obj_alist, object_t*);
ARRAY_MARK(obj_alist, object_t*);
ARRAY_FREE(obj_alist, object_t*);

OBJ_NEW(obj_stack);

ARRAY_INIT(obj_stack, object_t*, STACK_MINC);
ARRAY_RESIZE(obj_stack, object_t*, pad_stack_size, STACK_MINC);
ARRAY_CLEAR(obj_stack, object_t*, STACK_MINC);
ARRAY_WRITE(obj_stack, object_t*);
ARRAY_APPEND(obj_stack, object_t*);
ARRAY_POP(obj_stack, object_t*);
ARRAY_PUSH(obj_stack, object_t*);
ARRAY_MARK(obj_stack, object_t*);
ARRAY_FREE(obj_stack, object_t*);

// data and union type implementations
OBJ_NEW(data_type);
OBJ_NEW(union_type);

void mark_type(type_t *type)
{
  mark_object((object_t*)type_constructor(type));
  mark_object((object_t*)type_name(type));
}

void mark_data_type(object_t *object)
{
  mark_type((type_t*)object);
  mark_object((object_t*)type_slots(object));
}

void mark_union_type(object_t *object)
{
  mark_type((type_t*)object);
  mark_object((object_t*)type_members(object));
}

void init_type( type_t *type, type_t *selfType, value_t signature, hash_t hash, symbol_t *name, function_t *constructor, bool hashed)
{
  init_object( &type->object, (data_type_t*)selfType, 0 );

  type_signature(type)   = signature;
  type_hash(type)        = hash;
  type_name(type)        = name;
  type_constructor(type) = constructor;
  type_hashed(type)      = hashed;
}

void init_data_type(data_type_t *type, symbol_t *name, function_t *constructor, size_t base_size, data_type_t *eltype, bool fit_imm, slots_init_t *spec, mark_fn_t mark, free_fn_t free, bool inlined)
{
  bool     no_trace, fits_imm, fits_word, hashed;
  value_t  signature;
  hash_t   hash = 0;
  slots_t *slots;

  if (spec == NULL)
    {
      Ctype_t Ctype = base_size&15ul;
      no_trace      = true;
      base_size     = sizeof_Ctype(Ctype);
      fits_word     = base_size <= 8;
      fits_imm      = sizeof_Ctype(Ctype) <= 4 || fit_imm;
      signature     = tag_int(Ctype, CTYPE);
      hash          = hash_int(base_size);
      hashed        = true;
      slots         = NULL;
      
    }

  else
    {
      no_trace         = true;
      fits_imm         = false;
      fits_word        = false;
      hash             = 0;
      hashed           = false;
      slots            = new_slots();
      signature        = tag_ptr(slots, OBJECT_TAG);

      for (size_t count=0; spec->n_slots; count++)
	{
	  slot_init_t init = spec->slot_specs[count];

	  symbol_t *name = symbol(init.name);

	  slot_t *buf;

	  assert( slots_put(slots, name, &buf) );
	  init_slot( buf, name, init.offset, init.type, init.unboxed, init.read, init.write, init.trace );
	  no_trace = no_trace && init.trace;
	  
	}
    }

  init_type((type_t*)type, DataType, signature, hash, name, constructor, hashed );

  type_no_trace(type)  = no_trace;
  type_inlined(type)   = inlined;
  type_slots(type)     = slots;
  type_base_size(type) = base_size;
  type_eltype(type)    = eltype;
  type_fits_imm(type)  = fits_imm;
  type_fits_word(type) = fits_word;
  type_mark(type)      = mark;
  type_free(type)      = free;
}

// initialization
data_type_t   DataTypeObject, UnionTypeObject;
union_type_t  AnyTypeObject, NoneTypeObject, TypeObject;

void object_types_init( void )
{
  DataType  = (type_t*)&DataTypeObject;
  UnionType = (type_t*)&UnionTypeObject;
  AnyType   = (type_t*)&AnyTypeObject;
  NoneType  = (type_t*)&NoneTypeObject;
}

void object_types_objects_init( void )
{
  DATA_TYPE_SLOTS( dtype_slots,
		   13,
		   { offsetof(object_t, next), ":next", AnyType, false, false, false, false },
		   { offsetof(object_t, data_type), ":type", DataType, true, true, false, true },
		   { offsetof(type_t, signature), ":signature", AnyType, false, true, false, false },
		   { offsetof(type_t, hash), ":hash", Int64Type, true, true, false, false },
		   { offsetof(type_t, name), ":name", SymbolType, true, true, false, true },
		   { offsetof(type_t, constructor), ":constructor", FunctionType, true, true, false, true },
		   { offsetof(data_type_t, slots), ":slots", SlotsType, true, true, false, true },
		   { offsetof(data_type_t, base_size), ":base-size", Int64Type, true, true, false, false },
		   { offsetof(data_type_t, eltype), ":element-type", Type, true, true, false, false },
		   { offsetof(data_type_t, fits_imm), ":fits-immediate?", BoolType, true, true, false, false },
		   { offsetof(data_type_t, fits_word), ":fits-word?", BoolType, true, true, false, false },
		   { offsetof(data_type_t, mark), ":mark", PointerType, true, true, false, false },
		   { offsetof(data_type_t, free), ":free", PointerType, true, true, false, false } );

  init_data_type(&DataTypeObject,
		 symbol("data-type" ),
		 NULL,
		 sizeof(data_type_t),
		 (data_type_t*)NoneType,
		 false,
		 &dtype_slots,
		 mark_data_type,
		 NULL,
		 false );

  DATA_TYPE_SLOTS( utype_slots,
		   7,
		   { offsetof(object_t, next), ":next", AnyType, false, false, false, false },
		   { offsetof(object_t, data_type), ":type", DataType, true, true, false, true },
		   { offsetof(type_t, signature), ":signature", AnyType, false, true, false, false },
		   { offsetof(type_t, hash), ":hash", Int64Type, true, true, false, false },
		   { offsetof(type_t, name), ":name", SymbolType, true, true, false, true },
		   { offsetof(type_t, constructor), ":constructor", FunctionType, true, true, false, true },
		   { offsetof(union_type_t, members), ":members", SetType, true, true, false, true } );

  init_data_type(&UnionTypeObject,
		 symbol("union-type"),
		 NULL,
		 sizeof(union_type_t),
		 (data_type_t*)NoneType,
		 false,
		 &utype_slots,
		 mark_union_type,
		 NULL,
		 false );

  init_union_type(&AnyTypeObject, symbol("any"), NULL, QNAN|((value_t)ANY<<40), NULL );
  init_union_type(&NoneTypeObject, symbol("none"), NULL, QNAN|((value_t)NONE<<40), NULL );

  type_t *type_members[] = { DataType, UnionType };
  
  init_union_type(&TypeObject,
		  symbol("type"),
		  NULL,
		  2,
		  type_members );
}
