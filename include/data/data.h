#ifndef rl_data_data_h
#define rl_data_data_h

/* pulls together all the headers in the data/ subdirectory. and provides
   a common initialization function. */

#include "data/obj.h"
#include "data/expr.h"
#include "data/array.h"
#include "data/table.h"

#include "data/types/none.h"
#include "data/types/nul.h"
#include "data/types/eos.h"
#include "data/types/bool.h"
#include "data/types/glyph.h"
#include "data/types/chunk.h"
#include "data/types/alist.h"
#include "data/types/buf16.h"
#include "data/types/ref.h"
#include "data/types/upv.h"
#include "data/types/env.h"
#include "data/types/port.h"
#include "data/types/fun.h"
#include "data/types/sym.h"
#include "data/types/str.h"
#include "data/types/list.h"
#include "data/types/num.h"

static inline void toplevel_init_data(void) {
  /* Initialize all builtin type information (probably needs to be called first). */
  toplevel_init_data_type_none();
  toplevel_init_data_type_nul();
  toplevel_init_data_type_eos();
  toplevel_init_data_type_bool();
  toplevel_init_data_type_glyph();
  toplevel_init_data_type_chunk();
  toplevel_init_data_type_alist();
  toplevel_init_data_type_buf16();
  toplevel_init_data_type_ref();
  toplevel_init_data_type_upv();
  toplevel_init_data_type_env();
  toplevel_init_data_type_port();
  toplevel_init_data_type_fun();
  toplevel_init_data_type_sym();
  toplevel_init_data_type_str();
  toplevel_init_data_type_list();
  toplevel_init_data_type_num();
}

#endif
