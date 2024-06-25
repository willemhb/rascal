#include <string.h>

#include "error.h"

#include "util/stack.h"

#include "val/object.h"

#include "vm/interpreter.h"

/* ValStack API */
void init_vs(ValStack* s, Val* b, Val* e) {
  s->base = b;
  s->end  = e;
  s->next = s->base;
}

void reset_vs(ValStack* s) {
  s->next = s->base;
}

void trace_vs(ValStack* s) {
  for ( Val* b = s->base; b < s->next; b++ )
    mark(*b);
}

Val* vs_push_u(ValStack* s, Val x) {
  Val* o;

  o  = s->next++;
  *o = x;

  return o;
}

rl_err_t vs_push_s(const char* fn, Val** r, ValStack* s, Val x) {
  rl_err_t o = chk_ubound(RUNTIME_ERROR, fn, s->next+1, s->end);

  if ( o == OKAY ) {
    Val* pr = vs_push(s, x);

    if ( r )
      *r = pr;
  }

  return o;
}

Val* vs_write_u(ValStack* s, Val* x, size_t n) {
  Val* o   = s->next;
  s->next += n;

  if ( x != NULL ) // might want to reserve space without writing yet
    memcpy(o, x, n*sizeof(Val));

  return o;
}

rl_err_t vs_write_s(const char* fn, Val** r, ValStack* s, Val* x, size_t n) {
  rl_err_t o = chk_ubound(RUNTIME_ERROR, fn, s->next+n, s->end);

  if ( o == OKAY ) {
    Val* wr = vs_write_u(s, x, n);

    if ( r )
      *r = wr;
  }

  return o;
}

Val* vs_pushn_u(ValStack* s, size_t n, ...) {
  va_list va;
  va_start(va, n);
  Val* r = vs_pushv_u(s, n, va);
  va_end(va);
  return r;
}

rl_err_t vs_pushn_s(const char* fn, ValStack* s, Val** r, size_t n, ...) {
  rl_err_t o = chk_ubound(RUNTIME_ERROR, fn, s->next+n, s->end);

  if ( o == OKAY ) {
    va_list va;
    va_start(va, n);
    Val* pr = vs_pushv_u(s, n, va);
    va_end(va);

    if ( r )
      *r = pr;
  }

  return o;
}

Val* vs_pushv_u(ValStack* s, size_t n, va_list va) {
  Val b[n];

  for ( size_t i=0; i<n; i++ )
    b[i] = va_arg(va, Val);

  return vs_write_u(s, b, n);
}

rl_err_t vs_pushv_s(const char* fn, Val** r, ValStack* s, size_t n, va_list va) {
  rl_err_t o = chk_ubound(RUNTIME_ERROR, fn, s->next+n, s->end);

  if ( o == OKAY ) {
    Val* pr = vs_pushv_u(s, n, va);

    if ( r )
      *r = pr;
  }

  return o;
}

Val vs_pop_u(ValStack* s) {
  return *(--s->next);
}

rl_err_t vs_pop_s(const char* fn, Val* r, ValStack* s) {
  rl_err_t o = chk_lbound(RUNTIME_ERROR, fn, s->next-1, s->base);

  if ( o == OKAY ) {
    Val pr = vs_pop_u(s);

    if ( r )
      *r = pr;
  }

  return o;
}

Val vs_popn_u(ValStack* s, size_t n, bool e) {
  Val r    = *(e ? s->next-1 : s->next-n);
  s->next -= n;

  return r;
}

rl_err_t vs_popn_s(const char* fn, Val* r, ValStack* s, size_t n, bool e) {
  rl_err_t o = chk_lbound(RUNTIME_ERROR, fn, s->next-n, s->base);

  if ( o == OKAY ) {
    Val pr = vs_popn_u(s, n, e);

    if ( r )
      *r = pr;
  }

  return o;
}

Val vs_popr_u(ValStack* s, size_t n) {
  if ( n == 1 )
    return vs_pop_u(s);

  Val o = *(s->next-n);

  memmove(s->next-n, s->next-n-1, n-1*sizeof(Val));
  s->next--;

  return o;
}

rl_err_t vs_popr_s(const char* fn, Val* r, ValStack* s, size_t n) {
  rl_err_t o = chk_lbound(RUNTIME_ERROR, fn, s->next-n, s->base);

  if ( o == OKAY ) {
    Val pr = vs_popr_u(s, n);

    if ( r )
      *r = pr;
  }

  return o;
}

/* IfStack API */
void init_ifs(IfStack* s, IFrame* b, IFrame* e) {
  s->base = b;
  s->end  = e;
  s->next = s->base;
}

void reset_ifs(IfStack* s) {
  s->next = s->base;
}

void trace_ifs(IfStack* s) {
  for ( IFrame* f=s->base; f < s->end; f++ )
    mark_iframe(f);
}

IFrame* ifs_push_u(IfStack* s, IFrame f) {
  IFrame* r;

  r  = s->next++;
  *r = f;

  return r;
}

rl_err_t ifs_push_s(const char* fn, IFrame** r, IfStack* s, IFrame f) {
  rl_err_t o = chk_ubound(RUNTIME_ERROR, fn, s->next+1, s->end);

  if ( o == OKAY ) {
    IFrame* pr = ifs_push_u(s, f);

    if ( r )
      *r = pr;
  }

  return o;
}

IFrame* ifs_write_u(IfStack* s, IFrame* f, size_t n) {
  IFrame* o;

  o        = s->next;
  s->next += n;

  if ( f )
    memcpy(o, f, n * sizeof(IFrame));

  return o;
}

rl_err_t ifs_write_s(const char* fn, IFrame** r, IfStack* s, IFrame* f, size_t n) {
  rl_err_t o = chk_ubound(RUNTIME_ERROR, fn, s->next+n, s->end);

  if ( o == OKAY ) {
    IFrame* wr = ifs_write_u(s, f, n);

    if ( r )
      *r = wr;
  }

  return o;
}

IFrame ifs_pop_u(IfStack* s) {
  return *(--s->next);
}

rl_err_t ifs_pop_s(const char* fn, IFrame* r, IfStack* s) {
  rl_err_t o = chk_lbound(RUNTIME_ERROR, fn, s->next-1, s->base);

  if ( o == OKAY ) {
    IFrame pr = ifs_pop_u(s);

    if ( r )
      *r = pr;
  }

  return o;
}

IFrame ifs_popn_u(IfStack* s, size_t n, bool e) {
  IFrame o = *(e ? s->next-1 : s->next-n);
  s->next -= n;

  return o;
}

rl_err_t ifs_popn_s(const char* fn, IFrame* r, IfStack* s, size_t n, bool e) {
  rl_err_t o = chk_lbound(RUNTIME_ERROR, fn, s->next-n, s->base);

  if ( o == OKAY ) {
    IFrame pr = ifs_popn_u(s, n, e);

    if ( r )
      *r = pr;
  }

  return o;
}
