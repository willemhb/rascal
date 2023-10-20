#include "util/io.h"
#include "util/number.h"
#include "util/hashing.h"

#include "environment.h"
#include "type.h"
#include "equal.h"

#include "runtime.h"

// external API
void save_state(Context* ctx) {
  assert(ctx != NULL);

  ctx->next    = RlVm.m.ctx;
  ctx->g_frame = RlVm.h.frame;
  ctx->r_frame = RlVm.r.frame;
  ctx->c_frame = RlVm.c.frame;
  ctx->e_frame = RlVm.e.frame;
}

void restore_state(Context* ctx) {
  assert(ctx != NULL);
  
  RlVm.m.ctx   = ctx;
  RlVm.h.frame = ctx->g_frame;
  RlVm.r.frame = ctx->r_frame;
  RlVm.c.frame = ctx->c_frame;
  RlVm.e.frame = ctx->e_frame;
}

static void vprint_error(const char* fname, const char* fmt, va_list va) {
  fprintf(stderr, "error in %s: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
}

static void print_error(const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprint_error(fname, fmt, va);
  va_end(va);
}

static void rl_longjmp(int status) {
  if (RlVm.m.ctx == NULL) {
    fprintf(stderr, "Exiting due to unhandled error.\n");
    exit(1);
  } else {
    longjmp(RlVm.m.ctx->buf, status);
  }
}

#define SIGNAL_ERROR(fname, fmt)                \
  do {                                          \
    va_list _va;                                \
    va_start(_va, fmt);                         \
    vprint_error(fname, fmt, _va);              \
    va_end(_va);                                \
    rl_longjmp(1);                              \
  } while (false)

void error(const char* fname, const char* fmt, ...) {
  SIGNAL_ERROR(fname, fmt);
}

bool require(bool test, const char* fname, const char* fmt, ...) {
  if (!test)
    SIGNAL_ERROR(fname, fmt);

  return test;
}


size_t argco(size_t expected, size_t got, const char* fname) {
  static const char* fmt = "expected exactly %zu arguments, got %zu";
  require(expected == got, fname, fmt, expected, got);
  return got;
}

size_t vargco(size_t expected, size_t got, const char* fname) {
  static const char* fmt = "expected at least %zu arguments, got %zu";
  require(got >= expected, fname, fmt, expected, got);
  return got;
}

Type* argtype(Type* expect, Value got, const char* fname) {
  static const char* fmt = "expected value of type %s, got value of type %s";
  Type* got_type;

  got_type = type_of(got);
  require(is_instance(expect, got_type),
          fmt, fname, expect->name->name, got_type->name->name);
  return got_type;
}

Type* argtypes(size_t expected, Value got, const char* fname, ...) {
  static const char* fmt1 = "expected value of type %s, got value of type %s";
  static const char* fmt2 = "expected value of type %s or %s, got value of type %s";
  static const char* fmt3 = "expected value of type %s, %s, or %s, got value of type %s";

  va_list va;
  bool okay;
  FILE* tmp;
  Type* tg, *tx, *ty, *tz;

  va_start(va, fname);
  okay = true;
  tg = type_of(got);

  if (expected == 1) {
    tx = va_arg(va, Type*);
    va_end(va);
    require(is_instance(tg, tx), fname, fmt1, tx->name->name, tg->name->name);
  } else if (expected == 2) {
    tx = va_arg(va, Type*);
    ty = va_arg(va, Type*);
    va_end(va);
    require(is_instance(tg, tx) || is_instance(tg, ty), fname, fmt2,
            tx->name->name, ty->name->name, tg->name->name);
  } else if (expected == 3) {
    tx = va_arg(va, Type*);
    ty = va_arg(va, Type*);
    tz = va_arg(va, Type*);
    va_end(va);
    require(is_instance(tg, tx) || is_instance(tg, ty) || is_instance(tg, tz),
            fname, fmt3, tx->name->name, ty->name->name,
            tz->name->name, tg->name->name);
  } else {
    tmp = tmpfile();
    okay = false;

    assert(tmp != NULL);

    for (size_t i=0; !okay && i<expected; i++) {
      tx   = va_arg(va, Type*);
      okay = is_instance(tg, tx);

      if (!okay) {
        if (i == 0)
          fprintf(tmp, "expected a value of type %s", tx->name->name);

        else if (i + 1 < expected)
          fprintf(tmp, ", %s", tx->name->name);

        else
          fprintf(tmp, ", or %s, got a value of type %s",
                  tx->name->name, tg->name->name);
      }
    }

    if (!okay) {
      char* msg = read_file("<tmp>", tmp);
      print_error(fname, msg);
      deallocate(NULL, msg, 0);
      rl_longjmp(1);
    }
  }

  return tg;
}

size_t argcos(size_t expected, size_t got, const char* fname, ...) {
  static const char* fmt1 = "expected %zu arguments, got %zu";
  static const char* fmt2 = "expected %zu or %zu arguments, got %zu";
  static const char* fmt3 = "expected %zu, %zu, or %zu arguments, got %zu";

  va_list va;
  bool okay;
  FILE* tmp;
  size_t ex, ey, ez;

  va_start(va, fname);
  okay = true;

  if (expected == 1) {
    ex = va_arg(va, size_t);
    va_end(va);
    require(got == ex, fname, fmt1, ex, got);
  } else if (expected == 2) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got == ey, fname, fmt2, ex, ey, got);
  } else if (expected == 3) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    ez = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got == ey || got == ez, fname, fmt3, ex, ey, ez, got);
  } else {
    tmp = tmpfile();
    okay = false;

    assert(tmp != NULL);

    for (size_t i=0; !okay && i<expected; i++) {
      ex   = va_arg(va, size_t);
      okay = got == ex;

      if (!okay) {
        if (i == 0)
          fprintf(tmp, "expected %zu", ex);

        else if (i + 1 < expected)
          fprintf(tmp, ", %zu", ex);

        else
          fprintf(tmp, ", or %zu arguments, got  %zu", ex, got);
      }
    }

    if (!okay) {
      char* msg = read_file("<tmp>", tmp);
      print_error(fname, msg);
      deallocate(NULL, msg, 0);
      rl_longjmp(1);
    }
  }

  return got;
}


