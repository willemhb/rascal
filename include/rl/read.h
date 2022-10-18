#ifndef rascal_read_h
#define rascal_read_h

#include "obj/array.h"
#include "obj/stream.h"
#include "obj/record.h"

// read table, reader entry, and reader implementations are here
struct dispatches_t
{
  TABLE_SLOTS(dispatch_t*);
};

struct backrefs_t
{
  TABLE_SLOTS(entry_t*);
};

struct gensyms_t
{
  TABLE_SLOTS(symbol_t*);
};

struct reader_t
{
  record_t  record;

  reader_t     *parent;
  dispatches_t *dispatches;
  backrefs_t   *backrefs;
  gensyms_t    *gensyms;
};

// read table implementation goes here
extern reader_t Reader;

// toplevel read
value_t  lisp_read(stream_t *port);

// initialization
void read_init( void );

#endif
