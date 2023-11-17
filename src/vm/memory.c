#include <stdio.h>

#include "util/memory.h"

#include "vm/memory.h"

#include "val/type.h"
#include "val/object.h"

/* Globals */
Objects Grays = {
  .obj={
    .type =&ObjectsType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|NOTRACE,
  },
  .data=NULL,
  .cnt =0,
  .cap =0
};

#define MAX_FREE_LIST_SIZE 256
#define OBJ_ALIGNMENT sizeof(Value)
#define OBJ_ALIGNMENT_MASK (sizeof(Value)-1)
#define MAX_FREE_LIST_OBJ_SIZE (256*OBJ_ALIGNMENT)

Obj* FreeLists[MAX_FREE_LIST_SIZE] = {};

#define N_HEAP  (((size_t)1)<<19)
#define HEAP_LF 0.625

/* Internal API */
static size_t mod_obj_align(size_t n) {
  return n & OBJ_ALIGNMENT_MASK;
}

static size_t pad_obj_size(size_t n) {
  if (n < OBJ_ALIGNMENT)
    n = OBJ_ALIGNMENT;

  size_t extra = mod_obj_align(n);

  if (extra > 0)
    n += OBJ_ALIGNMENT - extra;

  return n;
}

static void add_gray(void* ptr) {
  objects_push(Ctx.h.grays, ptr);
}

static void mark_vm(RlCtx* vm) {
  (void)vm;
}

static void trace_vm(RlCtx* vm) {
  Objects* grays = vm->h.grays;
  Type* t; Obj* o;

  while (grays->cnt > 0) {
    o = objects_pop(grays);
    // assert(obj->gray == false);
    t = type_of(o);
    t->vtable->tracefn(o);
  }
}

static void sweep_vm(RlCtx* vm) {
  Obj** l = &vm->h.objects, *c = vm->h.objects, *t;
  Type* tp;

  while (c != NULL) {
    if (is_black(c)) {
      unmark(c);
      l = &c->next;
    } else {
      t = c;
      *l = c->next;
      tp = type_of(t);

      if (tp->vtable->finalizefn && !is_nofree(t))
        tp->vtable->finalizefn(t);

      if (!is_nosweep(t))
        dealloc_obj(vm, t);
    }

    c  = *l;
  }
}

static void resize_vm(RlCtx* vm) {
  size_t allocated = vm->h.size, available = vm->h.cap;

  /* probably an error */
  if (allocated >= vm->h.cap) {
    fprintf(stderr, "fatal error: gc couldn't free enough memory.");
    exit(1);
  }
    
  if (allocated > available * HEAP_LF && available < MAX_ARITY) {
    if (available == MAX_POW2)
        vm->h.cap = MAX_ARITY;
      
    else
      vm->h.cap <<= 1;
  }
}

static void manage_vm(RlCtx* vm) {
    mark_vm(vm);
    trace_vm(vm);
    sweep_vm(vm);
    resize_vm(vm);
}

static void manage_heap(RlCtx* vm, size_t n_bytes_added, size_t n_bytes_removed) {
  assert(vm != NULL);

  if (n_bytes_added > n_bytes_removed) {
    size_t diff = n_bytes_added - n_bytes_removed;

    if (diff + vm->h.size > vm->h.cap)
      manage_vm(vm);

    vm->h.size += diff;
  } else {
    size_t diff = n_bytes_removed - n_bytes_added;
    vm->h.size -= diff;
  }
}

static void* alloc_from_free_lists(RlCtx* ctx, size_t n_bytes) {
  size_t padded = pad_obj_size(n_bytes);
  void* out = NULL;
  manage_heap(ctx, padded, 0);

  if (padded <= MAX_FREE_LIST_OBJ_SIZE) {
    Obj** free_list = &FreeLists[padded>>3];

    if (*free_list != NULL) {
      out = *free_list;
      *free_list = (*free_list)->next;
    }
  }

  if (out == NULL) {
    out = SAFE_MALLOC(padded);
    memset(out, 0, padded);
  }

  return out;
}

