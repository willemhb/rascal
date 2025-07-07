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

#include "data/nul.h"
#include "data/num.h"
#include "data/bool.h"
#include "data/glyph.h"

#include "data/chunk.h"
#include "data/ref.h"
#include "data/env.h"
#include "data/upv.h"

#include "data/sym.h"
#include "data/port.h"
#include "data/fun.h"

#include "data/tuple.h"
#include "data/list.h"
#include "data/str.h"
// #include "data/bin.h"

// #include "data/mut-tuple.h"
#include "data/mut-list.h"
// #include "data/mut-map.h"
// #include "data/mut-str.h"
#include "data/mut-bin.h"


// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void toplevel_init_data(void);

#endif
