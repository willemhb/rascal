#include "util/number.h"

#include "vm.h"
#include "object.h"
#include "memory.h"

// internal GC helpers
static void addGray(void* ptr) {
  writeObjects(&RlVm.heap.grays, ptr);
}

static void markFrame(Frame* frame) {
  mark(frame->code);
}

static void markFrames(size_t n, Frame* frames) {
  for (size_t i=0; i<n; i++)
    markFrame(frames+i);
}

static void markVals(size_t n, Value* vs) {
  for (size_t i=0; i<n; i++)
    mark(vs[i]);
}

static void markObjs(size_t n, Obj** os) {
  for (size_t i=0; i<n; i++)
    mark(os[i]);
}

static void markSymbols(SymbolCache* cache) {
  SymbolCacheEntry* entries = cache->table;
  
  for (size_t i=0; i<cache->capacity; i++) {
    if (entries[i].key != NULL)
      mark(entries[i].val);
  }
}

static void markNsMap(NsMap* map) {
  NsMapEntry* entries = map->table;

  for (size_t i=0; i<map->capacity; i++) {
    if (entries[i].key != NULL) {
      mark(entries[i].key);
      mark(entries[i].val);
    }
  }
}

static void markMethodCache(MethodCache* cache) {
  MethodCacheEntry* entries = cache->table;

  for (size_t i=0; i<cache->capacity; i++) {
    if (entries[i].key != NULL) {
      mark(entries[i].key);
      mark(entries[i].val);
    }
  }
}

static void markLoadCache(LoadCache* cache) {
  LoadCacheEntry* entries = cache->table;

  for (size_t i=0; i<cache->capacity; i++) {
    if (entries[i].key != NULL) {
      mark(entries[i].key);
      mark(entries[i].val);
    }
  }
}

static void markAnnotations(Annotations* annot) {
  AnnotationsEntry* entries = annot->table;

  for (size_t i=0; i<annot->capacity; i++) {
    if (entries[i].key != NOTHING)
      mark(entries[i].val);
  }
}

void markObj(void* p) {
  if (p != NULL) {
    Obj* o = p;

    if (o->black == false) {
      o->black = true;
      markObj(o->annot);

      Type* t = typeOf(o);

      if (t->vTable->trace)
        addGray(o);

      else
        o->gray = false;
    }
  }
}

void markVal(Value val) {
  if (IS_OBJ(val))
    markObj(AS_OBJ(val));
}

static void markVm(Vm* vm) {
  // mark other globals (empty singletons)
  mark(&emptyList);
  mark(&emptyVector);
  mark(&emptyMap);

  // mark `vm->heap`
  markVals(vm->heap.saved.count, vm->heap.saved.data);

  // mark `vm->toplevel`
  markSymbols(&vm->toplevel.symbols);
  markNsMap(&vm->toplevel.globals);
  markAnnotations(&vm->toplevel.annot);
  markLoadCache(&vm->toplevel.used);

  // mark objects in `vm->reader`
  markVals(vm->reader.stack.count, vm->reader.stack.data);

  // mark objects in `vm->compiler`
  mark(vm->compiler.chunk);
  mark(vm->compiler.ns);
  markVals(vm->compiler.stack.count, vm->compiler.stack.data);

  // mark objects in `vm->exec`
  mark(vm->exec.upVals);
  mark(vm->exec.code);

  // mark stack
  markVals(vm->exec.sp, vm->stackBase);

  // mark call stack
  markFrames(vm->exec.fp, vm->framesBase);
}

static void traceVm(Vm* vm) {
  Objects* grays = &vm->heap.grays;
  Type* t; Obj* o;

  while (grays->count > 0) {
    o = popObjects(grays);
    // assert(obj->gray == false);
    t = typeOf(o);
    t->vTable->trace(o);
    o->gray = false;
  }
}

static void sweepVm(Vm* vm) {
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
      tp = typeOf(t);

      if (tp->vTable->free && !t->noFree)
        tp->vTable->free(t);

      if (!t->noSweep)
        deallocate(vm, t, sizeOf(t));
    }

    c  = *l;
  }
}

