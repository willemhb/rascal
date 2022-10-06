// system includes
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

// toplevel includes
#include "types.h"
#include "val.h"
#include "obj.h"

// object includes
#include "array.h"
#include "pair.h"
#include "atom.h"
#include "envt.h"
#include "port.h"
#include "number.h"
#include "func.h"
#include "method.h"

// runtime includes
#include "memory.h"
#include "read.h"
#include "prin.h"
#include "exec.h"

// utility includes
#include "utils.h"

// globals
// VM singletons
symt_t  Symbols;
heap_t  Heap;
readt_t Reader;
envt_t  Toplevel;
typet_t Types;

// standard streams
port_t Ins, Outs, Errs;

// famous symbols
val_t Quote, Error;

// dispatch tables
const char  *TypeNames[N_TYPES] =
  {
    [REAL]    = "real",        [CHRTYPE] = "chr",   [NULTYPE] = "nul",
    [ATOM]    = "atom",        [SYMT]    = "symt",  [READT]   = "readt",
    [RENTRY]  = "readt-entry", [PORT]    = "port",  [CONS]    = "cons",
    [ALIST]   = "alist",       [STACK]   = "stack", [ENVT]    = "envt",
    [VAR]     = "var",         [CODE]    = "code",  [INSTR]   = "instr",
    [VM]      = "vm",

    [ANY]     = "any",         [NONE]    = "none",
  };

printer_fn_t Print[N_TYPES] =
  {
   [REAL] = prin_real, [ATOM]   = prin_atom, [CHRTYPE] = prin_chr,
   [CONS] = prin_list, [NULTYPE]= prin_list,
  };

const size_t MinCs[N_TYPES] =
  {
    [ALIST]  = 8,   [SYMT]  = 256, [READT] = 256,
    [BUFFER] = 128, [STACK] = 64,  [ENVT]  = 8,
    [INSTR]  = 64,
  };

const size_t BaseSizes[N_TYPES] =
  {
   [REAL]  =sizeof(real_t),  [CHRTYPE]=sizeof(char_t),

   [CONS]  =sizeof(cons_t),
   [ATOM]  =sizeof(atom_t),  [SYMT]   =sizeof(symt_t),   [ENVT]  =sizeof(envt_t),
   [READT] =sizeof(readt_t), [RENTRY] =sizeof(rentry_t), [PORT]  =sizeof(port_t),
   [ALIST] =sizeof(alist_t), [STACK]  =sizeof(stack_t),  [BUFFER]=sizeof(buffer_t),
  };

mark_fn_t Mark[N_TYPES] =
  {
    [READT] = mark_readt, [SYMT]  = mark_symt,
    [STACK] = mark_stack, [ALIST] = mark_alist,
    [PORT]  = mark_port,  [CONS]  = mark_cons,
  };

free_fn_t Free[N_TYPES] =
  {
   [ATOM]  =free_atom,   [PORT]  =free_port,
   [SYMT]  =free_symt,   [READT] =free_readt,
   [STACK] =free_stack,  [ALIST] =free_alist,
   [BUFFER]=free_buffer,
  };

// initialization and some utilities

// reader
#define VERSION "%d.%d.%d.%c"
#define MAJOR   0
#define MINOR   1
#define PATCH   1
#define DEV     'a'

static void prin_welcome( void )
{
  printf("Welcome to rascal version "VERSION"!\n\n", MAJOR, MINOR, PATCH, DEV );
}

static void init_rascal( void )
{
  memory_init();
  atom_init();
  read_init();
  port_init();
  envt_init();
  exec_init();
  method_init();
}

int main(const int argc, const char *argv[argc])
{
  (void)argv;

  init_rascal();
  prin_welcome();
  lisp_repl();

  return 0;
}
