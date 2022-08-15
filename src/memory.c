#include <string.h>

#include "hashing.h"

#include "memory.h"

#include "value.h"
#include "list.h"
#include "environment.h"
#include "table.h"
#include "types.h"

// internals ------------------------------------------------------------------
bool_t heap_grow(value_t h) { return !!(heap_minor(h) & GROW); }
bool_t heap_grew(value_t h) { return !!(heap_minor(h) & GREW); }

size_t swap_cap(value_t h) {
  size_t limit = heap_cap(h);

  if (heap_grew(h))
    limit /= 2;

  return limit;
}


size_t swap_size(value_t h) {
  size_t out = heap_cap(h);

  if (heap_grew(h))
    out /= 2;

  return out;
}

size_t heap_alloc_size(value_t h) {
  /* calculate the extra space needed for the heap bitmap */
  return heap_cap(h) + heap_cap(h) / sizeof(value_t);
}

uchar_t *space_map(value_t h) { return heap_space(h) + heap_cap(h); }
uchar_t *swap_map(value_t h)  { return heap_swap(h) + swap_size(h); }

bool_t in_space(value_t x) {
  assert(is_allocated(x));
  
  return withinp(opval(x), heap_space(Heap), heap_cap(Heap));
}

bool_t in_swap(value_t x) {
  assert(is_allocated(x));
  
  return withinp(opval(x), heap_swap(Heap), swap_cap(Heap));
}

uchar_t *get_base(value_t x) {
  if (in_space(x))
    return heap_space(Heap);

  return heap_swap(Heap);
}

size_t get_offset(value_t x) {
  return (ouval(x) - get_base(Heap)) / sizeof(value_t);
}

uchar_t *get_map(value_t x) {
  return in_space(x) ? space_map(Heap) : swap_map(Heap);
}

uchar_t get_mflags(value_t x) {
  size_t   offset = get_offset(x);
  uchar_t *map    = get_map(x);
  offset         /= heap_align(Heap);

  return map[offset];
}

uchar_t set_mflags(value_t x, uchar_t fl) {
  size_t   offset = get_offset(x);
  uchar_t *map    = get_map(x);
  uchar_t  out    = map[offset];
  map[offset]    /= fl;

  return out;
}

uchar_t clear_mflags(value_t x, uchar_t fl) {
  size_t   offset = get_offset(x);
  uchar_t *map    = get_map(x);
  uchar_t  out    = map[offset];
  map[offset]    &= ~fl;

  return out;
}

void *next_object(value_t h) { return heap_space(h) + heap_used(h); }

uchar_t *next_map(value_t h) {
  uchar_t *m = space_map(h);
  size_t   o = heap_used(h) / sizeof(value_t);
  
  return   m+o;
}

// utilities ------------------------------------------------------------------
inline bool_t check_heap_overflow(value_t h, size_t n_bytes) {
  return heap_used(h) + n_bytes >= heap_cap(h);
}

static inline bool_t grow_stack_p(void) { return stack_sp(Stack) >= stack_cap(Stack); }
static inline bool_t shrink_stack_p(void) {
  return stack_cap(Stack) > stack_cap_min(Stack) && (stack_cap(Stack) / stack_sp(Stack) > 1);
}

void init_map(uchar_t *m, repr_t r, size_t n) {
  memset(m, DATA, n);
  m[0] = r;
}

void *bump(value_t h, size_t n, repr_t r) {
  void *out     = next_object(h);
  uchar_t *map  = next_map(h);
  heap_used(h) += n;

  memset(out, 0, n);
  init_map(map, r, n / heap_word(h));
  return out;
}

void *allocate(size_t n, repr_t r) {
  if (Allocate[r])
    return Allocate[r](n);

  size_t base_size = BaseSizes[r];
  size_t n_alloc = aligned(base_size+n, heap_align(Heap));

  if (check_heap_overflow(Heap, n_alloc))
    collect_garbage();

  return bump(Heap, n_alloc, r);
}

inline bool_t is_moved(value_t x)     { return (get_mflags(x) & GCMOVED) == GCMOVED; }
inline bool_t is_traversed(value_t x) { return  get_mflags(x) & TRAVERSED;           }
inline bool_t is_black(value_t x)     { return (get_mflags(x) & GCMOVED) == GCBLACK; }
inline bool_t is_white(value_t x)     { return (get_mflags(x) & GCMOVED) == GCWHITE; }
inline bool_t is_gray(value_t x)      { return (get_mflags(x) & GCMOVED) == GCGRAY;  }

inline ot_pred(heap, HEAP)
inline ot_pred(stack, STACK)

static void grow_stack(value_t s) {
  stack_cap(s) *= 2;
  stack_data(s) = realloc_s(stack_data(s), stack_cap(s)*sizeof(value_t));
}

static void shrink_stack(value_t s) {
  stack_cap(s)  /= 2;
  stack_data(s)  = realloc_s(stack_data(s), stack_cap(s)*sizeof(value_t));
}

index_t push(value_t x) {
  index_t out            = stack_sp(Stack)++;
  stack_data(Stack)[out] = x;

  if (grow_stack_p())
    grow_stack(Stack);

  return out;
}

