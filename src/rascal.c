// standard headers -----------------------------------------------------------

// utility headers ------------------------------------------------------------

// rascal headers -------------------------------------------------------------
#include "memory.h"
#include "runtime.h"

#include "read.h"
#include "prin.h"
#include "compile.h"
#include "eval.h"

#include "atom.h"
#include "list.h"
#include "function.h"
#include "array.h"
#include "port.h"

// globals --------------------------------------------------------------------
// runtime globals ------------------------------------------------------------
uchar *Heap, *Swap;

size_t HeapSize, HeapUsed = 0, SwapUsed = 0;

bool Grow = false, Grew = false, Collecting = false;

gc_frame_t *Saved = NULL, *Catch = NULL;

jmp_buf Toplevel;

// interpreter globals --------------------------------------------------------
table_t  *Symbols, *Namespace;
vector_t *Globals, *Stack, *Values;
object_t *Fn = NULL;
value_t   Val = NIL;
uint      Pc = 0;

// entry point ----------------------------------------------------------------
int main(void)
{
  return 0;
}