/* External API */
void unsave_gc_frame(GcFrame* frame) {
  Ctx.h.frames = frame->next;
}

void trace_val(Value x) {
  if (is_obj(x))
    trace_obj(as_obj(x));
}

void trace_obj(void* obj) {
  assert(obj);
  type_of(obj)->vtable->tracefn(obj);
}

void mark_val(Value val) {
  if (is_obj(val))
    mark_obj(as_obj(val));
}

void mark_obj(void* p) {
  if (p != NULL) {
    Obj* o = p;

    if (!is_black(o)) {
      set_mfl(o, BLACK);

      /* Trace common metadata pointers. */
      mark(o->type);
      mark(o->meta);

      /* If there's extra work to be done, save to grays stack. */
      if (o->type->vtable->tracefn)
        add_gray(o);

      else
        del_mfl(o, GRAY);
    }
  }
}

void mark_vals(Value* vs, size_t n) {
  for (size_t i=0; i<n; i++)
    mark(vs[i]);
}

void mark_objs(Obj** objs, size_t n) {
  for (size_t i=0; i<n; i++)
    mark(objs[i]);
}

void unmark_val(Value val) {
  if (is_obj(val))
    unmark_obj(as_obj(val));
}

void unmark_obj(void* obj) {
  Obj* o = obj;

  if (o) {
    del_mfl(o, BLACK);
    set_mfl(o, GRAY);
  }
}

void unmark_vals(Value* vals, size_t n) {
  for (size_t i=0; i<n; i++)
    unmark(vals[i]);
}

void unmark_objs(Obj** objs, size_t n) {
  if (objs)
    for (size_t i=0; i<n; i++)
      unmark(objs[i]);
}

// external GC helpers
void add_to_heap(RlCtx* ctx, Obj* o) {
  assert(o != NULL);

  o->next = ctx->h.objects;
  ctx->h.objects = o;
}

void* alloc_obj(RlCtx* ctx, size_t n_bytes) {
  assert(ctx);
  Obj* out = alloc_from_free_lists(ctx, n_bytes);
  out->next = ctx->h.objects;
  ctx->h.objects = out;
  return out;
}

void dealloc_obj(RlCtx* ctx, void* obj) {
  size_t obsize = size_of(obj);

  if (obsize <= MAX_FREE_LIST_OBJ_SIZE) {
    Obj* o = obj;
    Obj** buf = &FreeLists[obsize>>3];
    o->next = *buf;
    *buf = o;
  } else {
    free(obj);
  }

  ctx->h.size -= obsize;
}

void* allocate(RlCtx* vm, size_t n_bytes) {
  if (vm)
    manage_heap(vm, n_bytes, 0);

  void* out = SAFE_MALLOC(n_bytes);
  memset(out, 0, n_bytes);

  return out;
}

// memory API
void* duplicate(RlCtx* vm, void* pointer, size_t n_bytes) {
  void* cpy = allocate(vm, n_bytes);
  memcpy(cpy, pointer, n_bytes);
  return cpy;
}

char* duplicates(RlCtx* vm, char* s, size_t n_chars) {
  char* cpy;
  
  assert(s != NULL);
  
  if (n_chars == 0)
    n_chars = strlen(s);

  cpy = allocate(vm, n_chars+1);
  strcpy(cpy, s);
  return cpy;
}

void* reallocate(RlCtx* vm, void* pointer, size_t old_size, size_t new_size) {
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

void deallocate(RlCtx* vm, void* pointer, size_t n_bytes) {
  if (vm)
    manage_heap(vm, 0, n_bytes);

  free(pointer);
}

/* Initialization */
void vm_init_memory(void) {
  Ctx.h = (Heap) {
    .size   =0,
    .cap    =N_HEAP,
    .frames =NULL,
    .objects=NULL,
    .grays  =&Grays
  };
}
