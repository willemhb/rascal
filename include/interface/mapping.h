#ifndef rascal_interface_mapping_h
#define rascal_interface_mapping_h

#include "rascal.h"

typedef struct mapnode_t {
  object_t base;
  uint_t location, keylen;
  object_t *left, *right;
} mapnode_t;

typedef struct maproot_t {
  object_t base;
  uint_t len, offset;
  object_t *nodes, *keys;
} maproot_t;

#define MappingType(type)			\
  {						\
    mapnode_t type##node;			\
    maproot_t type##root;			\
  }

union mapping_t MappingType(mapping);
union dict_t    MappingType(dict);
union set_t     MappingType(set);
union table_t   MappingType(table);

enum mapfl_t {
  mapfl_root   = 0x080
};

#define asnode(x)    ((mapnode_t*)ptr(x))
#define asroot(x)    ((maproot_t*)ptr(x))

#define mapoff(x)    (asroot(x)->offset)
#define maplen(x)    (asroot(x)->len)
#define mapnodes(x)  (asroot(x)->nodes)
#define mapkeys(x)   (asroot(x)->keys)

#define mapklen(x)   (asnode(x)->keylen)
#define maploc(x)    (asnode(x)->location)
#define mapleft(x)   (asnode(x)->left)
#define mapright(x)  (asnode(x)->right)

// methods --------------------------------------------------------------------
object_t *map_construct( type_t t, flags_t fl, size_t n );
int_t     map_init( type_t t, flags_t fl, size_t n, value_t ini, void *spc );
value_t   map_relocate( value_t x );
size_t    map_print( FILE *ios, value_t x );
size_t    map_len( value_t x );

// search for k in the map m. Return the last comparison result, storing the last
// traversed node in b. Most mapping operations can be implemented in terms of this procedure
int_t     map_locate( object_t **b, object_t *m, value_t k );

#endif
