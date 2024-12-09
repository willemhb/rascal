#ifndef rl_val_map_h
#define rl_val_map_h


#include "val/object.h"

#include "util/amt.h"

/* Rascal mapping collection types. */
/* C types */
/* Persistent Map and supporting types. */
struct Map {
  HEADER;

  // size information
  size64 cnt;
  
  union {
    MNode*  root; // large maps (more than 16 keys)
    MLeaf** kvs;  // smaller maps
  };
};

struct MNode {
  HEADER;
  size16 cnt, cap, shft;
  size64 bm;
  Obj**  cn;
};

struct MLeaf {
  HEADER;
  Val key;
  Val val;
};

struct MLeafs {
  HEADER;
  size64  cnt;
  MLeaf** leafs;
};

// Map iterator
typedef struct {
  Map*    src;            // source object
  size64  cnt;            // current count
  MLeaf** small;          // copy of the Leaf array for small maps
  size64  lcnt;           // number of levels being tracked
  Obj*    lvls[HT_MAXD];
  Obj*    offs[HT_MAXD];
} MapIter;

// Mutable Maps and supporting types
typedef struct {
  Val    key;
  hash64 hash;
  Val    val;
} MMNode;

struct Table {
  HEADER;

  // size information
  size64 cnt, cap, nts;

  // actual key/value pairs
  MMNode* kvs;
};

/* Globals */
extern Type MapType, MNodeType, MLeafType, MLeafsType, MMapType;

/* APIs */
// Map and supporting types
#define is_map(x)   has_type(x, T_MAP)
#define as_map(x)   ((Map*)as_obj(x))

Map* mk_map(size64 n, Val* kvs);
bool map_get(Map* m, Val k, Val* v);
bool map_has(Map* m, Val k);
Map* map_set(Map* m, Val k, Val v);
Map* map_pop(Map* m, Val k);

MLeaf* map_iter(Map* m, MapIter* i);
MLeaf* map_iter_next(MapIter* i);

#define is_mnode(x) has_vtype(x, T_MNODE)
#define as_mnode(x) ((MNode*)as_obj(x))

MNode* mk_mnode(size64 shft);

#define is_mleaf(x) has_vtype(x, T_MLEAF)
#define as_mleaf(x) ((MLeaf*)as_obj(x))

MLeaf* mk_mleaf(hash64 h, Val k, Val v);

#define is_mmap(x) has_vtype(x, T_TABLE)
#define as_mmap(x) ((MMap*)as_obj(x))

MMap*  mk_mmap(State* vm);
void   free_mmap(State* vm, void* x);
void   init_mmap(State* vm, MMap* m);
bool   mmap_get(MMap* m, Val k, Val* v);
bool   mmap_set(MMap* m, Val k, Val v);
bool   mmap_add(MMap* m, Val k, MMNode** l);
bool   mmap_del(MMap* m, Val k, Val* v);
void   join_mmaps(MMap* mx, MMap* my);

/* Initialization */
void rl_toplevel_init_map(void);

#endif
