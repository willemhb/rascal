#include "obj/table.h"
#include "vm/memory.h"

// general table utilities
static const double table_resize_pressure = 0.75;

size_t pad_table_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;

  if (newl > oldl)			
    while (newl > newc*table_resize_pressure)
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < table_resize_pressure*(newc >> 1))
      newc >>= 1;
  if (newc < minc)
    newc = minc;
  return newc;
}

void trace_table( object_t *obj )
{
  table_t *table =(table_t*)obj;
  mark_objs(table->data, table->capacity );
}

void free_table( object_t *obj )
{
  table_t *table = (table_t*)obj;

  dealloc( table->data, table->capacity * sizeof( object_t* ) );
}

object_t **rehash_table( object_t *obj, object_t **news, size_t newc )
{
  table_t*   table = (table_t*)obj;
  object_t** olds  = table->data;
  size_t     oldc  = table->capacity;

  for (size_t i=0; i<oldc; i++)
    {
      if (olds[i] == NULL)
	continue;

      hash_t h = ((entry_t*)olds[i])->hash;
      size_t m = newc-1;
      size_t j = h&m;

      while (news[j]) j = (j+1) & m;

      news[j] = olds[i];
    }

  return olds;
}

void resize_table( object_t *obj, size_t newl )
{
  table_t   *table = (table_t*)obj;
  tb_impl_t *impl  = table_impl( obj );
  size_t oldl      = table->length;
  size_t oldc      = table->capacity;
  size_t newc      = impl->padlength( oldl, newl, oldc, impl->minc );
  table->length    = newl;
  table->capacity  = newc;

  if ( newc != oldc )
    {
      object_t **news = alloc( newc * sizeof(object_t*) );
      object_t **olds = rehash_table( obj, news, newc );
      table->data     = news;
      dealloc( olds, oldc * sizeof(object_t*) );
    }
}

static inline entry_t *table_search( table_t *table, rl_value_t key, hash_t *hb, size_t *jb )
{
    entry_t  **entries = (entry_t**)table->data;
    tb_impl_t *impl    = table_impl( (object_t*)table );
    hash_t     h       = *hb = impl->hashkey( key );
    size_t     m       = table->capacity-1;
    entry_t   *entry   = NULL;

    *jb = h&m;

    while ((entry=entries[*jb]))
      {
	if (impl->cmpkeys( key, entry->key ) == 0)
	  break;
	*jb = (*jb+1) & m;
      }
    return entry;
}

bool table_assc( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  table_t *table = (table_t*)obj; hash_t h; size_t j;
  entry_t *entry = table_search( table, key, &h, &j );

  if (buf)
    *buf = (rl_value_t)&entry->obj;

  return entry != NULL;
}

bool table_intern( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  table_t *table = (table_t*)obj; hash_t h; size_t j;
  entry_t *entry = table_search( table, key, &h, &j );  
  bool out       = entry == NULL;

  if (out)
    {
      set_init_t ini = { .key = key, .hash = h };
      entry          = (entry_t*)new_obj( table_entry_type( obj ),
					  table_impl( obj )->keysize( key ),
					  &ini );
      resize_table( obj, table->length+1 );
    }

  if (buf)
    *buf = (rl_value_t)&entry->obj;
  return out;
}

bool table_popi( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  table_t *table = (table_t*)obj; hash_t h; size_t j;
  entry_t *entry = table_search( table, key, &h, &j );  
  bool out       = entry != NULL;

  if (out)
    {
      table->data[j] = NULL;
      resize_table( obj, table->length - 1);
    }

  if (buf)
    *buf = (rl_value_t)&entry->obj;

  return out;
}

bool map_get( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  bool out = table_assc( obj, buf, key );

  if (out && buf)
    *buf = table_impl( obj )->getvalue( buf->as_object );

  return out;
}

bool map_add( object_t *obj, rl_value_t *buf, rl_value_t key, rl_value_t val )
{
  rl_value_t tmp;

  bool out = table_assc( obj, &tmp, key );

  table_impl( obj )->setvalue( tmp.as_object, val );

  if (buf)
    *buf = tmp;

  return out;
}

