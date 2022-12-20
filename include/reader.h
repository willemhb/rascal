#ifndef rl_reader_h
#define rl_reader_h

#include "rascal.h"
#include "val.h"

/* C types */
typedef void (*read_fn_t)(reader_t *reader, int dispatch);

typedef enum read_state_t read_state_t;

enum read_state_t {
  read_state_expr,
  read_state_ready,
  read_state_eos
};

#include "tpl/decl/hashmap.h"
#include "tpl/decl/buffer.h"

HASHMAP(readtable, int, read_fn_t);
BUFFER(readbuffer, ascii_t);

struct reader_t {
  FILE         *infile;
  read_state_t  state;
  readtable_t  *table;  /* dispatch table */
  readbuffer_t *buffer; /* token buffer */
  vals_t       *subx;   /* sub-expressions (for reading structures). */
  val_t         value;
};

/* globals */
extern reader_t Reader;

/* API */
val_t read_expression(reader_t *reader);
void  clear_reader(reader_t *reader);

/* initialization */
void reader_init(void);

#endif
