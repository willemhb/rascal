#ifndef rl_port_h
#define rl_port_h

#include <stdio.h>
#include <stdarg.h>

#include "val/val.h"

// simple wrapper around a C file object
typedef enum {
  INPUT_PORT=0x01,
  OUTPUT_PORT=0x02,
  TEXT_PORT=0x04,
  BINARY_PORT=0x08,
  LISP_PORT=0x10,
} IOMode;

#define IO_INPUT_MASK 0x03
#define IO_PTYPE_MASK 0x0c
#define IO_LISP_MASK  0x10

struct Port {
  HEAD;

  IOMode mode;
  FILE* ios;
};

// port API
Port* mk_port(RlState* rls, FILE* ios, IOMode io_mode);
Port* mk_port_s(RlState* rls, FILE* ios, IOMode io_mode);
Port* tmp_port(RlState* rls);
Port* tmp_port_s(RlState* rls);
Port* open_port(RlState* rls, char* fname, char* mode);
Port* open_port_s(RlState* rls, char* fname, char* mode);
void close_port(Port* port);
bool peof(Port* p);
int pseek(Port* p, long off, int orig);
void prewind(Port* p);
Glyph pgetc(Port* p);
Glyph pungetc(Port* p, Glyph g);
Glyph ppeekc(Port* p);
char* pgets(char* buf, size_t bsize, Port* p);
int pprintf(Port* p, char* fmt, ...);
int pvprintf(Port* p, char* fmt, va_list va);
void port_newline(Port* out, int n);
void clear_port(Port* in);

// convenience macros
#define as_port(x)        ((Port*)as_obj(x))
#define as_port_s(rls, x) ((Port*)as_obj_s(rls, &PortType, x))

#endif
