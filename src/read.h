#ifndef rascal_read_h
#define rascal_read_h

#include "rascal.h"

/* C types */

typedef enum {
  ReaderExpression,
  ReaderReady,
  ReaderDispatching,
  ReaderEOF,
  ReaderError
} ReadState;

#include "decl/htable.h"
#include "decl/alist.h"

HTABLE(Readers, char, ReadFn);
ALIST(Buffer, ascii);

struct Reader {
  Readers    readers;
  Buffer     buffer;

  FILE      *inFile;
  Value      expression;
  int        lineNo;
  int        colNo;
  ReadState  state;
  bool       panicking;
};

/* public API */
Value read(void);
Value readLn(void);
void  addMacro(Reader *reader, char dispatch, ReadFn handler);

#endif
