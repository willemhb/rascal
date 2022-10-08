#ifndef rascal_port_h
#define rascal_port_h
 
#include <stdio.h>

#include "array.h"

// core io object types and their apis are implemented here

// io types
typedef struct buffer_t
{
  object_t object;
  ARRAY_SLOTS(char);
} buffer_t;

typedef enum port_fl_t
  {
    port_fl_inport  =0x040,
    port_fl_outport =0x080,
    port_fl_lispport=0x100,
    port_fl_ready   =0x200,
  } port_fl_t;

struct stream_t
{
  object_t      obj;

  FILE      *stream;
  buffer_t  *buffer;

  value_t      value;
  value_t      temp;
};

// globals
extern stream_t Ins, Outs, Errs;

// forward declarations
buffer_t *new_buffer(void);
void      init_buffer(buffer_t *buffer);
void      free_buffer(object_t *obj);
void      clear_buffer(buffer_t *buffer);
void      reset_buffer(buffer_t *buffer);
void      resize_buffer(buffer_t *buffer, size_t newl);
arity_t   buffer_push(buffer_t *buffer, char val);
arity_t   buffer_write(buffer_t *buffer, char *src, arity_t n);
bool      buffer_pop(buffer_t *buffer, char *buf);

stream_t   *new_port(void);
void      init_port(stream_t *port, FILE *stream, flags16_t flags);
void      mark_port(object_t *obj);
void      free_port(object_t *obj);
void      reset_port(stream_t *port);
void      port_close(stream_t *port);

// IO predicates
bool      port_eosp(stream_t *port);
bool      port_insp(stream_t *port);
bool      port_outsp(stream_t *port);
bool      port_readyp(stream_t *port);

// low level IO
int32_t   port_prinf(stream_t *port, char *fmt, ...);
int32_t   port_princ(stream_t *port, int32_t ch);
int32_t   port_readc(stream_t *port);
int32_t   port_ungetc(stream_t *port, int32_t ch);
int32_t   port_peekc(stream_t *port);
value_t     stream_take(stream_t *port);
void      port_give(stream_t *port, value_t value);

// initialization
void port_init( void );

// convenience
#define is_port(val)     isa(val, PORT)
#define as_port(val)     ((stream_t*)as_ptr(val))

#endif
