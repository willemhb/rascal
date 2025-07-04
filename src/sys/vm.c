/**
 *
 * Internal VM API.
 * 
 **/

// headers --------------------------------------------------------------------
#include "data/data.h"

#include "sys/vm.h"
#include "sys/error.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
// vm manipulation ------------------------------------------------------------
Expr* vals_ref(int i) {
  int j = i;

  if ( j < 0 )
    j += Vm.sp;

  if ( j < 0 || j > Vm.sp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Vm.vals[j];
}

void setvp(int n) {
  assert(n > 0 && n <= N_VALS);
  Vm.sp = n;
}

Expr* vpush( Expr x ) {
  if ( Vm.sp == N_VALS )
    runtime_error("stack overflow");

  Vm.vals[Vm.sp] = x;

  return &Vm.vals[Vm.sp++];
}

Expr* vpushn( int n ) {
  if ( Vm.sp + n >= N_VALS )
    runtime_error("stack overflow");

  Expr* base = &Vm.vals[Vm.sp]; Vm.sp += n;

  return base;
}

Expr vpop( void ) {
  if ( Vm.sp == 0 )
    runtime_error("stack underflow");

  return Vm.vals[--Vm.sp];
}

Expr vrpop(void) {
  if ( Vm.sp < 2 )
    runtime_error("stack underflow");

  Expr out = Vm.vals[Vm.sp-2];
  Vm.vals[Vm.sp-2] = Vm.vals[Vm.sp-1];

  Vm.sp--;

  return out;
}

Expr vpopn( int n ) {
  if ( n > Vm.sp )
    runtime_error("stack underflow");

  Expr out = Vm.vals[Vm.sp-1]; Vm.sp -= n;

  return out;
}

Expr* frames_ref(int i) {
  int j = i;

  if ( j < 0 )
    j += Vm.fp;

  if ( j < 0 || j > Vm.fp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Vm.frames[j];
}

void setfp(int n) {
  assert(n > 0 && n <= N_FRAMES);
  Vm.fp = n;
}

Expr* fpush(Expr x) {
  if ( Vm.fp == N_FRAMES )
    runtime_error("stack overflow");

  Vm.frames[Vm.fp] = x;

  return &Vm.frames[Vm.fp++];
}

Expr* fpushn(int n) {
  if ( Vm.fp + n >= N_FRAMES )
    runtime_error("stack overflow");

  Expr* base = &Vm.frames[Vm.fp]; Vm.fp += n;

  return base;
}

Expr fpop(void) {
  if ( Vm.fp == 0 )
    runtime_error("stack underflow");

  return Vm.frames[--Vm.fp];
}

Expr frpop(void) {
  if ( Vm.fp < 2 )
    runtime_error("stack underflow");

  Expr out = Vm.frames[Vm.fp-2];
  Vm.frames[Vm.fp-2] = Vm.frames[Vm.fp-1];

  Vm.fp--;

  return out;
}

Expr fpopn(int n) {
  if ( n > Vm.fp )
    runtime_error("stack underflow");

  Expr out = Vm.frames[Vm.fp-1]; Vm.fp -= n;

  return out;
}

// frame manipulation ---------------------------------------------------------
void install_fun(Fun* fun, int bp) {
  Vm.fn = fun;
  Vm.pc = user_fn_instr(fun);
  Vm.bp = bp;
}

void save_frame(void) {
  // get frame
  Expr* frame = fpushn(FRAME_SIZE);
  
  // save caller state in frame
  frame[0] = tag_obj(Vm.fn);
  frame[1] = tag_ptr(Vm.pc);
  frame[2] = tag_fix(Vm.bp);
}

void restore_frame(void) {
  // restore caller state
  // becomes new sp
  int oldbp   = Vm.bp;

  // get frame to restore
  Expr* frame = frames_ref(Vm.fp-3);

  // get saved values/offsets
  Fun* caller = as_fun(frame[0]);
  instr_t* pc = as_ptr(frame[1]);
  int rtnbp   = as_fix(frame[2]);

  // restore values
  Vm.fn = caller;
  Vm.pc = pc;
  Vm.bp = rtnbp;

  // reset vp/fp
  Vm.sp = oldbp;
}

// upvalues -------------------------------------------------------------------
UpVal* get_upv(Expr* loc) {
  UpVal** spc = &Vm.upvs;
  UpVal* out = NULL;

  while ( out == NULL ) {
    UpVal* upv = *spc;

    if ( upv == NULL || loc < upv->loc )
      *spc = out = mk_upval(upv, loc);

    else if ( upv->loc == loc )
      out = upv;

    else
      spc = &upv->next;
  }

  return out;
}

void close_upvs(Expr* base) {
  UpVal* upv = Vm.upvs;

  while ( upv != NULL && upv->loc > base ) {
    UpVal* tmpu = upv->next;
    Expr tmpv   = *upv->loc;
    upv->next   = NULL;
    upv->closed = true;
    upv->val    = tmpv;
    upv         = tmpu;
  }
}

// initialization -------------------------------------------------------------
void toplevel_init_sys_vm(void) {}
