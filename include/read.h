#ifndef rascal_read_h
#define rascal_read_h

#include "table.h"

// read table, reader entry, and reader implementations are here
typedef struct rentry_t
{
  obj_t      obj;
  ENTRY_SLOTS(char32_t, dispatch, reader_fn_t, handler);
} rentry_t;

typedef struct readt_t
{
  obj_t obj;
  TABLE_SLOTS(rentry_t*);
} readt_t;


// read table implementation goes here
extern readt_t Reader;

// forward declarations
void   trace_readt(obj_t *obj);
void   init_readt(readt_t *readt);
void   free_readt(obj_t *obj);
void   clear_readt(readt_t *readt);
void   resize_readt(readt_t *readt, arity_t newl);
void   rehash_readt(rentry_t**old,arity_t oldc,rentry_t**new,arity_t newc);
bool   readt_put(readt_t *readt,char32_t dispatch, rentry_t **buf);
bool   readt_get(readt_t *readt,char32_t dispatch, rentry_t **buf);

// toplevel read
val_t  lisp_read(port_t *port);

// initialization
void read_init( void );

#endif
