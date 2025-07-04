#ifndef rl_lang_base_h
#define rl_lang_base_h

/* Globals and common APIs for the Rascal interpreter. */
// headers --------------------------------------------------------------------
#include <stdarg.h>

#include "common.h"

#include "sys/opcode.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
// Special form names (for fast identification)
extern Str* QuoteStr, * DefStr, * PutStr, * IfStr, * DoStr, * FnStr,
  * CatchStr, * ThrowStr;

// Standard streams
extern Port Ins, Outs, Errs;

// Global environment
extern Env Globals;

// Names for special characters (\0, \n, \t, etc)
extern char* CharNames[128];

// function prototypes --------------------------------------------------------
// Rascal IO helpers ----------------------------------------------------------
Port* open_port(char* fname, char* mode);
void  close_port(Port* port);
bool  peof(Port* p);
int   pseek(Port* p, long off, int orig);
Glyph pgetc(Port* p);
Glyph pungetc(Port* p, Glyph g);
Glyph ppeekc(Port* p);
int   pprintf(Port* p, char* fmt, ...);
int   pvprintf(Port* p, char* fmt, va_list va);

// Rascal environment helpers -------------------------------------------------
Ref* env_capture(Env* e, Ref* r);
Ref* env_resolve(Env* e, Sym* n, bool capture);
Ref* env_define(Env* e, Sym* n);

// toplevel environment utilities ---------------------------------------------
void toplevel_env_def(Env* e, Sym* n, Expr x);
void toplevel_env_set(Env* e, Sym* n, Expr x);
void toplevel_env_refset(Env* e, int n, Expr x);
Ref* toplevel_env_find(Env* e, Sym* n);
Expr toplevel_env_ref(Env* e, int n);
Expr toplevel_env_get(Env* e, Sym* n);
void def_builtin_fun(char* name, OpCode op);

// local environment utilities ------------------------------------------------
Expr upval_ref(Fun* fun, int i);
void upval_set(Fun* fun, int i, Expr x);

// miscellaneous utilities ----------------------------------------------------
bool is_falsey(Expr x);
bool is_literal(Expr x);

// initialization -------------------------------------------------------------

#endif