size_t vargcos(size_t expected, size_t got, const char* fname, ...) {
  static const char* fmt1 = "expected at least %zu arguments, got %zu";
  static const char* fmt2 = "expected %zu or at least %zu arguments, got %zu";
  static const char* fmt3 = "expected %zu, %zu, or at least %zu arguments, got %zu";

  va_list va;
  bool okay;
  FILE* tmp;
  size_t ex, ey, ez;

  va_start(va, fname);
  okay = true;

  if (expected == 1) {
    ex = va_arg(va, size_t);
    va_end(va);
    require(got >= ex, fname, fmt1, ex, got);
  } else if (expected == 2) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got >= ey, fname, fmt2, ex, ey, got);
  } else if (expected == 3) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    ez = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got == ey || got >= ez, fname, fmt3, ex, ey, ez, got);
  } else {
    tmp = tmpfile();
    okay = false;

    assert(tmp != NULL);

    for (size_t i=0; !okay && i<expected; i++) {
      ex   = va_arg(va, size_t);
      okay = i + 1 == expected ? got >= ex : got == ex;

      if (!okay) {
        if (i == 0)
          fprintf(tmp, "expected %zu", ex);

        else if (i + 1 < expected)
          fprintf(tmp, ", %zu", ex);

        else
          fprintf(tmp, ", or at least %zu arguments, got  %zu", ex, got);
      }
    }

    if (!okay) {
      char* msg = read_file("<tmp>", tmp);
      print_error(fname, msg);
      deallocate(NULL, msg, 0);
      rl_longjmp(1);
    }
  }

  return got;
}


#undef SIGNAL_ERROR

