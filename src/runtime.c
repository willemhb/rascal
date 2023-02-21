#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "runtime.h"
#include "object.h"

#include "util/memory.h"
#include "util/string.h"

// globals --------------------------------------------------------------------
#define N_HEAP  UINT16_MAX
#define N_STACK UINT16_MAX
#define N_FRAME 2048

Val   StackBuffer[N_STACK];
Frame FrameBuffer[N_FRAME];

struct Vm Vm = {
  .heap  = (struct HeapData) {
    .allocated=0,
    .next_gc =N_HEAP,
    .live    =NULL,
    .grays   ={ .count=0, .cap=0, .array=NULL }
  },

  .errors= (struct ErrorData) {
    .error=NO_ERROR
  },

  .reader= (struct ReaderData) {
    .token=READY_TOKEN,
    .saved={ .count=0, .cap=0, .array=NULL }
  },

  .interpreter= (struct InterpreterData) {
    .sp=StackBuffer,
    .fp=FrameBuffer
  }
};

// API ------------------------------------------------------------------------
// heap -----------------------------------------------------------------------
static void check_manage(uint n, usize obsize) {
  if (Heap.allocated + n*obsize <= Heap.next_gc)
    manage();
}

static void initialize_memory(void* spc, uint n, usize obsize, uint64 ini) {

  switch (obsize) {
    case 1:  mem8set(spc, ini, n); break;
    case 2:  mem16set(spc, ini, n); break;
    case 4:  mem32set(spc, ini, n); break;
    case 8:  mem64set(spc, ini, n); break;
    default: memxxset(spc, (void*)ini, n, obsize); break;
  }
}

void* allocate(uint n, usize obsize, uint64 ini) {
  check_manage(n, obsize);

  void* out = calloc_s(n, obsize);
  initialize_memory(out, n, obsize, ini);

  return out;
}

void* reallocate(void* ptr, uint new, uint old, usize obsize, uint64 ini) {
  void* out; uint diff;
  
  if (new < old) {
    diff            = old - new;
    Heap.allocated -= diff * obsize;
    out             = crealloc_s(ptr, new, obsize);
  } else {
    diff = old - new;
    
    check_manage(diff, obsize);
    
    Heap.allocated += diff * obsize;
    out             = crealloc_s(ptr, new, obsize);
    
    initialize_memory(out + old*obsize, diff, obsize, ini);
  }

  return out;
}

void deallocate(void* ptr, uint n, usize obsize) {
  Heap.allocated -= n * obsize;

  free(ptr);
}

void manage(void) {
  Heap.next_gc >>= 1;
}

// error ----------------------------------------------------------------------
static const char* ErrorNames[] = { "no-error", "read-error", "compile-error", "eval-error", "apply-error" };
bool panicking(void) {
  return !!Errors.error;
}

void panic(Error error, char* fmt, ...) {
  if (panicking())
    return;

  Errors.error = error;

  va_list va; va_start(va, fmt); char *msg = vstrfmt(fmt, va); va_end(va);

  bin_write(&Errors.buffer, strlen(msg), msg); free(msg);
}

Error recover(void) {
  Error out = Errors.error;

  if (out) {
    fprintf(stderr, "%s: %s.\n", ErrorNames[out], (char*)Errors.buffer.array);

    resize_bin(&Errors.buffer, 0);
    Errors.error = NO_ERROR;
  }

  return out;
}

// interpreter ----------------------------------------------------------------
Val* push(Val x) {
  assert(Vm.interpreter.sp < StackBuffer+N_STACK);

  Val* out = Vm.interpreter.sp++;
  *out     = x;

  return out;
}

Val pop(void) {
  assert(Vm.interpreter.sp > StackBuffer);

  return *(--Vm.interpreter.sp);
}

Val* peep(int i) {
  Val* loc = (i < 0 ? Vm.interpreter.sp : StackBuffer)+i;

  assert(loc >= StackBuffer && loc < StackBuffer+N_STACK);

  return loc;
}

Val peek(int i) {
  return *peep(i);
}

Frame* pushf(Func* func, uint n) {
  assert(Vm.interpreter.fp < FrameBuffer+N_FRAME);
  assert(Vm.interpreter.sp-n > StackBuffer);

  Frame* out = Vm.interpreter.fp++;

  out->func     = func;
  out->ip       = ((Chunk*)func->func)->code->array;
  out->slots    = Vm.interpreter.sp-n;
  out->captured = false;

  return out;
}

Val popf(void) {
  Val out = peek(-1);

  Vm.interpreter.sp = (--Vm.interpreter.fp)->slots;

  return out;
}

// initialization -------------------------------------------------------------
void runtime_init(void) {
  // initialize heap state ----------------------------------------------------
  init_objs(&Heap.grays);

  // initialize error state ---------------------------------------------------
  init_bin(&Errors.buffer, ENCODED|NOFREE, 0, NULL);
  
  // initialize reader state --------------------------------------------------
  init_bin(&Reader.buffer, ENCODED|NOFREE, 0, NULL);
  init_table(&Reader.table, NOFREE, 0, NULL);

  // initialize interpreter state ----------------------------------------------
  init_table(&Interpreter.globals, NOFREE, 0, NULL);
}
