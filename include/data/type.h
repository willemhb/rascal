#ifndef rl_data_type_h
#define rl_data_type_h

/**
 *
 * Definition for first class type representation.
 *
 **/

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
// Expression type codes
typedef enum {
  // Immediate types ----------------------------------------------------------
  EXP_NUL=1,
  EXP_NUM,

  // Vm types -----------------------------------------------------------------
  EXP_CHUNK,
  EXP_UPV,
  EXP_CNTL,

  // Language types -----------------------------------------------------------
  EXP_SYM,
  EXP_FUN,
  EXP_TYPE,

  // immutable aggregate types ------------------------------------------------
  EXP_LIST,
  EXP_TUPLE,
  EXP_MAP,

  // mutable aggregate types --------------------------------------------------
  EXP_STACK,
  EXP_TABLE,
  EXP_BUFFER,

  // Internal types -----------------------------------------------------------
  EXP_MAP_NODE,
} ExpType;

// internal type information structs
typedef struct {
  size_t     obsize;
  SizeFn     size_fn;
  AllocFn    alloc_fn;
  CloneFn    clone_fn;
  TraceFn    trace_fn;
  FreeFn     free_fn;
} ObjAPI;

struct Type {
  HEAD;

  /* metadata */
  char*   c_name;
  Sym*    rl_name;

  /* internal type information */
  ObjAPI* obj_api;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------

#endif
