#ifndef rl_lang_io_h
#define rl_lang_io_h

/* Medium-level Rascal IO facilities. */
// headers --------------------------------------------------------------------
#include <stdarg.h>

#include "common.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Port* open_port(char* fname, char* mode);
void  close_port(Port* port);
bool  peof(Port* p);
int   pseek(Port* p, long off, int orig);
Glyph pgetc(Port* p);
Glyph pungetc(Port* p, Glyph g);
Glyph ppeekc(Port* p);
int   pprintf(Port* p, char* fmt, ...);
int   pvprintf(Port* p, char* fmt, va_list va);

// initialization -------------------------------------------------------------

#endif
