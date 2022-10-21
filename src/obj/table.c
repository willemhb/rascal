#include "obj/table.h"
#include "obj/cvalue.h"
#include "vm/memory.h"
#include "utils/hashing.h"

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
  size_t oldc      = tb_cap( obj );
  size_t newc      = table_pad( obj, newl );
  tb_len( obj )    = newl;
  tb_cap( obj )    = newc;

  if ( newc != oldc )
    {
      object_t **news = alloc( newc * sizeof(object_t*) );
      object_t **olds = rehash_table( obj, news, newc );
      tb_data( obj )  = news;
      dealloc( olds, oldc * sizeof(object_t*) );
    }
}

static inline object_t *table_search( object_t *obj, rl_value_t key, hash_t *hb, size_t *jb )
{
  object_t **entries   = tb_data( obj );
  getter_t   getkey    = tb_getkey( obj );
  compare_t  keycmp    = tb_keycmp( obj );
  hash_t     h         = *hb = tb_keyhash( obj )( key );
  size_t     m         = tb_cap( obj );
  object_t  *entry     = NULL;

  *jb = h&m;

  while ((entry=entries[*jb]))
    {
      if (keycmp( key, getkey( entry ) ) == 0)
	break;
      *jb = (*jb+1) & m;
    }
  return entry;
}

bool table_assc( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  hash_t h; size_t j;
  object_t *entry = table_search( obj, key, &h, &j );

  if (buf)
    *buf = (rl_value_t)entry;

  return entry != NULL;
}

bool table_intern( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  hash_t h; size_t j;
  object_t *entry = table_search( obj, key, &h, &j );
  bool out        = entry == NULL;

  if (out)
    {
      entry = tb_intern( obj )( obj, key, h, j );
      resize_table( obj, tb_len( obj )+1 );
    }

  if (buf)
    *buf = (rl_value_t)entry;

  return out;
}

bool table_popi( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  hash_t h; size_t j;
  object_t *entry = table_search( obj, key, &h, &j );
  bool out        = entry != NULL;

  if ( out )
    {
      tb_data( obj )[j] = NULL;
      resize_table( obj, tb_len( obj ) - 1 );
    }

  if ( buf )
    *buf = (rl_value_t)entry;

  return out;
}

bool map_get( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  bool out = table_assc( obj, buf, key );

  if (out && buf)
    *buf = tb_getvalue( obj )( buf->as_object );

  return out;
}

bool map_add( object_t *obj, rl_value_t *buf, rl_value_t key, rl_value_t val )
{
  rl_value_t tmp;

  bool out = table_assc( obj, &tmp, key );

  tb_setvalue( obj )( tmp.as_object, val );

  if (buf)
    *buf = tmp;

  return out;
}

bool map_put( object_t *obj, rl_value_t *buf, rl_value_t key, rl_value_t val )
{
  rl_value_t tmp;

  bool out = table_assc( obj, &tmp, key );

  if (out)
    val = tb_setvalue( obj )( buf->as_object, val );

  else
    val = tb_getvalue( obj )( buf->as_object );

  if (buf)
    *buf = val;

  return out;
}

