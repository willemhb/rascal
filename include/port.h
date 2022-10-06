#ifndef rascal_port_h
#define rascal_port_h
 
#include <stdio.h>

#include "array.h"

// core io object types and their apis are implemented here

// io types
typedef struct buffer_t
{
  obj_t obj;
  ARRAY_SLOTS(char);
} buffer_t;

typedef enum port_fl_t
  {
    port_fl_inport  =0x040,
    port_fl_outport =0x080,
    port_fl_lispport=0x100,
    port_fl_ready   =0x200,
  } port_fl_t;

struct port_t
{
  obj_t      obj;

  FILE      *stream;
  buffer_t  *buffer;

  val_t      value;
  val_t      temp;
};

// globals
extern port_t Ins, Outs, Errs;

// forward declarations
buffer_t *new_buffer(void);
void      init_buffer(buffer_t *buffer);
void      free_buffer(obj_t *obj);
void      clear_buffer(buffer_t *buffer);
void      reset_buffer(buffer_t *buffer);
void      resize_buffer(buffer_t *buffer, size_t newl);
arity_t   buffer_push(buffer_t *buffer, char val);
arity_t   buffer_write(buffer_t *buffer, char *src, arity_t n);
bool      buffer_pop(buffer_t *buffer, char *buf);

port_t   *new_port(void);
void      init_port(port_t *port, FILE *stream, flags16_t flags);
void      mark_port(obj_t *obj);
void      free_port(obj_t *obj);
void      reset_port(port_t *port);
void      port_close(port_t *port);

// IO predicates
bool      port_eosp(port_t *port);
bool      port_insp(port_t *port);
bool      port_outsp(port_t *port);
bool      port_readyp(port_t *port);

// low level IO
int32_t   port_prinf(port_t *port, char *fmt, ...);
int32_t   port_princ(port_t *port, int32_t ch);
int32_t   port_readc(port_t *port);
int32_t   port_ungetc(port_t *port, int32_t ch);
int32_t   port_peekc(port_t *port);
val_t     port_take(port_t *port);
void      port_give(port_t *port, val_t val);

// initialization
void port_init( void );

// convenience
#define is_port(val)     isa(val, PORT)
#define as_port(val)     ((port_t*)as_ptr(val))

#endif
