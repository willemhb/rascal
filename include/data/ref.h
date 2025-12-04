#ifndef rl_data_ref_h
#define rl_data_ref_h

/**
 *
 * Implementation of the reference type, which stores information about
 * individual variables.
 *
 **/

// headers --------------------------------------------------------------------
#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
typedef enum {
  REF_UNDEF,
  REF_GLOBAL,
  REF_LOCAL,
  REF_LOCAL_UPVAL,
  REF_CAPTURED_UPVAL
} RefType;

struct Ref {
  HEAD;

  Ref*    captures;
  Sym*    name;
  RefType ref_type;
  int     offset;
  bool    final;
};

// globals --------------------------------------------------------------------
#define as_ref(x) ((Ref*)as_obj(x))

// function prototypes --------------------------------------------------------
Ref* mk_ref(Sym* n, int o);

// initialization -------------------------------------------------------------
void toplevel_init_data_ref(void);

#endif
