#ifndef rl_val_hamt_h
#define rl_val_hamt_h

#include "util/number.h"

#include "val/object.h"

/* common definitions and utilities for HAMT types
   (internal vector, dict, and set node types).    */

/* C types */
typedef struct VecNode     VecNode;
typedef struct VecNode     VecLeaf;
typedef struct DictNode    DictNode;
typedef struct DictLeaf    DictLeaf;
typedef struct SetNode     SetNode;
typedef struct SetLeaf     SetLeaf;

/* Globals */
#define HAMT_LEVEL_SIZE 0x40 // 64
#define HAMT_SHIFT      0x06 // 06
#define HAMT_MAX_SHIFT  0x30 // 48
#define HAMT_LEVEL_MASK 0x3f // 63

/* External API */
void*            freeze_hamt(void* obj);
extern Vector*   freeze_vec(Vector* v);
extern VecNode*  freeze_vec_node(VecNode* n);
extern Dict*     freeze_dict(Dict* d);
extern DictNode* freeze_dict_node(DictNode* n);
extern DictLeaf* freeze_dict_leaf(DictLeaf* l);
extern Set*      freeze_set(Set* s);
extern SetNode*  freeze_set_node(SetNode* n);
extern SetLeaf*  freeze_set_leaf(SetLeaf* l);

#define freeze(x)                               \
  generic((x),                                  \
          Vector*:freeze_vec,                   \
          VecNode*:freeze_vec_node,             \
          Dict*:freeze_dict,                    \
          DictNode*:freeze_dict_node,           \
          DictLeaf*:freeze_dict_leaf,           \
          Set*:freeze_set,                      \
          SetNode*:freeze_set_node,             \
          SetLeaf*:freeze_set_leaf,             \
          default:freeze_hamt)(x)

void*            unfreeze_hamt(void* obj);
extern Vector*   unfreeze_vec(Vector* v);
extern VecNode*  unfreeze_vec_node(VecNode* n);
extern Dict*     unfreeze_dict(Dict* d);
extern DictNode* unfreeze_dict_node(DictNode* n);
extern DictLeaf* unfreeze_dict_leaf(DictLeaf* l);
extern Set*      unfreeze_set(Set* s);
extern SetNode*  unfreeze_set_node(SetNode* n);
extern SetLeaf*  unfreeze_set_leaf(SetLeaf* l);

#define unfreeze(x)                               \
  generic((x),                                    \
          Vector*:unfreeze_vec,                   \
          VecNode*:unfreeze_vec_node,             \
          Dict*:unfreeze_dict,                    \
          DictNode*:unfreeze_dict_node,           \
          DictLeaf*:unfreeze_dict_leaf,           \
          Set*:unfreeze_set,                      \
          SetNode*:unfreeze_set_node,             \
          SetLeaf*:unfreeze_set_leaf,             \
          default:unfreeze_hamt)(x)


bool   get_hamt_editp(void* obj);
bool   set_hamt_editp(void* obj);
size_t get_hamt_cnt(void* obj);
size_t set_hamt_cnt(void* obj, size_t n);
size_t get_hamt_cap(void* obj);
size_t set_hamt_cap(void* obj, size_t n);
size_t get_hamt_shift(void* obj);
size_t set_hamt_shift(void* obj, size_t n);

int    hamt_hash_to_index(hash_t h, size_t sh, size_t bm);
size_t hamt_index_for_level(size_t i, size_t sh);
void*  freeze_hamt(void* obj);
void   init_hamt(void* obj, void** arr, void* data, size_t cnt, size_t sh, size_t es);
size_t resize_hamt_array(void* obj, void** arr, size_t new_cnt, size_t els);

#endif
