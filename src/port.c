#include <string.h>
#include <stdarg.h>

#include "port.h"
#include "memory.h"
#include "utils.h"


// io types
// buffer
OBJ_NEW(buffer);
ARRAY_INIT(buffer, char, Ctype_ascii, BUFFER);
ARRAY_FREE(buffer, char);
ARRAY_RESIZE(buffer, char, BUFFER, pad_stack_size);
ARRAY_WRITE(buffer, char);
ARRAY_PUSH(buffer, char);
ARRAY_POP(buffer, char);
ARRAY_CLEAR(buffer, char, BUFFER);

// port type
OBJ_NEW(port);

void init_port(port_t *port, FILE *stream, flags16_t flags)
{
  init_obj( &port->obj, PORT, flags );
  port->stream = stream;
  port->value = port->temp = NUL;
  port->buffer = new_buffer();
  init_buffer( port->buffer );
}

void trace_port(obj_t *obj)
{
  port_t *port = (port_t*)obj;

  mark_obj((obj_t*)port->buffer);
  mark_val(port->value);
  mark_val(port->temp);
}

void free_port(obj_t *obj)
{
  port_t *port = (port_t*)obj;

  port_close(port);
  free_obj((obj_t*)port->buffer);
}

void port_close(port_t *port)
{
  FILE *stream = port->stream;

  if (stream == stdin || stream == stdout || stream == stderr)
    return;

  fclose(stream);
}

// IO predicates
bool port_eosp(port_t *port)
{
  
  return feof(port->stream);
}

bool port_insp(port_t *port)
{
  return flag_p(port->obj.flags, port_fl_inport);
}

bool port_outsp(port_t *port)
{
  return flag_p(port->obj.flags, port_fl_outport);
}

bool port_readyp(port_t *port)
{
  return flag_p(port->obj.flags, port_fl_ready);
}

// low level IO
int32_t port_prinf(port_t *port, char *fmt, ...)
{
  assert(port_outsp(port));
  va_list va;
  va_start(va, fmt);
  arity_t out = vfprintf(port->stream, fmt, va);
  va_end(va);
  return out;
}

int32_t port_princ(port_t *port, int32_t ch)
{
  return fputc( ch, port->stream);
}

int32_t port_readc(port_t *port)
{
  if (port_eosp(port))
    return EOF;

  return fgetc(port->stream);
}

int32_t port_peekc(port_t *port)
{
  if (port_eosp(port))
    return EOF;

  int32_t out = fgetc(port->stream);
  ungetc(out, port->stream);
  return out;
}

int32_t port_ungetc(port_t *port, int32_t ch)
{
  if (port_eosp(port))
    return EOF;

  return ungetc(ch, port->stream);
}

val_t  port_take(port_t *port)
{
  if (port_readyp(port) || port_eosp(port))
    return port->value;
  
  val_t out      = port->value;
  port->value    = NUL;

  port->obj.flags |= port_fl_ready;
  
  clear_buffer(port->buffer);
  return out;
}

void port_give(port_t *port, val_t val)
{
  port->obj.flags &= ~port_fl_ready;
  port->value      = val;
}

// initialization
void port_init( void )
{
  init_port(&Ins, stdin, port_fl_inport|port_fl_lispport|Ctype_ascii);
  init_port(&Outs, stdout, port_fl_outport|port_fl_lispport|Ctype_ascii);
  init_port(&Errs, stderr, port_fl_outport|port_fl_lispport|Ctype_ascii);
}
