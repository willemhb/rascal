// standard headers -----------------------------------------------------------
#include <string.h>

// utility headers ------------------------------------------------------------
#include "numutils.h"
#include "memutils.h"

// rascal headers -------------------------------------------------------------
#include "array.h"
#include "memory.h"

// local utilities ------------------------------------------------------------
#define TABLE_PRESSURE 0.625
#define N_STACK 512

// misc utilities -------------------------------------------------------------
size_t Ctype_size(Ctype_t ct)
{
  static size_t dispatch[] =
    {
      [C_sint8]  = 1, [C_uint8]  = 1, [C_ascii]   = 1, [C_latin1]  = 1, [C_utf8] = 1,
      [C_sint16] = 2, [C_uint16] = 2, [C_utf16]   = 2,
      [C_sint32] = 4, [C_uint32] = 4, [C_utf32]   = 4, [C_float32] = 4,
      [C_sint64] = 8, [C_uint64] = 8, [C_float64] = 8
    };

  return dispatch[ct];
}

bool Ctype_is_encoded(Ctype_t ct)
{
  static bool dispatch[] =
    {
      [C_sint8]  = false, [C_uint8]  = false,
      [C_ascii]  = true,  [C_latin1] = true,  [C_utf8]    = true,
      [C_sint16] = false, [C_uint16] = false, [C_utf16]   = true,
      [C_sint32] = false, [C_uint32] = false, [C_utf32]   = true, [C_float32] = false,
      [C_sint64] = false, [C_uint64] = false, [C_float64] = false
    };

  return dispatch[ct];
}

// size helpers ---------------------------------------------------------------
size_t pad_table_length( size_t len, size_t cap )
{
  cap = ceil_log2(cap);

  if (len < (cap / 2) * TABLE_PRESSURE)
    return cap / 2;
  
  if (len < cap * TABLE_PRESSURE)
    return cap;

  return cap * 2;
}

size_t pad_stack_length( size_t new_len, size_t cap )
{
  if (new_len >= cap)
    return cap * 2;

  if (new_len < cap / 2 && cap > N_STACK) // never go below initial size
    return cap / 2;

  return cap;
}

size_t pad_array_length( size_t old_len, size_t new_len, size_t cap )
{
  /* Python overallocation algorithm */
  if (cap >= new_len && new_len >= cap / 2)
    return cap;

  size_t padded = (new_len + (new_len>>3) + 6) & ~(size_t)3;

  if (new_len - old_len > padded - new_len)
    padded = (new_len + 3) & ~(size_t)3;

  return padded;
}

size_t pad_string_length( size_t old_len, size_t new_len, size_t new )
{
  return pad_array_length( old_len, new_len, new ) + 1;
}

vector_t *resize_vector( vector_t *v, size_t new_len )
{
  size_t padded;
  
  if (v == NULL)
    {
      v = new_vector( new_len, &padded, false );
      init_vector( v, 0, new_len, padded, NULL );
    }

  else
    {
      padded = pad_array_length( vec_len(v), new_len, vec_cap(v) );

      if (padded != vec_cap(v))
	{
	  value_t buf = tagp(v, VECTOR);
	  preserve(1, &buf);
	  value_t *new_spc = allocate(padded*sizeof(value_t), is_global(v));
	  memcpy(vec_data(buf), new_spc, vec_len(v) * sizeof(value_t) );
	  vec_cap(buf) = padded;
	  v = pval(buf);
	}
    }

  return v;
}

binary_t *resize_binary( binary_t *b, size_t new_len )
{
  size_t padded;
  
  if (b == NULL)
    {
      b = new_binary( new_len, &padded, false );
      init_vector( b, 0, new_len, padded, NULL );
    }

  else
    {
      padded = pad_array_length( vec_len(v), new_len, vec_cap(v) );

      if (padded != vec_cap(v))
	{
	  value_t buf = tagp(v, VECTOR);
	  preserve(1, &buf);
	  value_t *new_spc = allocate(padded*sizeof(value_t), is_global(v));
	  memcpy(vec_data(buf), new_spc, vec_len(v) * sizeof(value_t) );
	  vec_cap(buf) = padded;
	  v = pval(buf);
	}
    }

  return v;
}



// constructors & initializers ------------------------------------------------
vector_t *new_vector( size_t n_args, size_t *padded, bool gl )
{
  if (n_args == 0)
    return NULL;

  size_t n_alloc = n_args;

  if (padded)
    {
      n_alloc = pad_array_length( 0, n_args, 0 );
      *padded = n_alloc;
    }
  
  size_t alloc_size = sizeof(vector_t) + n_alloc * sizeof(value_t);

  return allocate( alloc_size, gl );
}

void init_vector( vector_t *v, uint fl, size_t n, size_t p, value_t *a )
{
  init_object( v, sizeof(object_t), tag_vector, fl );
  vec_len(v)  = n;
  vec_cap(v)  = p;

  if (is_global(v))
      vec_data(v) = calloc_s( p, sizeof(value_t) );

  else
    vec_data(v) = (value_t*)(v+1);

  if (a)
    memcpy( vec_data(v), a, n*sizeof(value_t));

  else
    memset( vec_data(v), 0, n*sizeof(value_t));
}



// accessors ------------------------------------------------------------------
