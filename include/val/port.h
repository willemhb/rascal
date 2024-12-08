#ifndef rl_val_port_h
#define rl_val_port_h

#include "val/object.h"

/* C types */
struct Port {
  HEADER;

  bool  isrl; // represents Rascal source (versus plain text and binary)
  FILE* ios;  // wrapped FILE object
};

/* Globals */
extern Port Ins, Outs, Errs;

/* API */

/* Initialization */
#define is_port(x) has_type(x, T_PORT)
#define as_port(x) ((Port*)as_obj(x))

Port* rl_popen(char* f, bool r);
int   rl_pclose(Port* p);
int   rl_printf(Port* p, char* fmt, ...);
int   rl_printv(Port* p, char* fmt, va_list va);
int   rl_putc(Port* p, Glyph g);
int   rl_getc(Port* p);
int   rl_ungetc(Port* p, Glyph g);
int   rl_peekc(Port* p);

#endif
