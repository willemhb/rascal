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

// globals --------------------------------------------------------------------
extern data_type_t PortType, BinaryType, StringType, BufferType,
  AsciiType, Latin1Type, Utf8Type, Utf16Type, Utf32Type;

// API ------------------------------------------------------------------------
// port -----------------------------------------------------------------------
#define is_port(x) ISA(x, PortType)
#define as_port(x) ASP(x, port_t)

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
#define   is_string(x) ISA(x, StringType)
#define   as_string(x) ASP(x, string_t)

string_t* string(char* chars);

// binary ---------------------------------------------------------------------
#define   is_binary(x) ISA(x, BinaryType)
#define   as_binary(x) ASP(x, binary_t)

binary_t* binary(usize n, ubyte* bytes);

// buffer ---------------------------------------------------------------------
#define   is_buffer(x) ISA(x, BufferType)
#define   as_buffer(x) ASP(x, buffer_t)

buffer_t* buffer(int elsize, int encoded);
usize     buffer_write(buffer_t* self, usize n, void* dat);
void      buffer_clear(buffer_t* self);

// character types ------------------------------------------------------------
// ascii ----------------------------------------------------------------------
#define is_ascii(x) IST(x, ASCIITAG, WTAG_MASK)
#define as_ascii(x) ASV(x, ascii_t)

// latin1 ---------------------------------------------------------------------
#define is_latin1(x) IST(x, LATIN1TAG, WTAG_MASK)
#define as_latin1(x) ASV(x, latin1_t)

// utf8 ---------------------------------------------------------------------
#define is_utf8(x)   IST(x, UTF8TAG, WTAG_MASK)
#define as_utf8(x)   ASV(x, utf8_t)

// utf16 ---------------------------------------------------------------------
#define is_utf16(x)  IST(x, UTF16TAG, WTAG_MASK)
#define as_utf16(x)  ASV(x, utf16_t)

// UTF32 ---------------------------------------------------------------------
#define is_utf32(x)  IST(x, UTF32TAG, WTAG_MASK)
#define as_utf32(x)  ASV(x, utf32_t)

#endif
