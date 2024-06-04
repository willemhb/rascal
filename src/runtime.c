#include <string.h>
#include <stdlib.h>

#include "runtime.h"
#include "memory.h"
#include "util.h"

/* Globals. */
#define INITIAL_HEAP_SIZE (1 << 19) // 2^16 * sizeof(Value)
#define MAXIMUM_HEAP_SIZE MAX_ARITY
#define STACK_SIZE (1 << 16)
#define HEAP_ALIGNMENT ((size_t)8)
#define HEAP_ALIGNMENT_SHIFT 3
#define OBJECT_CACHE_SIZE ((size_t)1024)
#define MAX_REUSABLE_OBJECT_SIZE (HEAP_ALIGNMENT*OBJECT_CACHE_SIZE)

// array of free lists sorted by size
Object* ObjectCache[OBJECT_CACHE_SIZE] = {};

/* External APIs. */
/* Buffer APIs. */
static rl_status_t overflow(const char* fname) {
  return rl_error(RUNTIME_ERROR, fname, "buffer overflow");
}

static rl_status_t underflow(const char* fname) {
  return rl_error(RUNTIME_ERROR, fname, "buffer underflow");
}

// value buffer
rl_status_t init_value_buffer(ValueBuffer* b, Value*fb, uint32_t sm) {
  b->base  = fb;
  b->fbase = fb;
  b->small = true;
  b->next  = 0;
  b->max   = sm;
  b->smax  = sm;

  return OKAY;
}

rl_status_t free_value_buffer(ValueBuffer* b) {
  rl_status_t out = OKAY;

  if ( !b->small )
    deallocate(b->base, 0, false);

  out = init_value_buffer(b, b->fbase, b->smax);

  return out;
}

rl_status_t grow_value_buffer(ValueBuffer* b, size_t n) {
  rl_status_t out = n < UINT32_MAX ? OKAY : overflow("grow-value-buffer");
  void* tmp;

  if ( unlikely(out == RUNTIME_ERROR) )
    goto end;

  if ( b->small ) {
    allocate(&tmp, n*sizeof(Value), false);

    memcpy(tmp, b->base, b->next*sizeof(Value));

  } else {
    tmp = b->base;
    reallocate(&tmp, b->max*sizeof(Value), n*sizeof(Value), false);
  }

    b->base  = tmp;
    b->small = false;
    b->max   = n;

  end:
    return out;
}

