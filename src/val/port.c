#include <string.h>

#include "val/port.h"
#include "vm.h"
#include "util/util.h"

// forward declarations
void free_port(RlState* rls, void* ptr);

// Type object
Type PortType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_PORT,
  .obsize   = sizeof(Port),
  .free_fn  = free_port
};

// port API
static IOMode get_io_mode(char* fname, char* mode) {
  // infer the right flags from the file name and the C style mode
  IOMode out = 0;

  if ( endswith(fname, ".rl") )
    out |= TEXT_PORT | LISP_PORT;

  else if ( strchr(mode, 'b') )
    out |= BINARY_PORT;

  else
    out |= TEXT_PORT;

  if ( strchr(mode, 'r') )
    out |= INPUT_PORT;

  else if ( strpbrk(mode, "aw") )
    out |= OUTPUT_PORT;

  else
    out |= INPUT_PORT;

  return out;
}

Port* mk_port(RlState* rls, FILE* ios, IOMode io_mode) {
  Port* p = mk_obj(rls, &PortType, 0);
  p->ios  = ios;
  p->mode = io_mode;

  return p;
}

Port* mk_port_s(RlState* rls, FILE* ios, IOMode io_mode) {
  Port* out = mk_port(rls, ios, io_mode);
  stack_push(rls, tag_obj(out));

  return out;
}

Port* open_port(RlState* rls, char* fname, char* mode) {
  FILE* ios = fopen(fname, mode);

  require(rls, ios != NULL, "couldn't open %s: %s.", fname, strerror(errno));

  IOMode io_mode = get_io_mode(fname, mode);

  return mk_port(rls, ios, io_mode);
}

Port* open_port_s(RlState* rls, char* fname, char* mode) {
  Port* out = open_port(rls, fname, mode);
  stack_push(rls, tag_obj(out));

  return out;
}

void  close_port(Port* port) {
  if ( port->ios != NULL ) {
    fclose(port->ios);
    port->ios = NULL;
  }
}

Port* tmp_port(RlState* rls) {
  FILE* ios = tmpfile();
  Port* out = mk_port(rls, ios, INPUT_PORT | OUTPUT_PORT | LISP_PORT | TEXT_PORT);
  return out;
}

Port* tmp_port_s(RlState* rls) {
  Port* out = tmp_port(rls);
  stack_push(rls, tag_obj(out));

  return out;
}

// stdio.h wrappers
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

void prewind(Port* p) {
  assert(p->ios != NULL);
  rewind(p->ios);
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

char* pgets(char* buf, size_t bsize, Port* p) {
  assert(p->ios != NULL);

  return fgets(buf, bsize, p->ios);
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

void clear_port(Port* in) {
  int c;

  while ((c=pgetc(in)) != '\n' && c != EOF);
}

void port_newline(Port* out, int n) {
  for ( int i=0; i<n; i++ )
    pprintf(out, "\n");
}

// lifetime methods
void free_port(RlState* rls, void* ptr) {
  (void)rls;
  Port* p = ptr;

  close_port(p);
}