// external API
void init_vm(Vm* vm) {
  // initialize miscellaneous state
  vm->m.ctx   = NULL;
  vm->m.init  = false;

  // initialize global stacks
  vm->h.stack = (Objects) {
    .obj={
      .type    =&ObjectsType,
      .annot   =&EmptyMap,
      .no_trace=true,
      .no_sweep=true,
    },
    .data=NULL,
    .cnt =0,
    .cap =0,
  };

  vm->r.stack   = (Alist) {
    .obj={
      .type    =&AlistType,
      .annot   =&EmptyMap,
      .no_sweep=true,
      .gray    =true,
    },
    .data=NULL,
    .cnt =0,
    .cap =0,
  };

  vm->c.stack = (Alist) {
    .obj={
      .type    =&AlistType,
      .annot   =&EmptyMap,
      .no_sweep=true,
      .gray    =true,
    },
    .data=NULL,
    .cnt =0,
    .cap =0,
  };

  // initialize heap
  vm->h.objs  = NULL;
  vm->h.used  = 0;
  vm->h.cap   = N_HEAP;
  vm->h.frame = NULL;

  // initialize environment
  vm->n.symbols = new_symbol_table(0, NULL, NULL, NULL);
  vm->n.globals = new_name_space(0, NULL, NULL, NULL);
  vm->n.annot   = new_table(0, NULL, NULL, NULL);
  vm->n.used    = new_table(0, NULL, NULL, NULL);

  // initialize reader
  vm->r.table   = new_table(0, NULL, hash_word, same);
  vm->r.frame   = ReadFrames;

  // initialize compiler
  vm->c.frame = CompFrames;

  // initialize interpreter
  vm->e.frame  = ExecFrames;
  vm->e.upvals = NULL;
  vm->e.sp     = 0;
}

void free_vm(Vm* vm) {
  // deallocate global objects
  free_objects(&vm->h.stack);
  free_alist(&vm->r.stack);
  free_alist(&vm->c.stack);

  // traverse free objects list
  Obj* o = vm->h.objs;

  while (o != NULL) {
    Obj* n  = o;
    o       = o->next;

    if (!n->no_free)
      free_obj(n);

    if (!n->no_sweep)
      free(n);
  }
}

void sync_vm(Vm* vm) {
  (void)vm;
}

size_t push(Value x) {
  assert(RlVm.e.sp < N_VALUES);
  
  size_t out = RlVm.e.sp;
  StackSpace[RlVm.e.sp++] = x;
  return out;
}

Value pop(void) {
  assert(RlVm.e.sp > 0);
  return StackSpace[--RlVm.e.sp];
}

size_t pushn(size_t n) {
  assert(RlVm.e.sp + n < N_VALUES);
  size_t out = RlVm.e.sp;
  RlVm.e.sp += n;

  return out;
}

Value popn(size_t n) {
  assert(n <= RlVm.e.sp);
  Value out = StackSpace[RlVm.e.sp-n];
  RlVm.e.sp -= n;
  return out;
}

Value* peek(int i) {
  if (i < 0)
    i += RlVm.e.sp;

  assert(i >= 0 && (size_t)i < RlVm.e.sp);

  return StackSpace+i;
}

// internal GC helpers
static void add_gray(void* ptr) {
  objects_push(&RlVm.h.stack, ptr);
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
  Objects* grays = &vm->h.stack;
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
  Obj** l = &vm->h.objs, *c = vm->h.objs, *t;
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
  size_t allocated = vm->h.used, available = vm->h.cap;

  if (!vm->m.init) {
    vm->h.cap <<= 1;
  } else {
    if (allocated >= available) {
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
}

static void manage_vm(Vm* vm) {
  if (vm->m.init) {
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

    if (diff + vm->h.used > vm->h.cap)
      manage_vm(vm);

    vm->h.used += diff;
  } else {
    size_t diff    = n_bytes_removed - n_bytes_added;
    vm->h.used -= diff;
  }
}

// external GC helpers
void add_to_heap(void* p) {
  assert(p != NULL);

  Obj* o      = p;
  o->next     = RlVm.h.objs;
  RlVm.h.objs = o;
}

void* allocate(Vm* vm, size_t n_bytes) {
  if (vm)
    manage_heap(vm, n_bytes, 0);

  void* out = SAFE_MALLOC(n_bytes);
  memset(out, 0, n_bytes);

  return out;
}

// memory API
void* duplicate(Vm* vm, void* pointer, size_t n_bytes) {
  void* cpy = allocate(vm, n_bytes);

  memcpy(cpy, pointer, n_bytes);

  return cpy;
}

char* duplicates(Vm* vm, char* s, size_t n_chars) {
  char* cpy;
  
  assert(s != NULL);
  
  if (n_chars == 0)
    n_chars = strlen(s);

  cpy = allocate(vm, n_chars+1);
  strcpy(cpy, s);
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
