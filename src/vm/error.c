#include <stdlib.h>
#include <string.h>

#include "val/value.h"

#include "vm/state.h"
#include "vm/error.h"
#include "vm/environ.h"
#include "vm/heap.h"

/* C types */
#define EBUF_SIZE 2048
#define EBUF_MAX  2047

typedef struct {
  size64 space;          // remaining writable space
  char*  cursor;         // next writable space
  char   buf[EBUF_SIZE]; // underlying buffer
} ErrorBuffer;

/* Globals */
ErrorBuffer Ebuf;

/* Internal APIs */
// miscellaneous helpers
static bool is_fatal(Error e) {
  return e > E_EVAL;
}

static char* ename(Error e) {
  char* out;

  switch ( e ) {
    case E_OKAY: out = "okay";    break;
    case E_USER: out = "user";    break;
    case E_READ: out = "read";    break;
    case E_COMP: out = "compile"; break;
    case E_EVAL: out = "eval";    break;
    case E_RUNT: out = "runtime"; break;
    case E_SYS:  out = "system";  break;
  }

  return out;
}

// error buffer API
static void reset_ebuf(ErrorBuffer* b) {
  memset(b->buf, 0, EBUF_SIZE);
  b->cursor = b->buf;
  b->space  = EBUF_MAX;
}

static void ebuf_vprintf(ErrorBuffer* b, char* t, va_list va) {
  if ( b->space > 0 ) {
    int n = vsnprintf(b->cursor, b->space, t, va);

    if ( n < 0 || b->space + n > EBUF_MAX ) {
      b->space  = EBUF_MAX;
      b->cursor = NULL;
    } else {
      b->space  -= n;
      b->cursor += n;
    }
  }
}

static void ebuf_printf(ErrorBuffer* b, char* t, ...) {
  va_list va;
  va_start(va, t);
  ebuf_vprintf(b, t, va);
  va_end(va);
}

static void ebuf_printh(ErrorBuffer* b, Error e, char* f) {
  reset_ebuf(b);

  if ( is_fatal(e) )
    ebuf_printf(b, "fatal ");

  ebuf_printf(b, "%s error: %s: ", ename(e), f);
}

static char* rl_verror(ErrorBuffer* b, Error e, char* f, char* t, va_list va) {
  ebuf_printh(b, e, f);
  ebuf_vprintf(b, t, va);

  return b->buf;
}

static char* rl_perror(ErrorBuffer* b, Error e, char* f, char* t, ...) {
  va_list va;
  va_start(va, t);
  char* out = rl_verror(b, e, f, t, va);
  va_end(va);
  return out;
}

/* External APIs */
// managing error state
void rl_epush(Proc* p) {
  assert(p->cp < p->catches_end);

  EFrame* e = p->cp++;

  /* Save current state */
  e->hfs  = p->vm->hfs;
  e->code = p->code;
  e->ip   = p->ip;
  e->bp   = p->sp;
  e->fp   = p->fp;
}

void rl_epop(Proc* p, bool r) {
  assert(p->cp > p->catches);

  if ( r ) {
    p->vm->hfs = p->cp->hfs;
    p->code    = p->cp->code;
    p->ip      = p->cp->ip;
    p->bp      = p->cp->bp;
    p->fp      = p->cp->fp;
  }

  p->cp--;
}

// invoking errors
int rl_err(Proc* p, Error e, char* f, char* t, ...) {
  va_list va;
  va_start(va, t);
  rl_verror(&Ebuf, e, f, t, va);
  va_end(va);

  if ( is_fatal(e) ) {
    exit(1);
  } else {
    close_upvs(p, NULL);
    rl_ejmp(p, e); 
  }
}

int rl_require(Proc* p, bool c, Error e, char* f, char* t, ...) {
  if ( c ) {
    va_list va;
    va_start(va, t);
    rl_verror(&Ebuf, e, f, t, va);
    va_end(va);

    if ( is_fatal(e) ) {
      exit(1);
    } else {
      close_upvs(p, NULL);
      rl_ejmp(p, e); 
    }
  }

  return 0;
}

size32 rl_argco(Proc* p, char* f, size32 g, size32 e, bool v) {
  static char* ff = "expected %d arguments to #, got %d";
  static char* vf = "expected at least %d arguments to #, got %d";

  if ( v )
    rl_require(p, g >= e, E_EVAL, f, vf, e, g);

  else
    rl_require(p, g == e, E_EVAL, f, ff, e, g);

  return g;
}

size64 rl_bounds(Proc* p, char* f, size64 o, size64 l, size64 u) {
  static char* t = "index %zu not between %zu and %zu";

  rl_require(p, o >= l && o <= u, E_EVAL, f, t, o, l, u);

  return l;
}

Type rl_argtype(Proc* p, char* f, Type g, Type e) {
  static char* t = "# expected type %s, got %s";

  rl_require(p, e == g, E_EVAL, f, t, tname(e), tname(g));

  return g;
}

Type rl_argtypes(Proc* p, char* f, Type g, size64 n, ...) {
  // This one is such a pain in the ass because of English syntax and because
  // an informative error message depends on the arguments
  static char* t1 = "#expted type %s, got %s";
  static char* t2 = "# expected type %s or %s, got %s";
  
  va_list va;
  va_start(va, n);
  
  if ( n == 1 ) {
    Type e1 = va_arg(va, int);
    va_end(va);
    rl_require(p, g == e1, E_EVAL, f, t1, tname(e1), tname(g));
  } else if ( n == 2 ) {
    Type e1 = va_arg(va, int);
    Type e2 = va_arg(va, int);
    va_end(va);
    rl_require(p, g == e1 || g == e2, E_EVAL, f, t2, tname(e1), tname(e2), tname(g));
  } else {
    ebuf_printh(&Ebuf, E_EVAL, f);

    bool okay = false;

    for ( size64 i=0; !okay && i < n; i++ ) {
      Type e = va_arg(va, int);
      okay   = g == e;

      if ( !okay ) {
        char* t;

        if ( i == 0 )
          t = "# expected type %s, ";

        else if ( i+1 == n )
          t = "or %s, ";

        else
          t = "%s, ";

        ebuf_printf(&Ebuf, t, tname(e));
      }
    }

    va_end(va);

    if ( !okay ) {
      // finish printing error message
      ebuf_printf(&Ebuf, "got %s", tname(g));

      // prepare for jump
      close_upvs(p, NULL);

      // perform jump
      rl_ejmp(p, E_EVAL);
    }
  }

  return g;
}
