#ifndef text_h
#define text_h

#include <stdio.h>

#include "value.h"

/* text and IO types */
// C types --------------------------------------------------------------------
typedef enum {
  CREATE_MODE = 0x01,
  APPEND_MODE = 0x02,
  WRITE_MODE  = 0x03,
  TEXT_PORT   = 0x04,
  BINARY_PORT = 0x08,
  INPUT_PORT  = 0x10,
  OUTPUT_PORT = 0x20
} port_fl_t;

struct port_t {
  HEADER;
  val_type_t encoding;
  bool       is_lisp_port;
  FILE*      stream;
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
// port -----------------------------------------------------------------------
port_t* port(FILE* ios, flags fl);
port_t* rl_open(const char* fname, const char* mode);
void    rl_close(port_t* ios);

// port -> FILE operations ----------------------------------------------------
int  rl_putc(int ch, port_t* ios);
int  rl_getc(port_t* ios);
int  rl_ungetc(int ch, port_t* ios);
int  rl_peekc(port_t* ios);
int  rl_puts(char* s, port_t* ios);
int  rl_printf(port_t* ios, const char* fmt, ...);
bool rl_feof(port_t* ios);

// string ---------------------------------------------------------------------
string_t* string(char* chars);

// binary ---------------------------------------------------------------------
binary_t* binary(usize n, ubyte* bytes);

// buffer ---------------------------------------------------------------------
buffer_t* buffer(int elsize, int encoded);

#endif
