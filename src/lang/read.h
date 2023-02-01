#ifndef rascal_read_h
#define rascal_read_h

#include <stdio.h>

#include "rascal.h"
#include "value.h"

/* C types */
enum ReadState {
  ReadReady,
  ReadExpr,
  ReadEOS,
  ReadError
};

#include "decl/alist.h"

ALIST(Buffer, ascii);


struct Reader {
  Buffer     buffer;
  Values     subxpr;

  FILE      *infile;
  int        line;
  ReadState  state;
  Value      expr;
};

/* API */
Value read(void);

#endif