rl_status_t shrink_value_buffer(ValueBuffer* b, size_t n) {
  rl_status_t out = OKAY;
  void* tmp;
  bool small = n <= b->smax;

  if ( small ) {
    n   = b->smax;
    tmp = b->fbase;
    out = deallocate(b->base, 0, false);

    if ( unlikely(out != OKAY) )
      goto end;

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(Value), n*sizeof(Value), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->base  = tmp;
  b->small = small;
  b->max   = n;

 end:
  return out;
}

rl_status_t value_buffer_write(ValueBuffer* b, Value* src, size_t n) {
  rl_status_t out = OKAY;
  size_t news = b->next + n;

  if ( unlikely(news >= b->max) ) {
    size_t m = ceil2(news);
    out      = grow_value_buffer(b, m);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  memcpy(b->base+b->next, src, n*sizeof(Value));
  b->next = news;

 end:
  return out;
}

rl_status_t value_buffer_push(ValueBuffer* b, Value v) {
  rl_status_t out = OKAY;
  
  if ( unlikely(b->next == b->max) ) {
    out = grow_value_buffer(b, b->max >> 1);

    if ( out != OKAY )
      goto end;
  }

  b->base[b->next++] = v;

 end:
  return out;
}

rl_status_t value_buffer_pushn(ValueBuffer* b, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = value_buffer_pushv(b, n, va);
  va_end(va);
  return out;
}

rl_status_t value_buffer_pushv(ValueBuffer* b, size_t n, va_list va) {
  Value tmp[n];

  for ( size_t i=0; i<n; i++ )
    tmp[i] = va_arg(va, Value);

  return value_buffer_write(b, tmp, n);
}

rl_status_t value_buffer_pop(ValueBuffer* b, Value* r) {
  rl_status_t out = b->next > 0 ? OKAY : underflow("value-buffer-pop");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( unlikely(!b->small && b->next-1 < (b->max >> 1)) ) {
    out = shrink_value_buffer(b, b->max >> 1);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  if ( r )
    *r = b->base[--b->next];

  else
    --b->next;

 end:
  return out;
}

rl_status_t value_buffer_popn(ValueBuffer* b, Value* r, bool top, size_t n) {
  rl_status_t out = b->next > n ? OKAY : underflow("value-buffer-popn");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( r ) {
    if ( top )
      *r = b->base[b->next-1];

    else
      *r = b->base[b->next-n];
  }

  if ( unlikely(!b->small && b->next-n < (b->max >> 1)) ) {
    out = shrink_value_buffer(b, ceil2(b->next-n));

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->next -= n;

 end:
  return out;
}

// object buffer
rl_status_t init_object_buffer(ObjectBuffer* b, Object** fb, uint32_t sm) {
  b->base  = fb;
  b->fbase = fb;
  b->small = true;
  b->next  = 0;
  b->max   = sm;
  b->smax  = sm;

  return OKAY;
}

rl_status_t free_object_buffer(ObjectBuffer* b) {
  rl_status_t out = OKAY;

  if ( !b->small ) {
    out = deallocate(b->base, 0, false);

    if ( out != OKAY )
      goto end;
  }

  out = init_object_buffer(b, b->fbase, b->smax);

 end:
  return out;
}

rl_status_t grow_object_buffer(ObjectBuffer* b, size_t n) {
  rl_status_t out = n < UINT32_MAX ? OKAY : overflow("grow-object-buffer");
  void* tmp;

  if ( unlikely(out == RUNTIME_ERROR) )
    goto end;

  if ( b->small ) {
    out = allocate(&tmp, n*sizeof(Object*), false);

    if ( unlikely(out != OKAY) )
      goto end;

    memcpy(tmp, b->base, b->next*sizeof(Object*));

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(Object*), n*sizeof(Object*), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

    b->base  = tmp;
    b->small = false;
    b->max   = n;

  end:
    return out;
}

rl_status_t shrink_object_buffer(ObjectBuffer* b, size_t n) {
  rl_status_t out = OKAY;
  void* tmp;
  bool small = n <= b->smax;

  if ( small ) {
    n   = b->smax;
    tmp = b->fbase;
    out = deallocate(b->base, 0, false);

    if ( unlikely(out != OKAY) )
      goto end;

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(Object*), n*sizeof(Object*), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->base  = tmp;
  b->small = small;
  b->max   = n;

 end:
  return out;
}

rl_status_t object_buffer_write(ObjectBuffer* b, Object** src, size_t n) {
  rl_status_t out = OKAY;
  size_t news = b->next + n;

  if ( unlikely(news >= b->max) ) {
    size_t m = ceil2(news);
    out      = grow_object_buffer(b, m);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  memcpy(b->base+b->next, src, n*sizeof(Object*));
  b->next = news;

 end:
  return out;
}

rl_status_t object_buffer_push(ObjectBuffer* b, Object *v) {
  rl_status_t out = OKAY;
  
  if ( unlikely(b->next == b->max) ) {
    out = grow_object_buffer(b, b->max >> 1);

    if ( out != OKAY )
      goto end;
  }

  b->base[b->next++] = v;

 end:
  return out;
}

rl_status_t object_buffer_pushn(ObjectBuffer* b, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = object_buffer_pushv(b, n, va);
  va_end(va);
  return out;
}

rl_status_t object_buffer_pushv(ObjectBuffer* b, size_t n, va_list va) {
  Object* tmp[n];

  for (size_t i=0; i<n; i++)
    tmp[i] = va_arg(va, Object*);

  return object_buffer_write(b, tmp, n);
}

rl_status_t object_buffer_pop(ObjectBuffer* b, Object** r) {
  rl_status_t out = b->next > 0 ? OKAY : underflow("object-buffer-pop");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( unlikely(!b->small && b->next-1 < (b->max >> 1)) ) {
    out = shrink_object_buffer(b, b->max >> 1);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  if ( r )
    *r = b->base[--b->next];

  else
    --b->next;

 end:
  return out;
}

rl_status_t object_buffer_popn(ObjectBuffer* b, Object** r, bool top, size_t n) {
  rl_status_t out = b->next > n ? OKAY : underflow("object-buffer-popn");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( r ) {
    if ( top )
      *r = b->base[b->next-1];

    else
      *r = b->base[b->next-n];
  }

  if ( unlikely(!b->small && b->next-n < (b->max >> 1)) ) {
    out = shrink_object_buffer(b, ceil2(b->next-n));

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->next -= n;

 end:
  return out;
}

// text buffer
rl_status_t init_text_buffer(TextBuffer* b, char* fb, uint32_t sm) {
  b->base  = fb;
  b->fbase = fb;
  b->small = true;
  b->next  = 0;
  b->max   = sm;
  b->smax  = sm;

  return OKAY;
}

rl_status_t free_text_buffer(TextBuffer* b) {
  rl_status_t out = OKAY;

  if ( !b->small ) {
    out = deallocate(b->base, 0, false);

    if ( out != OKAY )
      goto end;
  }

  out = init_text_buffer(b, b->fbase, b->smax);

 end:
  return out;
}

rl_status_t grow_text_buffer(TextBuffer* b, size_t n) {
  rl_status_t out = n < UINT32_MAX ? OKAY : overflow("grow-text-buffer");
  void* tmp;

  if ( unlikely(out == RUNTIME_ERROR) )
    goto end;

  if ( b->small ) {
    out = allocate(&tmp, n*sizeof(char), false);

    if ( unlikely(out != OKAY) )
      goto end;

    memcpy(tmp, b->base, b->next*sizeof(char));

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(char), n*sizeof(char), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

    b->base  = tmp;
    b->small = false;
    b->max   = n;

  end:
    return out;
}

rl_status_t shrink_text_buffer(TextBuffer* b, size_t n) {
  rl_status_t out = OKAY;
  void* tmp;
  bool small = n <= b->smax;

  if ( small ) {
    n   = b->smax;
    tmp = b->fbase;
    out = deallocate(b->base, 0, false);

    if ( unlikely(out != OKAY) )
      goto end;

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(char), n*sizeof(char), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->base  = tmp;
  b->small = small;
  b->max   = n;

 end:
  return out;
}

rl_status_t text_buffer_write(TextBuffer* b, char* src, size_t n) {
  rl_status_t out = OKAY;
  size_t news = b->next + n + 1;

  if ( unlikely(news >= b->max) ) {
    size_t m = ceil2(news);
    out      = grow_text_buffer(b, m);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  memcpy(b->base+b->next, src, n*sizeof(char));
  b->next = news;

 end:
  return out;
}

rl_status_t text_buffer_push(TextBuffer* b, char v) {
  rl_status_t out = OKAY;
  
  if ( unlikely(b->next+1 == b->max) ) {
    out = grow_text_buffer(b, b->max >> 1);

    if ( out != OKAY )
      goto end;
  }

  b->base[b->next++] = v;

 end:
  return out;
}

rl_status_t text_buffer_pushn(TextBuffer* b, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = text_buffer_pushv(b, n, va);
  va_end(va);
  return out;
}

rl_status_t text_buffer_pushv(TextBuffer* b, size_t n, va_list va) {
  char tmp[n+1] = {};

  for (size_t i=0; i<n; i++)
    tmp[i] = va_arg(va, int);

  return text_buffer_write(b, tmp, n);
}

rl_status_t text_buffer_pop(TextBuffer* b, char* r) {
  rl_status_t out = b->next > 0 ? OKAY : underflow("text-buffer-pop");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( unlikely(!b->small && b->next-1 < (b->max >> 1)) ) {
    out = shrink_text_buffer(b, b->max >> 1);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  if ( r )
    *r = b->base[--b->next];

  else
    --b->next;

 end:
  return out;
}

rl_status_t text_buffer_popn(TextBuffer* b, char* r, bool top, size_t n) {
  rl_status_t out = b->next > n ? OKAY : underflow("text-buffer-popn");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( r ) {
    if ( top )
      *r = b->base[b->next-1];

    else
      *r = b->base[b->next-n];
  }

  if ( unlikely(!b->small && b->next-n < (b->max >> 1)) ) {
    out = shrink_text_buffer(b, ceil2(b->next-n));

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->next -= n;

 end:
  return out;
}

// reader frame buffer
rl_status_t init_rframe_buffer(RFrameBuffer* b, RFrame* fb, uint32_t sm) {
  b->base  = fb;
  b->fbase = fb;
  b->small = true;
  b->next  = 0;
  b->max   = sm;
  b->smax  = sm;

  return OKAY;
}

rl_status_t free_rframe_buffer(RFrameBuffer* b) {
  rl_status_t out = OKAY;

  if ( !b->small ) {
    out = deallocate(b->base, 0, false);

    if ( out != OKAY )
      goto end;
  }

  out = init_rframe_buffer(b, b->fbase, b->smax);

 end:
  return out;
}

rl_status_t grow_rframe_buffer(RFrameBuffer* b, size_t n) {
  rl_status_t out = n < UINT32_MAX ? OKAY : overflow("grow-rframe-buffer");
  void* tmp;

  if ( unlikely(out == RUNTIME_ERROR) )
    goto end;

  if ( b->small ) {
    out = allocate(&tmp, n*sizeof(RFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;

    memcpy(tmp, b->base, b->next*sizeof(RFrame));

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(RFrame), n*sizeof(RFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

    b->base  = tmp;
    b->small = false;
    b->max   = n;

  end:
    return out;
}

rl_status_t shrink_rframe_buffer(RFrameBuffer* b, size_t n) {
  rl_status_t out = OKAY;
  void* tmp;
  bool small = n <= b->smax;

  if ( small ) {
    n   = b->smax;
    tmp = b->fbase;
    out = deallocate(b->base, 0, false);

    if ( unlikely(out != OKAY) )
      goto end;

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(RFrame), n*sizeof(RFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->base  = tmp;
  b->small = small;
  b->max   = n;

 end:
  return out;
}

rl_status_t rframe_buffer_write(RFrameBuffer* b, RFrame* src, size_t n) {
  rl_status_t out = OKAY;
  size_t news = b->next + n;

  if ( unlikely(news >= b->max) ) {
    size_t m = ceil2(news);
    out      = grow_rframe_buffer(b, m);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  memcpy(b->base+b->next, src, n*sizeof(RFrame));
  b->next = news;

 end:
  return out;
}

rl_status_t rframe_buffer_push(RFrameBuffer* b, RFrame v) {
  rl_status_t out = OKAY;
  
  if ( unlikely(b->next == b->max) ) {
    out = grow_rframe_buffer(b, b->max >> 1);

    if ( out != OKAY )
      goto end;
  }

  b->base[b->next++] = v;

 end:
  return out;
}

rl_status_t rframe_buffer_pushn(RFrameBuffer* b, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = rframe_buffer_pushv(b, n, va);
  va_end(va);
  return out;
}

rl_status_t rframe_buffer_pushv(RFrameBuffer* b, size_t n, va_list va) {
  RFrame tmp[n];

  for (size_t i=0; i<n; i++)
    tmp[i] = va_arg(va, RFrame);

  return rframe_buffer_write(b, tmp, n);
}

rl_status_t rframe_buffer_pop(RFrameBuffer* b, RFrame* r) {
  rl_status_t out = b->next > 0 ? OKAY : underflow("rframe-buffer-pop");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( unlikely(!b->small && b->next-1 < (b->max >> 1)) ) {
    out = shrink_rframe_buffer(b, b->max >> 1);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  if ( r )
    *r = b->base[--b->next];

  else
    --b->next;

 end:
  return out;
}

rl_status_t rframe_buffer_popn(RFrameBuffer* b, RFrame* r, bool top, size_t n) {
  rl_status_t out = b->next > n ? OKAY : underflow("rframe-buffer-popn");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( r ) {
    if ( top )
      *r = b->base[b->next-1];

    else
      *r = b->base[b->next-n];
  }

  if ( unlikely(!b->small && b->next-n < (b->max >> 1)) ) {
    out = shrink_rframe_buffer(b, ceil2(b->next-n));

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->next -= n;

 end:
  return out;
}

// compiler frame buffer
rl_status_t init_cframe_buffer(CFrameBuffer* b, CFrame* fb, uint32_t sm) {
  b->base  = fb;
  b->fbase = fb;
  b->small = true;
  b->next  = 0;
  b->max   = sm;
  b->smax  = sm;

  return OKAY;
}

rl_status_t free_cframe_buffer(CFrameBuffer* b) {
  rl_status_t out = OKAY;

  if ( !b->small ) {
    out = deallocate(b->base, 0, false);

    if ( out != OKAY )
      goto end;
  }

  out = init_cframe_buffer(b, b->fbase, b->smax);

 end:
  return out;
}

rl_status_t grow_cframe_buffer(CFrameBuffer* b, size_t n) {
  rl_status_t out = n < UINT32_MAX ? OKAY : overflow("grow-cframe-buffer");
  void* tmp;

  if ( unlikely(out == RUNTIME_ERROR) )
    goto end;

  if ( b->small ) {
    out = allocate(&tmp, n*sizeof(CFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;

    memcpy(tmp, b->base, b->next*sizeof(CFrame));

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(CFrame), n*sizeof(CFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

    b->base  = tmp;
    b->small = false;
    b->max   = n;

  end:
    return out;
}

rl_status_t shrink_cframe_buffer(CFrameBuffer* b, size_t n) {
  rl_status_t out = OKAY;
  void* tmp;
  bool small = n <= b->smax;

  if ( small ) {
    n   = b->smax;
    tmp = b->fbase;
    out = deallocate(b->base, 0, false);

    if ( unlikely(out != OKAY) )
      goto end;

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(CFrame), n*sizeof(CFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->base  = tmp;
  b->small = small;
  b->max   = n;

 end:
  return out;
}

rl_status_t cframe_buffer_write(CFrameBuffer* b, CFrame* src, size_t n) {
  rl_status_t out = OKAY;
  size_t news = b->next + n;

  if ( unlikely(news >= b->max) ) {
    size_t m = ceil2(news);
    out      = grow_cframe_buffer(b, m);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  memcpy(b->base+b->next, src, n*sizeof(CFrame));
  b->next = news;

 end:
  return out;
}

rl_status_t cframe_buffer_push(CFrameBuffer* b, CFrame v) {
  rl_status_t out = OKAY;
  
  if ( unlikely(b->next == b->max) ) {
    out = grow_cframe_buffer(b, b->max >> 1);

    if ( out != OKAY )
      goto end;
  }

  b->base[b->next++] = v;

 end:
  return out;
}

rl_status_t cframe_buffer_pushn(CFrameBuffer* b, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = cframe_buffer_pushv(b, n, va);
  va_end(va);
  return out;
}

rl_status_t cframe_buffer_pushv(CFrameBuffer* b, size_t n, va_list va) {
  CFrame tmp[n];

  for (size_t i=0; i<n; i++)
    tmp[i] = va_arg(va, CFrame);

  return cframe_buffer_write(b, tmp, n);
}

rl_status_t cframe_buffer_pop(CFrameBuffer* b, CFrame* r) {
  rl_status_t out = b->next > 0 ? OKAY : underflow("cframe-buffer-pop");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( unlikely(!b->small && b->next-1 < (b->max >> 1)) ) {
    out = shrink_cframe_buffer(b, b->max >> 1);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  if ( r )
    *r = b->base[--b->next];

  else
    --b->next;

 end:
  return out;
}

rl_status_t cframe_buffer_popn(CFrameBuffer* b, CFrame* r, bool top, size_t n) {
  rl_status_t out = b->next > n ? OKAY : underflow("cframe-buffer-popn");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( r ) {
    if ( top )
      *r = b->base[b->next-1];

    else
      *r = b->base[b->next-n];
  }

  if ( unlikely(!b->small && b->next-n < (b->max >> 1)) ) {
    out = shrink_cframe_buffer(b, ceil2(b->next-n));

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->next -= n;

 end:
  return out;
}

// interpreter frame buffer
rl_status_t init_iframe_buffer(IFrameBuffer* b, IFrame* fb, uint32_t sm) {
  b->base  = fb;
  b->fbase = fb;
  b->small = true;
  b->next  = 0;
  b->max   = sm;
  b->smax  = sm;

  return OKAY;
}

rl_status_t free_iframe_buffer(IFrameBuffer* b) {
  rl_status_t out = OKAY;

  if ( !b->small ) {
    out = deallocate(b->base, 0, false);

    if ( out != OKAY )
      goto end;
  }

  out = init_iframe_buffer(b, b->fbase, b->smax);

 end:
  return out;
}

rl_status_t grow_iframe_buffer(IFrameBuffer* b, size_t n) {
  rl_status_t out = n < UINT32_MAX ? OKAY : overflow("grow-iframe-buffer");
  void* tmp;

  if ( unlikely(out == RUNTIME_ERROR) )
    goto end;

  if ( b->small ) {
    out = allocate(&tmp, n*sizeof(IFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;

    memcpy(tmp, b->base, b->next*sizeof(IFrame));

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(IFrame), n*sizeof(IFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

    b->base  = tmp;
    b->small = false;
    b->max   = n;

  end:
    return out;
}

rl_status_t shrink_iframe_buffer(IFrameBuffer* b, size_t n) {
  rl_status_t out = OKAY;
  void* tmp;
  bool small = n <= b->smax;

  if ( small ) {
    n   = b->smax;
    tmp = b->fbase;
    out = deallocate(b->base, 0, false);

    if ( unlikely(out != OKAY) )
      goto end;

  } else {
    tmp = b->base;
    out = reallocate(&tmp, b->max*sizeof(IFrame), n*sizeof(IFrame), false);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->base  = tmp;
  b->small = small;
  b->max   = n;

 end:
  return out;
}

rl_status_t iframe_buffer_write(IFrameBuffer* b, IFrame* src, size_t n) {
  rl_status_t out = OKAY;
  size_t news = b->next + n;

  if ( unlikely(news >= b->max) ) {
    size_t m = ceil2(news);
    out      = grow_iframe_buffer(b, m);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  memcpy(b->base+b->next, src, n*sizeof(IFrame));
  b->next = news;

 end:
  return out;
}

rl_status_t iframe_buffer_push(IFrameBuffer* b, IFrame v) {
  rl_status_t out = OKAY;
  
  if ( unlikely(b->next == b->max) ) {
    out = grow_iframe_buffer(b, b->max >> 1);

    if ( out != OKAY )
      goto end;
  }

  b->base[b->next++] = v;

 end:
  return out;
}


rl_status_t iframe_buffer_pushn(IFrameBuffer* b, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = iframe_buffer_pushv(b, n, va);
  va_end(va);
  return out;
}


rl_status_t iframe_buffer_pushv(IFrameBuffer* b, size_t n, va_list va) {
  IFrame tmp[n];

  for (size_t i=0; i<n; i++)
    tmp[i] = va_arg(va, IFrame);

  return iframe_buffer_write(b, tmp, n);
}

rl_status_t iframe_buffer_pop(IFrameBuffer* b, IFrame* r) {
  rl_status_t out = b->next > 0 ? OKAY : underflow("iframe-buffer-pop");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( unlikely(!b->small && b->next-1 < (b->max >> 1)) ) {
    out = shrink_iframe_buffer(b, b->max >> 1);

    if ( unlikely(out != OKAY) )
      goto end;
  }

  if ( r )
    *r = b->base[--b->next];

  else
    --b->next;

 end:
  return out;
}

rl_status_t iframe_buffer_popn(IFrameBuffer* b, IFrame* r, bool top, size_t n) {
  rl_status_t out = b->next > n ? OKAY : underflow("iframe-buffer-popn");

  if ( unlikely(out != OKAY) )
    goto end;

  if ( r ) {
    if ( top )
      *r = b->base[b->next-1];

    else
      *r = b->base[b->next-n];
  }

  if ( unlikely(!b->small && b->next-n < (b->max >> 1)) ) {
    out = shrink_iframe_buffer(b, ceil2(b->next-n));

    if ( unlikely(out != OKAY) )
      goto end;
  }

  b->next -= n;

 end:
  return out;
}

/* HState API & Heap APIs */
rl_status_t hstate_push(HState* h, Object* o) {
  return object_buffer_push(&h->grays, o);
}

rl_status_t hstate_pushn(HState* h, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t out = object_buffer_pushv(&h->grays, n, va);
  va_end(va);
  return out;
}

rl_status_t hstate_pop(HState* h, Object** o) {
  return object_buffer_pop(&h->grays, o);
}

rl_status_t hstate_popn(HState* h, Object** o, bool t, size_t n) {
  return object_buffer_popn(&h->grays, o, t, n);
}

// preserving values
void unpreserve(HFrame* frame) {
  Heap.frames = frame->next;
}

// memory management
static inline size_t pad_alloc_size(size_t osize) {
  // incredibly stupid bit twiddling that yields ceiling modulo 8
  return osize + (((~osize & 0x7) + 1) & 0x7);
}

static inline bool checkgc(size_t n) {
  return n + Heap.size >= Heap.max_size;
}



#define try_gc(n, r, f)                                                 \
  do {                                                                  \
    if ( checkgc(n) ) {                                                 \
      r = collect_garbage();                                            \
                                                                        \
      if ( unlikely(r != OKAY) )                                        \
        rl_fatal_error(RUNTIME_ERROR, f, "gargage collection failed");  \
    }                                                                   \
  } while (false)

#define try_obj_cache(n, s)                     \
  do {                                          \
    if ( n <= MAX_REUSABLE_OBJECT_SIZE ) {      \
      size_t w = n >> HEAP_ALIGNMENT_SHIFT;     \
      if ( ObjectCache[w] ) {                   \
        s = ObjectCache[w];                     \
        ObjectCache[w] = ObjectCache[w]->next;  \
      }                                         \
    }                                           \
  } while (false)

rl_status_t allocate(void** b, size_t n, bool h) {
  rl_status_t r = OKAY;
  void* s = NULL;

  if ( h ) {
    n = pad_alloc_size(n);

    try_gc(n, r, "allocate");
    try_obj_cache(n, s);
  }

  if ( s == NULL )
    rl_malloc(&s, n, 0);

  *b = s;
  return r;
}

rl_status_t reallocate(void** b, size_t p, size_t n, bool h);
rl_status_t duplicate(const void* ptr, void** buf, size_t n_bytes, bool use_heap);
rl_status_t deallocate(void* ptr, size_t n_bytes, bool use_heap);

rl_status_t collect_garbage(void) {
  rl_status_t r = OKAY;

  if ( (r=smark(&Heap)) != OKAY )
    goto end;

  if ( (r=smark(&Reader)) != OKAY )
    goto end;

  if ( (r=smark(&Compiler)) != OKAY )
    goto end;

  if ( (r=smark(&Interpreter)) != OKAY )
    goto end;

  if ( (r=mark(&Globals)) != OKAY )
    goto end;

  if ( (r=mark(&Modules)) != OKAY )
    goto end;

  if ( (r=mark(&Strings)) != OKAY )
    goto end;

  if ( (r=mark(&Unions)) != OKAY )
    goto end;

  if ( (r=mark(&BaseRt)) != OKAY )
    goto end;

  while ( r == OKAY && Heap.grays.next > 0 ) {
    Object* ob;

    spop(&Heap, &ob);
    r = trace(ob);

    ob->gray = false;
  }

  if ( r == OKAY ) { // cleanup
    
  }

 end:
  return r;
}

/* RState and reader APIs */
rl_status_t rstate_push(RState* s, Value v) {
  return value_buffer_push(&s->stack, v);
}

rl_status_t rstate_write(RState* s, Value* vs, size_t n) {
  return value_buffer_write(&s->stack, vs, n);
}

rl_status_t rstate_pushn(RState* s, size_t n, ...) {
  va_list va;
  va_start(va, n);
  rl_status_t r = value_buffer_pushv(&s->stack, n, va);
  va_end(va);
  return r;
}

rl_status_t rstate_pushf(RState* s, Port* i, ReadTable* rt, Gensyms* gs) {
  RFrame frame = {
    .input=s->input,
    .rt   =s->rt,
    .gs   =s->gs
  };

  s->input=i;
  s->rt   =rt;
  s->gs   =gs;

  return rframe_buffer_push(&s->frames, frame);
}

rl_status_t rstate_writef(RState* s, RFrame* f, size_t n) {
  return rframe_buffer_write(&s->frames, f, n);
}

rl_status_t rstate_popf(RState* s) {
  RFrame buf;

  rl_status_t r = rframe_buffer_pop(&s->frames, &buf);

  if ( r == OKAY ) {
    s->input = buf.input;
    s->rt    = buf.rt;
    s->gs    = buf.gs;
  }

  return r;
}

rl_status_t rstate_writec(RState* s, char c) {
  return text_buffer_push(&s->buffer, c);
}

rl_status_t rstate_writecs(RState* s, char* cs, size_t n) {
  return text_buffer_write(&s->buffer, cs, n);
}

rl_status_t rstate_pop(RState* s, Value* b) {
  return value_buffer_pop(&s->stack, b);
}

rl_status_t rstate_popn(RState* s, Value* b, bool t, size_t n) {
  return value_buffer_popn(&s->stack, b, t, n);
}
