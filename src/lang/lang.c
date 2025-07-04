/* Initialization for the files in the `lang` subdirectory. */

// headers --------------------------------------------------------------------
#include <string.h>

#include "lang/lang.h"

#include "sys/memory.h"

#include "data/types/str.h"
#include "data/types/port.h"
#include "data/types/sym.h"
#include "data/types/env.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
Str* QuoteStr, *DefStr, * PutStr, * IfStr, * DoStr, * FnStr,
  * CatchStr, * ThrowStr;

// static wrapper objects for standard streams
Port Ins = {
  .type    = EXP_PORT,
  .black   = false,
  .gray    = true,
  .nosweep = true
};

Port Outs = {
  .type    = EXP_PORT,
  .black   = false,
  .gray    = true,
  .nosweep = true  
};

Port Errs = {
  .type    = EXP_PORT,
  .black   = false,
  .gray    = true,
  .nosweep = true
};

// Names for special characters (\0, \n, \t, etc)
char* CharNames[128] = {
  ['\0'] = "nul",      ['\n'] = "newline",
  [' ']  = "space",    ['\a'] = "bel",
  ['\t'] = "tab",      ['\r'] = "return",
  ['\f'] = "formfeed", ['\v'] = "vtab",
  ['\b'] = "backspace"
};

// Global environment
Env Globals = {
  .type    = EXP_ENV,
  .black   = false,
  .gray    = true,
  .nosweep = true,

  .parent = NULL,
  .arity  = 0,
  .ncap   = 0,

  .vars = {
    .kvs       = NULL,
    .count     = 0,
    .max_count = 0
  },

  .vals = {
    .vals      = NULL,
    .count     = 0,
    .max_count = 0
  }
};

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------

// initialization -------------------------------------------------------------
static void toplevel_init_standard_streams(void) {
  // add FILE* objects to corresponding standard ports
  Ins.ios  = stdin;
  Outs.ios = stdout;
  Errs.ios = stderr;
}

static void toplevel_init_static_objects(void) {
  // register static global objects in heap
  // so that they get unmarked during the sweep phase
  add_to_heap(&Globals);
  add_to_heap(&Ins);
  add_to_heap(&Outs);
  add_to_heap(&Errs);
}

static void toplevel_init_syntax_names(void) {
  // special forms and other syntactic markers
  QuoteStr = mk_str("quote");
  DefStr   = mk_str("def");
  PutStr   = mk_str("put");
  IfStr    = mk_str("if");
  DoStr    = mk_str("do");
  FnStr    = mk_str("fn");
  CatchStr = mk_str("catch");
  ThrowStr = mk_str("throw");
}

static void toplevel_init_builtins(void) {
  // initialize builtin functions
  def_builtin_fun("+", OP_ADD);
  def_builtin_fun("-", OP_SUB);
  def_builtin_fun("*", OP_MUL);
  def_builtin_fun("/", OP_DIV);
  def_builtin_fun("=", OP_EGAL);
  def_builtin_fun("type", OP_TYPE);
  def_builtin_fun("cons", OP_CONS);
  def_builtin_fun("head", OP_HEAD);
  def_builtin_fun("tail", OP_TAIL);
  def_builtin_fun("nth", OP_NTH);
  def_builtin_fun("*heap-report*", OP_HEAP_REPORT);
  def_builtin_fun("*dis*", OP_DIS);

  // initialize other globals
  toplevel_env_def(&Globals, mk_sym("&ins"), tag_obj(&Ins));
  toplevel_env_def(&Globals, mk_sym("&outs"), tag_obj(&Outs));
  toplevel_env_def(&Globals, mk_sym("&errs"), tag_obj(&Errs));  
}

static void toplevel_init_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_TYPES; i++ ) {
    ExpTypeInfo* info = &Types[i]; strcpy(buffer+1, info->name);
    info->repr        = mk_sym(buffer);
  }
}

void toplevel_init_lang(void) {
  toplevel_init_standard_streams();
  toplevel_init_static_objects();
  toplevel_init_syntax_names();
  toplevel_init_builtins();
  toplevel_init_types();
}
