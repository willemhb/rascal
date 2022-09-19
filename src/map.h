#ifndef rascal_map_h
#define rascal_map_h

#include "hamt.h"

// C types --------------------------------------------------------------------
typedef struct
{
  HAMT_FL_SPEC;
} MapFl;

typedef union
{
  Map **data;

  struct
  {
    Value key;
    Value val;
  };
  
} MapData;

struct Map
{
  HAMT_SPEC(MapData, MapFl);
};

#define MAP_MAX_DEPTH   8
#define MAP_MAX_LENGTH 64

typedef struct
{
  HAMT_BUFFER_SPEC( Map, MAP_MAX_DEPTH );
} MapPathBuffer;

// forward declarations -------------------------------------------------------
Map   *newMap( Value *args, Arity nArgs );
Value *mapGet( Map *m, Value key, MapPathBuffer *buffer );
Map   *mapSet( Map *m, Value key, Value val, Bool inPlace );
Map   *mapPut( Map *m, Value key, Bool inPlace );
Map   *mapPop( Map *m, Value key, Bool inPlace );

// utility macros & statics ---------------------------------------------------
#define mapArity(map)      (asMap(map)->object.arity)
#define mapLength(map)     (asMap(map)->object.flags.length)
#define mapDepth(map)      (asMap(map)->object.flags.depth)
#define mapShared(map)     (asMap(map)->object.flags.shared)
#define mapInlined(map)    (asMap(map)->object.flags.inlined)
#define mapInternal(map)   (asMap(map)->object.flags.internal)
#define mapLeaf(map)       (asMap(map)->object.flags.leaf)
#define mapBitmap(map)     (asMap(map)->bitmap)
#define mapHash(map)       (asMap(map)->hash)
#define mapCache(map)      (asMap(map)->cache)
#define mapParent(map)     (asMap(map)->parent)
#define mapData(map)       (asMap(map)->_data.data)
#define mapKey(map)        (asMap(map)->_data.key)
#define mapVal(map)        (asMap(map)->_data.val)

#endif