bool map_popk( object_t *obj, rl_value_t *buf, rl_value_t key )
{
  rl_value_t tmp;
  bool out = table_assc( obj, &tmp, key );

  if (out && buf)
    *buf = tb_getkey( obj )( tmp.as_object );

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

bool set_sup( object_t *self, object_t *other )
{
  if ( tb_len( self ) <= tb_len( other ) )
    return false;

  getter_t   otherk = tb_getkey( other );
  object_t **others = tb_data( other );
  size_t     otherl = tb_len( other );
  size_t     otherc = tb_cap( other );

  for (size_t c=0, l=0; c < otherc && l<otherl; c++)
    {
      if (others[c] == NULL)
	continue;

      l++;

      if ( set_has( self, otherk( others[c] ) ) )
	continue;
      return false;
    }

  return true;
}

bool set_sub( object_t *self, object_t *other )
{
  return set_sup( other, self );
}

bool set_rmv( object_t *obj, rl_value_t key )
{
  return table_popi( obj, NULL, key );
}

void init_map( object_t *obj, type_t *type, size_t n, void *data )
{
  (void)type;
  
  tb_len( obj )  = 0;
  tb_cap( obj )  = table_pad( obj, n );
  tb_data( obj ) = alloc( n * sizeof( object_t* ) );

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

  tb_len( obj )  = 0;
  tb_cap( obj )  = table_pad( obj, n );
  tb_data( obj ) = alloc( n * sizeof( object_t* ) );

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

// general implementation functions
size_t id_keysize( rl_value_t key )
{
  (void)key;
  return 0;
}

rl_value_t id_getkey( object_t *obj )
{
  return (rl_value_t)obj;
}

rl_value_t id_getval( object_t *obj )
{
  return (rl_value_t)obj;
}

hash_t id_gethash( rl_value_t key )
{
  return hash_long( key.as_value );
}

int id_keycmp( rl_value_t x, rl_value_t y )
{
  return x.as_fixnum != y.as_fixnum;
}

object_t *id_intern( object_t *obj, rl_value_t k, hash_t h, size_t j )
{
  (void)h;
  tb_data( obj )[j] = k.as_object;
  return k.as_object;
}

object_t *etype_intern( object_t *obj, rl_value_t k, hash_t h, size_t j )
{
  set_init_t ini = { .key=k, .hash=h };
  object_t *out = tb_data( obj )[j] = new_obj( tb_etype( obj ), tb_keysize( obj )( k ), &ini );
  return out;
}

// globals
type_t *TableImplSigData[] =
  {
    &UInt64Type,  &DataType,    &PointerType, &PointerType, &PointerType,
    &PointerType, &PointerType, &PointerType, &PointerType, &PointerType
  };

signature_t TableImplSig =
  {
    { .dtype=&DataType, .permanent=true },

    .length=10,
    .capacity=10,
    .data = TableImplSigData
  };

layout8_t TableImplLayout =
  {
    { .base_size=sizeof( tb_impl_t ), .n_fields=10 },
    {
      { sizeof(size_t),    NTUINT, false, offsetof( tb_impl_t, minc )       },
      { sizeof(type_t*),   NTPTR,  false, offsetof( tb_impl_t, entry_type ) },
      { sizeof(pad_t),     NTPTR,  false, offsetof( tb_impl_t, pad )        },
      { sizeof(sizefn_t),  NTPTR,  false, offsetof( tb_impl_t, keysize )    },
      { sizeof(hashfn_t),  NTPTR,  false, offsetof( tb_impl_t, keyhash )    },
      { sizeof(compare_t), NTPTR,  false, offsetof( tb_impl_t, keycmp )     },
      { sizeof(getter_t),  NTPTR,  false, offsetof( tb_impl_t, getkey )     },
      { sizeof(getter_t),  NTPTR,  false, offsetof( tb_impl_t, getvalue )   },
      { sizeof(setter_t),  NTPTR,  false, offsetof( tb_impl_t, setvalue )   },
      { sizeof(intern_t),  NTPTR,  false, offsetof( tb_impl_t, intern )     },
    }
  };

dtype_impl_t TableImplImpl =
  {
    .value=Ctype_object,
    .repr=OBJ,
    .data_size=sizeof(object_t*),
    .layout=&TableImplLayout.layout,
  };

type_t TableImplType =
  {
    .obj = { .dtype=&DataType, .permanent=true },
    
    .dtype=&TableImplImpl
  };

layout8_t TableLayout =
  {
    { .base_size=sizeof(table_t), .n_fields=3 },
    {
      { sizeof(size_t), NTUINT, false, offsetof( table_t, length )   },
      { sizeof(size_t), NTUINT, false, offsetof( table_t, capacity ) },
      { sizeof(size_t), NTPTR,  false, offsetof( table_t, data )     }
    }
  };

void rl_obj_table_init( void )
{
  init_prim_dtype( &TableImplType, "table-impl", NOCONS, 0 );
}
