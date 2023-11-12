#ifndef rl_val_hamt_h
#define rl_val_hamt_h

#include "val/object.h"

/* common definitions and utilities for HAMT types
   (internal vector, dict, and set node types).    */

/* C types */
typedef enum {
  EDITP =0x02000000u,
} HamtFl;

/* Globals */
#define HAMT_LEVEL_SIZE 0x40 // 64
#define HAMT_SHIFT      0x06 // 06
#define HAMT_MAX_SHIFT  0x30 // 48
#define HAMT_LEVEL_MASK 0x3f // 63

/* External API */
bool   hamt_get_editp(void* obj);
void   hamt_set_editp(void* obj);
size_t hamt_get_cnt(void* obj);
void   hamt_set_cnt(void* obj, size_t n);
size_t hamt_get_cap(void* obj);
void   hamt_set_cap(void* obj, size_t n);
size_t hamt_get_shift(void* obj);
void   hamt_set_shift(void* obj, size_t n);

#endif
