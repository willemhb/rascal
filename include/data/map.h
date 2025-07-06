#ifndef rl_data_map_h
#define rl_data_map_h

/**
 *
 * Core associative type. Rascal maps are HAMT based.
 *
 **/

// headers --------------------------------------------------------------------
#include "data/base.h"

// macros ---------------------------------------------------------------------
#define is_map(x)         has_type(x, EXP_MAP)
#define as_map(x)         ((Map*)as_obj(x))
#define map_node_shift(n) ((n)->flags)

// C types --------------------------------------------------------------------
struct Map {
  HEAD;

  size_t   arity;
  MapNode* root;
};

struct MapNode {
  HEAD;

  size_t bitmap;
  Obj**  children;
};

struct MapLeaf {
  HEAD;

  Val key;
  Val val;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------

#endif
