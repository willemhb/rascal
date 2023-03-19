#ifndef text_h
#define text_h

#include "value.h"

/* text and IO types */

// C types --------------------------------------------------------------------
struct port_t {
  HEADER;
  long desc, pos;
  int flags;
  int lisp;
  buffer_t* buffer;
};

struct binary_t {
  HEADER;
  usize  len;
  ubyte  bytes[];
};

struct string_t {
  HEADER;
  usize   len;
  ubyte   chars[];
};

struct buffer_t {
  HEADER;
  usize len, cap;
  int elsize, encoded;
  ubyte *array;
};

// API ------------------------------------------------------------------------
int rl_putc(port_t* ios, int ch);
int rl_getc(port_t* ios);
int rl_peekc(port_t* ios);
int rl_printf(port_t* ios, const char* fmt, ...);

#endif
