/* Medium-level Rascal IO facilities. */
// headers --------------------------------------------------------------------
#include "data/types/port.h"
#include "data/types/glyph.h"
#include "data/types/str.h"

#include "sys/error.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
Port* open_port(char* fname, char* mode) {
  FILE* ios = fopen(fname, mode);

  require(ios != NULL, "couldn't open %s: %s", fname, strerror(errno));

  return mk_port(ios);
}

void  close_port(Port* port) {
  if ( port->ios != NULL ) {
    fclose(port->ios);
    port->ios = NULL;
  }
}

// stdio.h wrappers -----------------------------------------------------------
// at some point we hope to create a better
// port implementation using lower-level
// C functions, so it makes sense to create
// these APIs now
bool peof(Port* p) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return feof(p->ios);
}

int pseek(Port* p, long off, int orig) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return fseek(p->ios, off, orig);
}

Glyph pgetc(Port* p) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return fgetc(p->ios);
}

Glyph pungetc(Port* p, Glyph g) {
  // don't call on a closed port
  assert(p->ios != NULL);

  return ungetc(g, p->ios);
}

Glyph ppeekc(Port* p) {
  // don't call on a closed port
  assert(p->ios != NULL);

  int c = fgetc(p->ios);

  if ( c != EOF )
    ungetc(c, p->ios);

  return c;
}

int pprintf(Port* p, char* fmt, ...) {
  // don't call on a closed port
  assert(p->ios != NULL);
  va_list va;
  va_start(va, fmt);
  int o = vfprintf(p->ios, fmt, va);
  va_end(va);
  return o;
}

int pvprintf(Port* p, char* fmt, va_list va) {
  // don't call on a closed port
  assert(p->ios != NULL);

  int o = vfprintf(p->ios, fmt, va);
  return o;
}

// initialization -------------------------------------------------------------
