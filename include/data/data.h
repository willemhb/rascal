#ifndef rl_data_data_h
#define rl_data_data_h

/**
 *
 * Pulls together all the headers in the data/ subdirectory
 * and provides a common initialization function.
 *
 **/

// headers -------------------------------------------------------------------- 
#include "data/base.h"

#include "data/none.h"
#include "data/nul.h"
#include "data/eos.h"
#include "data/bool.h"
#include "data/glyph.h"
#include "data/chunk.h"
#include "data/alist.h"
#include "data/buf16.h"
#include "data/ref.h"
#include "data/upv.h"
#include "data/env.h"
#include "data/port.h"
#include "data/fun.h"
#include "data/sym.h"
#include "data/str.h"
#include "data/list.h"
#include "data/num.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void toplevel_init_data(void);

#endif