bool map_put( object_t *obj, rl_value_t *buf, rl_value_t key, rl_value_t val )
{
  rl_value_t tmp;

  bool out = table_assc( obj, &tmp, key );

  if (out)
      val = table_set_value( obj, buf->as_object, val );

  else
    val = table_get_value( obj, buf->as_object );

  if (buf)
    *buf = val;

  return out;
}

bool map_popk( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  rl_value_t tmp;
  bool out = table_assc( obj, &tmp, key );

  if (out && buf)
    *buf = table_get_key( obj, tmp.as_object );

  return out;
}

bool map_popi( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  rl_value_t tmp;
  bool out = table_assc( obj, &tmp, key );

  if (out && buf)
    *buf = tmp;

  return out;
}

bool set_has( object_t *obj, rl_value_t key )
{
  return table_assc( obj, NULL, key );
}

bool set_add( object_t *obj, rl_value_t key )
{
  return table_intern( obj, NULL, key );
}

bool set_rmv( object_t *obj, rl_value_t key )
{
  return table_popi( obj, NULL, key );
}

void init_map( object_t *obj, type_t *type, size_t n, void *data )
{
  (void)type;
  
  table_t* table  = (table_t*)obj;

  table->length   = 0;
  table->capacity = table_pad_length( obj, n );
  table->data     = alloc( n * sizeof( object_t* ) );

  if ( data )
    {
      map_init_t *inis = data;
      for (size_t i=0; i<n;i++)
	{
	  map_init_t ini = inis[i];
	  map_add( obj, NULL, ini.key, ini.val );
	}
    }
}

void init_set( object_t *obj, type_t *type, size_t n, void *data )
{
  (void)type;

  table_t* table  = (table_t*)obj;

  table->length   = 0;
  table->capacity = table_pad_length( obj, n );
  table->data     = alloc( n * sizeof( object_t* ) );

  if ( data )
    {
      map_init_t *inis = data;
      for (size_t i=0; i<n;i++)
	{
	  map_init_t ini = inis[i];
	  map_add( obj, NULL, ini.key, ini.val );
	}
    }
}

// globals
layout8_t TableImplLayout =
  {
    { .base_size=sizeof(tb_impl_t), .n_fields=8 },
    {
      { sizeof(size_t),   NTUINT, false, offsetof(tb_impl_t, minc) },
      { sizeof(padfn_t),  NTPTR,  false, offsetof(tb_impl_t, padlength) },
      { sizeof(sizeof_t), NTPTR,  false, offsetof(tb_impl_t, keysize)   },
      { sizeof(hashfn_t), NTPTR,  false, offsetof(tb_impl_t, hashkey)   },
      { sizeof(cmpfn_t),  NTPTR,  false, offsetof(tb_impl_t, cmpkeys)   },
      { sizeof(getter_t), NTPTR,  false, offsetof(tb_impl_t, getkey)    },
      { sizeof(getter_t), NTPTR,  false, offsetof(tb_impl_t, getvalue)  },
      { sizeof(setter_t), NTPTR,  false, offsetof(tb_impl_t, setvalue)  },
    }
  };

dtype_impl_t TableImplImpl =
  {
    .value=Ctype_object, .repr=OBJ,  .data_size=sizeof(object_t*),
    .layout=&TableImplLayout.layout,
  };

type_t TableImplType =
  {
    { .dtype=&DataType },
    
    .dtype=&TableImplImpl
  };

layout8_t TableLayout =
  {
    { .base_size=sizeof(table_t), .n_fields=4 },
    {
      { sizeof(size_t), NTUINT, false, offsetof(table_t, length) },
      { sizeof(size_t), NTUINT, false, offsetof(table_t, capacity) },
      { sizeof(size_t), NTPTR,  false, offsetof(table_t, data) }
    }
  };

void rl_obj_table_init( void )
{
  init_dtype( &TableImplType, "table-impl", NOCONS, 0 );
}