void dup(void) {
  index_t tos = stack_sp(Stack)-1;
  index_t loc = stack_sp(Stack)++;
  
  if (grow_stack_p())
    grow_stack(Stack);
  
  stack_data(Stack)[loc] = stack_data(Stack)[tos];
}

index_t pushn(size_t n) {
  index_t out      = stack_sp(Stack);
  stack_sp(Stack) += n;

  if (grow_stack_p())
    grow_stack(Stack);

  return out;
}

value_t pop(void) {
  assert(stack_sp(Stack) > 0);
  value_t out = stack_data(Stack)[--stack_sp(Stack)];

  if (shrink_stack_p())
    shrink_stack(Stack);

  return out;
}

static void resize_heap(value_t h) {
  heap_minor(h) |= RESIZING;
  
  if (heap_grow(h)) {
    heap_cap(h)   *= 2;
    heap_swap(h)   = realloc_s(heap_swap(h), heap_alloc_size(h));
    heap_minor(h) &= ~GROW;
    heap_minor(h) |= GREW;

  } else if (heap_grew(h)) {
    heap_swap(h)   = realloc_s(heap_swap(h), heap_alloc_size(h));
    heap_minor(h) &= ~(GROW|GREW);
  }

  heap_minor(h) &= ~RESIZING;
}

static void swap_heap(value_t h) {
  uchar_t *tmp      = heap_space(h);
  heap_swap(h)      = tmp;
  heap_used(h)      = 0;
}

void trace_runtime(void) {
  Heap         = relocate(Heap);
  Stack        = relocate(Stack);
  Module       = relocate(Module);
  Modules      = relocate(Modules);
  BuiltinTypes = relocate(BuiltinTypes);
  Symbols      = relocate(Symbols);
}

void trace_globals(void);

void trace_gc_frames(void) {
  gc_frame_t *f = Saved;

  while (f) {
    size_t n        = f->length;
    value_t **saved = f->saved;

    for (size_t i=0; i<n; i++) {
      value_t tmp = *saved[i];
      tmp         = relocate(tmp);
      *saved[i]   = tmp;
    }

    f = f->next;
  }
}

void finalize_swap(void) {}

void collect_garbage(void) {
  heap_stw(Heap)    = true;
  heap_minor(Heap) |= RUNNING;

  resize_heap(Heap);
  swap_heap(Heap);
  trace_runtime();
  trace_globals();
  trace_gc_frames();
  finalize_swap();

  heap_minor(Heap) &= ~RUNNING;
  heap_stw(Heap)    = false;
}

// implementation -------------------------------------------------------------
tag_hash(stack, STACK)
tag_hash(heap, HEAP)

void *heap_allocate(size_t n) {
  size_t capacity      = n * sizeof(cons_t);
  size_t map_size      = n * 2;
  size_t total         = capacity + map_size;
  uchar_t *space       = malloc_s(total);
  uchar_t *swap        = malloc_s(total);
  uchar_t *space_map   = space + capacity;
  uchar_t *swap_map    = swap + capacity;

  heap_t *h      = (heap_t*)space;

  h->tag         = 0xffff;
  h->align       = sizeof(cons_t);
  h->word        = sizeof(value_t);
  h->major       = 0;
  h->minor       = 0;
  h->stw         = false;
  h->cap         = capacity;
  h->used        = sizeof(heap_t);
  h->space       = space;
  h->swap        = swap;

  memset(space_map, UNUSED, map_size);
  memset(swap_map, UNUSED, map_size);

  space_map[0] = REPR_HEAP;
  memset(space_map+1, DATA, 5);

  return h;
}

void *stack_allocate(size_t n) {
  size_t base = aligned(sizeof(stack_t), heap_align(Heap));
  
}

void stack_finalize(value_t x) {
  free_s(stack_data(x));
}

void heap_finalize(value_t x) {
  uchar_t *space = heap_space(x);
  uchar_t *swap  = heap_swap(x);

  free_s(space);
  free_s(swap);
}

value_t stack_relocate(value_t x) {
  size_t   max  = stack_sp(x);
  value_t *data = stack_data(x);
  for (size_t i=0; i < max; i++) {
    value_t tmp = data[i];
    tmp         = relocate(tmp);
    data[i]     = tmp;
  }

  return x;
}

// initialization -------------------------------------------------------------
void memory_init_dispatch(void) {
  BaseSizes[REPR_HEAP]   = sizeof(heap_t);
  Ctypes[REPR_HEAP]      = C_pointer;
  MightCycle[REPR_HEAP]  = false;
  Hash[REPR_HEAP]        = heap_hash;
  Allocate[REPR_HEAP]    = heap_allocate;
  Finalize[REPR_HEAP]    = heap_finalize;

  BaseSizes[REPR_STACK]  = sizeof(stack_t);
  Ctypes[REPR_STACK]     = C_pointer;
  MightCycle[REPR_STACK] = true;
  Relocate[REPR_STACK]   = stack_relocate;
  Hash[REPR_STACK]       = stack_hash;
  Finalize[REPR_STACK]   = stack_finalize;
}

void memory_init_globals(void) {
  heap_t *h = heap_allocate(N_HEAP);
  Heap      = tag_ptr(h, HEAP);
  
}

void memory_init(void) {
  // initialize global objects ------------------------------------------------

  // 
}
