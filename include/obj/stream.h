#ifndef rascal_port_h
#define rascal_port_h
 
#include <stdio.h>

#include "obj/object.h"
#include "obj/array.h"
#include "utils/Ctype.h"

// core io object types and their apis are implemented here

// io types

typedef enum
  {
    port_fl_lispport=0x001,
    port_fl_textport=0x002,
    port_fl_binport =0x003,
    port_fl_inport  =0x004,
    port_fl_outport =0x008,
    port_fl_ready   =0x010,
  } port_fl_t;

struct bytes_t
{
  ARRAY_SLOTS(char, 2);
};

struct text_t
{
  ARRAY_SLOTS(char, 2);
};

struct stream_t
{
  object_t  object;

  port_fl_t flags;
  Ctype_t   Ctype;

  rl_data_t value;
  FILE     *stream;

  union
  {
    bytes_t    *bin;
    text_t     *txt;
  } buf;
};

// globals
extern stream_t Ins, Outs, Errs;

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
value_t   stream_take(stream_t *port);
void      port_give(stream_t *port, value_t value);

// initialization
void port_init( void );

// convenience
#define is_port(val)     isa(val, PORT)
#define as_port(val)     ((stream_t*)as_ptr(val))

#endif
