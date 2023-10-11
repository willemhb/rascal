#ifndef rascal_reader_h
#define rascal_reader_h

#include "common.h"
#include "object.h"

// generics
#include "tpl/declare.h"

typedef void (*ReadFn)(Vm* vm, int dispatch);

ARRAY_TYPE(TextBuffer, char);
TABLE_TYPE(ReadTable, readTable, int, ReadFn);

// C types
typedef enum ReaderState {
  READER_READY,
  READER_EXPRESSION,
  READER_DONE,
  READER_ERROR
} ReaderState;

struct Reader {
  FILE*       source;
  ReaderState state;
  TextBuffer  buffer;
  ReadTable   table;
  Values      stack;
};

// globals
extern Value QuoteSym;

// external API
void  initReader(Reader* reader);
void  freeReader(Reader* reader);
void  resetReader(Reader* reader);
void  syncReader(Reader* reader);

// main logic
Value read(Vm* vm);

#endif