static void resizeVm(Vm* vm) {
  size_t allocated = vm->heap.used, available = vm->heap.cap;

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

static void manageVm(Vm* vm) {
  markVm(vm);
  traceVm(vm);
  sweepVm(vm);
  resizeVm(vm);
}

static void manageHeap(Vm* vm, size_t nBytesAdded, size_t nBytesRemoved) {
  assert(vm != NULL);

  if (nBytesAdded > nBytesRemoved) {
    size_t diff = nBytesAdded - nBytesRemoved;

    if (diff + vm->heap.used > vm->heap.cap)
      manageVm(vm);

    vm->heap.used += diff;
  } else {
    size_t diff = nBytesRemoved - nBytesAdded;
    vm->heap.used -= diff;
  }
}

// external GC helpers
// trace helpers
void traceFunction(void* p) {
  Function* f = p;

  mark(f->name);
  mark(f->methods);
  mark(f->singleton);
}

void traceType(void* p) {
  Type* t = p;

  mark(t->name);
  mark(t->parent);
  mark(t->left);
  mark(t->right);
  mark(t->ctor);
}

void traceBinding(void* p) {
  Binding* b = p;

  mark(b->name);
  mark(b->scope);
  mark(b->value);
}

void traceList(void* p) {
  List* l = p;

  mark(l->head);
  mark(l->tail);
}

void traceVector(void* p) {
  Vector* v = p;

  mark(v->root);
  markVals(v->arity & 63, v->tail);
}

void traceMap(void* p) {
  Map* m = p;

  mark(m->root);
}

void traceMethodTable(void* p) {
  MethodTable* m = p;

  markMethodCache(&m->cache);
  mark(m->zeroArityMethod);
  mark(m->fixedArityMethods);
  mark(m->variadicMethods);
}

void traceChunk(void* p) {
  Chunk* c = p;

  mark(c->name);
  markVals(c->vals.count, c->vals.data);
}

void traceClosure(void* p) {
  Closure* c = p;

  mark(c->code);
  mark(c->envt);
}

void traceControl(void* p) {
  Control* c = p;

  markFrame(&c->frame);
  markVals(c->nStack, c->stackCopy);
  markFrames(c->nFrames, c->framesCopy);
}

void traceScope(void* p) {
  Scope* s = p;

  mark(s->parent);
  markNsMap(&s->locals);
  markNsMap(&s->upVals);
}

void traceNameSpace(void* p) {
  NameSpace* n = p;

  // globals already marked
  markNsMap(n->private);
  mark(n->scope);
}

void traceEnvironment(void* p) {
  Environment* e = p;

  mark(e->parent);
  mark(e->ns);
  markObjs(e->upvals.count, e->upvals.data);
}

void traceUpValue(void* p) {
  UpValue* u = p;

  mark(u->next);
  mark(u->value);
}

void traceVecNode(void* p) {
  VecNode* n = p;

  markObjs(n->count, n->children);
}

void traceVecLeaf(void* p) {
  VecLeaf* l = p;

  markVals(64, l->slots);
}

void traceMapNode(void* p) {
  MapNode* n = p;

  markObjs(n->count, n->children);
}

void traceMapLeaf(void* p) {
  MapLeaf* l = p;

  mark(l->next);
  mark(l->key);
  mark(l->val);
}

// free helpers
void freeSymbol(void* p) {
  Symbol* s = p;

  deallocate(NULL, s->name, 0);
}

void freeStream(void* p) {
  Stream* s = p;

  if (s->ios != stdout && s->ios != stdin && s->ios != stderr)
    fclose(s->ios);
}

void freeBig(void* p) {
  Big* b = p;

  deallocate(NULL, b->digits, 0);
}

void freeBits(void* p) {
  Bits* b = p;

  deallocate(NULL, b->data, 0);
}

void freeMethodTable(void* p) {
  MethodTable* m = p;

  freeMethodCache(&m->cache);
}

void freeChunk(void* p) {
  Chunk* c = p;

  freeValues(&c->vals);
  freeByteCode(&c->code);
}

void freeControl(void* p) {
  Control* c = p;

  deallocate(NULL, c->stackCopy, 0);
  deallocate(NULL, c->framesCopy, 0);
}

void freeScope(void* p) {
  Scope* s = p;

  freeNsMap(&s->locals);
  freeNsMap(&s->upVals);
}

void freeNameSpace(void* p) {
  NameSpace* n = p;

  if (!!(n->obj.flags == COMPILER_WITH)) {
    freeNsMap(n->private);
    deallocate(NULL, n->private, 0);
  }
}

void freeEnvironment(void* p) {
  Environment* e = p;

  freeObjects(&e->upvals);
}

void freeVecNode(void* p) {
  VecNode* n = p;

  deallocate(NULL, n->children, 0);
}

void freeMapNode(void* p) {
  MapNode* n = p;

  deallocate(NULL, n->children, 0);
}

// external API
void  initHeap(Vm* vm) {
  vm->heap.objs = NULL;
  vm->heap.used = 0;
  vm->heap.cap  = N_HEAP;

  initObjects(&vm->heap.grays);
  initValues(&vm->heap.saved);
}

void freeHeap(Vm* vm) {
  freeObjects(&vm->heap.grays);
  freeValues(&vm->heap.saved);
  
  // TODO: free live objects
}

void addToHeap(void* p) {
  assert(p != NULL);

  Obj* o         = p;
  o->next        = RlVm.heap.objs;
  RlVm.heap.objs = o;
}

void* allocate(Vm* vm, size_t nBytes) {
  if (vm)
    manageHeap(vm, nBytes, 0);

  void* out = SAFE_MALLOC(nBytes);
  memset(out, 0, nBytes);

  return out;
}

void* duplicate(Vm* vm, void* pointer, size_t nBytes) {
  void* cpy = allocate(vm, nBytes);

  memcpy(cpy, pointer, nBytes);

  return cpy;
}

void* reallocate(Vm* vm, void* pointer, size_t oldSize, size_t newSize) {
  void* out;
  
  if (newSize == 0) {
    deallocate(vm, pointer, oldSize);
    out = NULL;
  }
  
  else {
    if (vm)
      manageHeap(vm, newSize, oldSize);

    out = SAFE_REALLOC(pointer, newSize);

    if (newSize > oldSize)
      memset(out+oldSize, 0, newSize-oldSize);
  }
  
  return out;
}

void deallocate(Vm* vm, void* pointer, size_t nBytes) {
  if (vm)
    manageHeap(vm, 0, nBytes);

  free(pointer);
}
