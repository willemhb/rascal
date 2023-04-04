#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "internal.h"

// heap -----------------------------------------------------------------------
object_t* LiveObjects = NULL;
usize     NHeap = 0xfffff, NUsed = 0;
double    HeapLoad = 0.75;
bool      Collecting = false;

alist_t Grays;

// local helpers --------------------------------------------------------------
static void update_heap(usize n) {
  NUsed += n;
  
  if (NUsed >= NHeap)
    manage();
}

// memory ---------------------------------------------------------------------
void* allocate(usize size) {
  update_heap(size);

  void* out = ALLOC_S(malloc, size);

  memset(out, 0, size);

  return out;
}

void* duplicate(void* ptr, usize size) {
  void* out = allocate(size);

  memcpy(out, ptr, size);

  return out;
}

void* reallocate(void* ptr, usize old, usize new) {
  if (old > new)
    NUsed -= (old-new);

  else
    update_heap(new-old);

  ptr = ALLOC_S(realloc, ptr, new);

  if (new > old)
    memset(ptr+old, 0, new-old);

  return ptr;
}

void deallocate(void* ptr, usize size) {
  NUsed -= size;
  free(ptr);
}

// GC internals ---------------------------------------------------------------
void trace_symbol(void* ptr);
void trace_vector(void* ptr);
void trace_dict(void* ptr);
void trace_set(void* ptr);
void trace_tuple(void* ptr);
void trace_stencil(void* ptr);

void free_symbol(void* ptr);


void trace_vector(void* ptr) {
  vector_t* vec = ptr;

  mark_object(vec->vals);
}

void trace_dict(void* ptr) {
  dict_t* dict = ptr;

  mark_object(dict->map);
  mark_object(dict->vals);
}

void trace_set(void* ptr) {
  set_t* set = ptr;

  mark_object(set->map);
  mark_object(set->vals);
}

void trace_stencil(void* ptr) {
  stencil_t* st = ptr;

  trace_values(stencil_len(st), st->array);
}

void manage_init(void) {
  Collecting = true;
}

void mark_roots(void) {
  extern symbol_t* SymbolTable;

  mark_object((object_t*)SymbolTable);
}

void trace_grays(void) {
  while (Grays.len > 0) {
    object_t* next_obj = objects_pop(&Grays);

    Trace[next_obj->type](next_obj);

    next_obj->gray = false;
  }
}

void free_symbol(void* ptr) {
  symbol_t* sym = ptr;

  deallocate(sym->name, strlen(sym->name)+1);
}

void free_object(void* ptr) {
  object_t* obj = ptr;

  if (Free[obj->type])
    Free[obj->type](obj);

  deallocate(ptr, size_of(obj));
}

void sweep_objects(void) {
  object_t** lives = &LiveObjects;

  while (*lives != NULL) {
    if ((*lives)->black) {
      (*lives)->black = false;
      (*lives)->gray  = true;
      lives           = &(*lives)->next;
    } else {
      object_t* next   = (*lives)->next;
      object_t* tofree = *lives;
      *lives           = next;

      free_object(tofree);
    }
  }
}

void resize_heap(void) {
  while (NUsed >= NHeap * HeapLoad)
      NHeap <<= 1;
}

void manage_cleanup(void) {
  Collecting = false;
}

// GC api ---------------------------------------------------------------------
void manage(void) {
  manage_init();
  mark_roots();
  trace_grays();
  sweep_objects();
  resize_heap();
  manage_cleanup();
}

// initialization -------------------------------------------------------------
void memory_init(void) {
  init_objects(&Grays);
}
