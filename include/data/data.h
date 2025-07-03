#ifndef rl_data_data_h
#define rl_data_data_h

/* pulls together all the headers in the data/ subdirectory. and provides
   a common initialization function. */

// headers -------------------------------------------------------------------- 
#include "data/base.h"

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

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void toplevel_init_data(void);

#endif
