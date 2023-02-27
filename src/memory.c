#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory.h"
#include "htable.h"
#include "value.h"

// heap -----------------------------------------------------------------------
#define MIN_GRAYS 8

object_t* LiveObjects = NULL;
usize     NHeap = 0xfffff, NUsed = 0;
double    HeapLoad = 0.75;
bool      Collecting = false;

objects_t Grays;

// local helpers --------------------------------------------------------------
static void push_gray(object_t *obj) {
  objects_push(&Grays, obj);
}

static object_t *pop_gray(void) {
  return objects_pop(&Grays);
}

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
void manage_init(void) {
  Collecting = true;
}

void mark_object(void* ptr) {
  object_t* obj = ptr;

  if (obj == NULL)
    return;

  if (obj->black)
    return;

  obj->black = true;

  push_gray(obj);
}

void mark_value(value_t val) {
  if (is_object(val))
    mark_object(as_object(val));
}

void trace_symbol(void* ptr) {
  symbol_t* sym = ptr;

  mark_value(sym->bind);
  mark_object(sym->left);
  mark_object(sym->right);
}

void trace_list(void* ptr) {
  list_t* list = ptr;

  mark_value(list->head);
  mark_object(list->tail);
}

void trace_object(void* ptr) {
  object_t* obj = ptr;

  switch (obj->type) {
    case SYMBOL: trace_symbol(ptr); break;
    case LIST:   trace_list(ptr);   break;
    default: break;
  }

  obj->gray = false;
}

void mark_roots(void) {
  extern symbol_t* SymbolTable;

  mark_object((object_t*)SymbolTable);
}

void trace_grays(void) {
  while (Grays.len > 0) {
    object_t* next_obj = pop_gray();

    trace_object(next_obj);
  }
}

void free_symbol(void* ptr) {
  symbol_t* sym = ptr;

  deallocate(sym->name, strlen(sym->name)+1);
}

void free_object(void* ptr) {
  object_t* obj = ptr;
  usize obsize;

  switch(obj->type) {
    case SYMBOL:
      free_symbol(ptr);
      obsize = sizeof(symbol_t);
      break;

    case LIST:
      obsize = sizeof(list_t);
      break;

    default:
      break;
  }

  deallocate(ptr, obsize);
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
