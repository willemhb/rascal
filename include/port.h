#ifndef rascal_port_h
#define rascal_port_h

#include "common.h"

// C types --------------------------------------------------------------------
struct port_t {
  uint_t   length;
  uchar_t  Ctype;
  uchar_t  in;
  uchar_t  out;
  uchar_t  token;
  ushort_t tag;

  value_t   stream;
  value_t   buffer;
  value_t   value;

  char     name[0];
};

struct io_buffer_t {
  uint_t   flags;
  ushort_t Ctype;
  ushort_t tag;

  uint_t   bufi;
  uint_t   bufc;

  schar_t *data;
};

// utilities ------------------------------------------------------------------
size_t port_puts(port_t *ios, char *s);
size_t port_putc(port_t *ios, int ch);
size_t port_putf(port_t *ios, const char *fmt, ...);

// implementation -------------------------------------------------------------
hash_t port_hash(value_t x);
sint_t port_order(value_t x, value_t y);

// native functions -----------------------------------------------------------

// initialization -------------------------------------------------------------

#endif
