#include <string.h>
#include <assert.h>

#include "obj/atom.h"
#include "obj/type.h"
#include "obj/string.h"

#include "vm/heap.h"
#include "vm/memory.h"
#include "vm/error.h"

#include "util/hashing.h"
#include "util/string.h"
#include "util/number.h"

/* commentary */

/* C types */
typedef struct
{
  char   *name;
  size_t  n_chars;
  hash_t  hash;
} atom_init_t;

/* internal forward declarations */
object_t *make_atom( type_t *type, size_t n, uint flags, void *data );
void      init_atom( object_t *object, type_t *type, size_t n, uint flags, void *data );
void      free_atom( object_t *object );
void      trace_atom( object_t *object );
int       compare_atoms( value_t x, value_t y, bool eq );
hash_t    hash_atom( value_t x );

atom_t **intern_atom( atom_t **root, char *name );

/* globals */
#define SAFE_GENSYM_BUFFER_PAD 66
#define GENSYM_FORMAT "%s#%lu"

atom_t *SymbolTable = NULL;
ulong   SymbolCounter = 0;

type_t AtomType =
  {
    { NULL, NUL, &TypeType, sizeof(type_t)+sizeof("atom"), mem_is_leaf, true, false },

    "atom",
    atom_idno,
    sizeof(atom_t),
    repr_void,
    true,
    OBJECT,
    NUL,

    /* methods */
    make_atom,
    init_atom,
    free_atom,
    trace_atom,
    compare_atoms,
    hash_atom
  };

/* API */
object_t *make_atom( type_t *type, size_t n, uint flags, void *data )
{
  (void)flags;
  (void)n;

  atom_init_t* ini = data;

  atom_t *out = alloc(type->ob_size);
  out->name   = dup_cstring(ini->name, ini->n_chars, sizeof(glyph_t));

  return &out->obj;
}

void init_atom( object_t *object, type_t *type, size_t n, uint flags, void *data )
{
  (void)flags;
  (void)type;

  object->size    += n+1;

  atom_init_t *ini = data;
  atom_t *atom     = as_atom(object);

  atom->left       = NULL;
  atom->right      = NULL;
  atom->hash       = ini->hash;
  atom->idno       = ++SymbolCounter;
}

void free_atom( object_t *object )
{
  dealloc(((atom_t*)object)->name, object->size - object->type->ob_size);
}

void trace_atom( object_t *object )
{
  atom_t *atom = (atom_t*)object;

  mark_object((object_t*)atom->left);
  mark_object((object_t*)atom->right);
}

int compare_atoms( value_t x, value_t y, bool eq )
{
  if (x == y)
    return 0;
  
  if (eq)
    return 1;

  int o = strcmp(as_atom(x)->name, as_atom(y)->name);

  if (o)
    return o;

  return numcmp(as_atom(x)->idno, as_atom(y)->idno);
}

hash_t hash_atom( value_t x )
{
  return as_atom(x)->hash;
}

value_t atom( char *name )
{
  assert(name);
  assert(*name != '\0');

  atom_t **space = intern_atom(&SymbolTable, name);

  if ( *space == NULL )
    {
      atom_init_t ini = { name, strlen(name), hash_str8(name) };
      object_t   *new = make_object(&AtomType, 0, 0, &ini);
      init_object(new, &AtomType, 0, 0, &ini);
      *space = (atom_t*)new;
    }

  return tag_object((object_t*)*space);
}

value_t gensym( char *name )
{
  if ( name == NULL )
    name = "symbol";

  /* hash on a string created from the name and the idno (to avoid gensym hash collisons) */
  atom_init_t init = { name, strlen(name), 0 };
  size_t padded    = init.n_chars + SAFE_GENSYM_BUFFER_PAD;
  
  char buffer[init.n_chars+SAFE_GENSYM_BUFFER_PAD];
  memset(buffer, 0, padded);

  snprintf(buffer, padded, GENSYM_FORMAT, init.name, SymbolCounter+1);
  
  init.hash = hash_str8(buffer);

  object_t *out = make_object(&AtomType, 0, 0, &init);
  
  init_object(out, &AtomType, 0, 0, &init);

  return tag_object(out);
}

/* runtime */
void rl_obj_atom_init( void )
{
  AtomType.obj.next = Heap.live_objects;
  Heap.live_objects = &AtomType.obj;
}

void rl_obj_atom_mark( void )
{
  mark_object(&AtomType.obj);
}

/* convenience */
