#include <stdlib.h>
#include <string.h>

#include "obj/table.h"
#include "obj/cons.h"
#include "obj/atom.h"
#include "obj/str.h"

#include "mem.h"

// parameters
static const arity_t table_min_cap     = 8;
static const real_t  table_load_factor = 0.75;

// forward declarations
int     symt_ord(val_t x, val_t y);
hash_t  symt_hash(val_t x);
obj_t  *symt_intern(table_t *table, val_t key, hash_t hash);
obj_t  *table_intern(table_t *table, val_t key, hash_t hash);
hash_t  envt_hash(val_t x);
int     envt_ord(val_t x, val_t y);
obj_t  *envt_intern(table_t *table, val_t key, hash_t hash);

// static helpers
size_t pad_table_size(size_t oldl, size_t newl, size_t oldc)
{
  size_t newc = max(oldc, table_min_cap);

  if (newl < newc * table_load_factor
      && newl > (newc>>1) * table_load_factor)
    return newc;

  if (oldl > newl)
    {
      while (newc > table_min_cap
	     && newl < newc / 2 * table_load_factor)
	newl >>= 1;
    } 
  else if (newl > oldl)
    {
      while (newl > newc * table_load_factor)
	newc <<= 1;
    }
  return newc;
}

// main
table_t *new_table(arity_t n, flags_t fl)
{
  table_t *out = alloc( sizeof(table_t) );
  init_table( out, n, fl );
  return out;
}

void init_table(table_t *table, arity_t n, flags_t fl)
{
  arity_t cap = pad_table_size(0, n, 0);

  init_obj( (obj_t*)table, table_type, fl );
  init_objs( &table->entries, n );

  table->locations = allocv( cap, arity_t );

  if (flag_p(fl, table_fl_symt))
    {
      table->ord    = symt_ord;
      table->hash   = symt_hash;
      table->intern = symt_intern;
    }

  else if (flag_p(fl, table_fl_envt))
    {
      table->ord    = envt_ord;
      table->hash   = envt_hash;
      table->intern = envt_intern;
    }

  else
    {
      table->ord    = ord_vals;
      table->hash   = hash_val;
      table->intern = table_intern;
    }
}

void finalize_table(table_t *table)
{
  deallocv(table->locations, table_cap(table), arity_t );
  finalize_objs(&table->entries);
}

void trace_table(obj_t *obj)
{
  table_t *table = (table_t*)obj;
  trace( table_entries(table), table_cap(table) );
}




void resize_table(table_t *table, arity_t newn)
{
  arity_t oldn = table_len(table), oldc = table_cap(table);
  arity_t newc = pad_table_size(oldn, newn, oldc);

  if (newc == oldc)
    return;

  kv_t    **newkv  = allocv( newc, kv_t* ), **oldkv = ((kv_t**)table_entries(table));
  int      *newloc = allocv( newc, int ), *oldloc = table->locations;
  
  copyv( newkv, oldkv, min(oldn, newn), kv_t* );
  
  for (arity_t i=0; i<= oldn; i++)
    {	
      kv_t * entry = newkv[i];
      arity_t j = entry->hash & (newc-1);
      while (newloc[j] > -1)
	{								
	  j++;							
	  if (j >= newc)
	    j = 0;						
	}
      newloc[j] = i;
    }
  deallocv(oldloc, oldc, int );
  deallocv(oldkv, oldc, kv_t*);
}

bool table_get(table_t *table, val_t val, obj_t **buf)		
{								
  kv_t    **entries   = (kv_t**)table->entries.data;
  kv_t     *entry     = NULL;
  int      *locations = table->locations;
  hash_t h            = table->hash(val);
  arity_t idx         = h&(table_cap(table)-1);
  int location;
  ord_fn_t ord_fn     = table->ord;
  
      while ((location=locations[idx]) > -1)			
	{							
	  entry = entries[location];
	  if (entry->hash == h && ord_fn(val, entry->cons.hd) == 0)
	    {							
	      if (buf)
		*buf = (obj_t*)entry;
	      return true;				
	    }							
	  idx++;					
	  if (idx >= table_cap(table))				
	    idx = 0;				
	}							
      return false;						
    }

bool_t table_put(table_t  *table, val_t k, obj_t **buf)
    {								
      kv_t **kvs      = (kv_t**)table->entries.data;
      kv_t  *kv       = NULL;
      int   *locs     = table->locations;
      int    loc      = 0;
      hash_t h        = table->hash( k );
      arity_t idx     = h  & (table_cap(table)-1);
      ord_fn_t ord_fn = table->ord;
      								
      while ((loc=locs[idx]) > -1)
	{
	  kv = kvs[loc];
	  
	  if (kv->hash == h && ord_fn(k, kv_key(kv))==0)
	    {							
	      if (buf)
		*buf = (obj_t*)kv;
	      return false;			
	    }					
	  idx++;						
	  if (idx >= table_cap(table))					
	    idx = 0;					
	}							
      obj_t *newkv = table->intern( table, k, h );
      table_entries(table)[loc] = newkv;
      resize_table(table, table_len(table)+1);
      if (buf)
	*buf = newkv;
      return true;
    }

    bool table_pop(table_t *table, val_t k, obj_t **buf)		
    {								
      kv_t **kvs  = (kv_t**)table->entries.data;
      kv_t  *kv   = NULL;				
      hash_t h    = table->hash(k);				
      int loc     = 0;
      int *locs   = table->locations;
      arity_t idx = h & (table_cap(table)-1);
      
      ord_fn_t ord_fn = table->ord;

      while ((loc=locs[idx]) > -1)
	{
	  kv = kvs[loc];
	  if (kv->hash == h && ord_fn(k, kv_key(kv)) == 0)	
	    {							
	      if (buf)	
		*buf = (obj_t*)kv;
	      
	      kvs[loc]  = NULL;
	      locs[idx] = -1;
	      resize_table( table, table_len(table)-1);
	      return true;		
	    }		
	  idx++;
	  if (idx >= table_cap(table))
	    idx = 0;
	}					
      return false;
    }

// symt implementation
int symt_ord(val_t x, val_t y)
{
  char *xchrs = as_ptr(x);
  char *ychrs = as_ptr(as_cons(y)->hd);
  return strcmp(xchrs, ychrs);
}

hash_t symt_hash(val_t val)
{
  char *vchrs = as_ptr(val);

  return hash_string(vchrs);
}

obj_t *symt_intern(table_t *table, val_t key, hash_t h)
{
  // create symbol name
  char  *kchrs  = as_ptr(key);

  atom_t *atom  = new_atom( kchrs, h, true, false );

  key        = tag_val((void*)atom->name->ascii, POINTER);
  val_t b    = tag_val((obj_t*)atom, OBJECT);

  return (obj_t*)new_kv( key, b, atom_type, table_len(table), h );
}

// table implementation
obj_t *table_intern(table_t *table, val_t key, hash_t h)
{
  return (obj_t*)new_kv( key, NIL, any_type, table_len(table), h );
}

// envt implementation
hash_t envt_hash( val_t name )
{
  assert( typeof_val(name) == atom_type );

  return as_atom(name)->hash;
}

int envt_ord( val_t x, val_t y )
{
  assert( typeof_val(x) == atom_type );
  assert( typeof_val(y) == atom_type );

  return x != y; // it expects a 0 if they are equal, so negate the test we're actually interested in
}

obj_t *envt_intern(table_t *table, val_t key, hash_t h)
{
  return (obj_t*)new_var( key, NIL, any_type, table_len(table), h);
}
