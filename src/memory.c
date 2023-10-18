#include "util/number.h"

#include "type.h"

#include "vm.h"

#include "memory.h"

// internal GC helpers
static void add_gray(void* ptr) {
  objects_push(&RlVm.heap.grays, ptr);
}

static void mark_frame(Frame* frame) {
  mark(frame->code);
}

static void mark_frames(size_t n, Frame* frames) {
  for (size_t i=0; i<n; i++)
    mark_frame(frames+i);
}

void mark_vals(Value* vs, size_t n) {
  for (size_t i=0; i<n; i++)
    mark(vs[i]);
}

void mark_objs(void** objs, size_t n) {
  for (size_t i=0; i<n; i++)
    mark(objs[i]);
}

void mark_obj(void* p) {
  if (p != NULL) {
    Obj* o = p;

    if (o->black == false) {
      o->black = true;
      mark_obj(o->annot);

      Type* t = type_of(o);

      if (t->v_table->trace)
        add_gray(o);

      else
        o->gray = false;
    }
  }
}

void mark_val(Value val) {
  if (is_obj(val))
    mark_obj(as_obj(val));
}

static void mark_vm(Vm* vm) {
  (void)vm;
}

static void trace_vm(Vm* vm) {
  Objects* grays = &vm->heap.grays;
  Type* t; Obj* o;

  while (grays->cnt > 0) {
    o = objects_pop(grays);
    // assert(obj->gray == false);
    t = type_of(o);
    if (t->v_table->trace)
      t->v_table->trace(o);
    o->gray = false;
  }
}

static void sweep_vm(Vm* vm) {
  Obj** l = &vm->heap.objs, *c = vm->heap.objs, *t;
  Type* tp;

  while (c != NULL) {
    if (c->black) {
      c->black = false;
      c->gray  = true;
      l        = &c->next;
    } else {
      t  = c;
      *l = c->next;
      tp = type_of(t);

      if (tp->v_table->free && !t->no_free)
        tp->v_table->free(t);

      if (!t->no_sweep)
        deallocate(vm, t, size_of(t));
    }

    c  = *l;
  }
}

static void resize_vm(Vm* vm) {
  size_t allocated = vm->heap.used, available = vm->heap.cap;

  if (!vm->initialized) {
    vm->heap.cap <<= 1;
  } else {
    if (allocated >= available) {
      fprintf(stderr, "fatal error: gc couldn't free enough memory.");
      exit(1);
    }
    
    if (allocated > available * HEAP_LF && available < MAX_ARITY) {
      if (available == MAX_POW2)
        vm->heap.cap = MAX_ARITY;
      
      else
        vm->heap.cap <<= 1;
    }
  }
}

static void manage_vm(Vm* vm) {
  if (vm->initialized) {
    mark_vm(vm);
    trace_vm(vm);
    sweep_vm(vm);
  }
  
  resize_vm(vm);
}

static void manage_heap(Vm* vm, size_t n_bytes_added, size_t n_bytes_removed) {
  assert(vm != NULL);

  if (n_bytes_added > n_bytes_removed) {
    size_t diff = n_bytes_added - n_bytes_removed;

    if (diff + vm->heap.used > vm->heap.cap)
      manage_vm(vm);

    vm->heap.used += diff;
  } else {
    size_t diff    = n_bytes_removed - n_bytes_added;
    vm->heap.used -= diff;
  }
}

// external GC helpers
// trace helpers

// external API
void  init_heap(Vm* vm) {
  vm->heap.objs = NULL;
  vm->heap.used = 0;
  vm->heap.cap  = N_HEAP;

  obj_head(&vm->heap.grays, Objects, 0, .no_sweep=true);
  init_objects(&vm->heap.grays);
}

void free_heap(Vm* vm) {
  free_objects(&vm->heap.grays);
  
  // TODO: free live objects
}

void add_to_heap(void* p) {
  assert(p != NULL);

  Obj* o         = p;
  o->next        = RlVm.heap.objs;
  RlVm.heap.objs = o;
}

void* allocate(Vm* vm, size_t n_bytes) {
  if (vm)
    manage_heap(vm, n_bytes, 0);

  void* out = SAFE_MALLOC(n_bytes);
  memset(out, 0, n_bytes);

  return out;
}

void* duplicate(Vm* vm, void* pointer, size_t n_bytes) {
  void* cpy = allocate(vm, n_bytes);

  memcpy(cpy, pointer, n_bytes);

  return cpy;
}

void* reallocate(Vm* vm, void* pointer, size_t old_size, size_t new_size) {
  void* out;
  
  if (new_size == 0) {
    deallocate(vm, pointer, old_size);
    out = NULL;
  }
  
  else {
    if (vm)
      manage_heap(vm, old_size, new_size);

    out = SAFE_REALLOC(pointer, new_size);

    if (new_size > old_size)
      memset(out+old_size, 0, new_size-old_size);
  }
  
  return out;
}

void deallocate(Vm* vm, void* pointer, size_t n_bytes) {
  if (vm)
    manage_heap(vm, 0, n_bytes);

  free(pointer);
}
