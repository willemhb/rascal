#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "array.h"

/* C types and APIs for the runtime system, including global state, memory management, and internal error handling. */

// C types
/* stores values on the C stack that need to be preserved during garbage collection. */
struct GcFrame {
  GcFrame* next;
  size_t   cnt;
  Value*   vals;
};

/* compiler frame */
struct CompFrame {
  CompState    state; // current compiler state (distinguishes what type of chunk is being compiled)
  flags_t      flags; // miscellaneous flags
  char*        name;  // name of the compiling code unit
  Chunk*       code;  // compiling code object
  Environment* envt;  // naming context
  Handlers*    hndl;  // effect handlers
};

/* reader frame */
struct ReadFrame {
  ReadState state;      // current reader state
  flags_t   flags;      // miscellaneous flags
  Table*    table;      // current readtable
  Stream*   source;     // input source
};

/* execution frame */
struct ExecFrame {
  Closure*  code;       // executing code object
  uint16_t* ip;         // current instruction pointer
  size_t    bp;         // stack address of current closure arguments
  size_t    fp;         // address of caller's frame
  size_t    pp;         // address of nearest enclosing prompt
};

/* Composite global state object. */
struct Vm {
  // heap
  struct {
    Obj*     objs;
    size_t   used;
    size_t   cap;
    GcFrame* frames;  // chain of values on the C stack that need to be preserved during collection
    Objects  stack;   // stack of marked values waiting to be traced
  } h;

  // environment
  struct {
    SymbolTable* symbols;
    NameSpace*   globals;
    Table*       annot;
    Table*       used;
  } n;

  // reader state
  struct {
    Table*     table;   // toplevel readtable
    ReadFrame* frame;   // pointer to current reader frame
    Alist      stack;   // private stack for storing subexpressions during `read`
  } r;

  // compiler state
  struct {
    CompFrame* frame;  // pointer to current compiler frame
    Alist      stack;  // private stack for storing subexpressions during `compile`
  } c;

  // execution state
  struct {
    UpValue*   upVals;  // toplevel list of open upvalues
    ExecFrame* frame;   // current exec frame
    size_t     sp;      // current stack pointer
  } e;

  // miscellaneous state
  struct {
    Context* ctx;  // error recovery context
    bool     init; // indicates whether the VM has finished initializing
  } m;  
};

/* miscellaneous object types vital in the runtime system */
/* represents a reified continuation. */
struct Control {
  Obj        obj;
  Value*     stack_copy;
  ExecFrame* frames_copy;
  size_t     n_stack, n_frames;
};

/* holds an upvalue (or its address). */
struct UpValue {
  Obj      obj;
  UpValue* next;
  size_t   offset;
  Value    value;
};

// globals
#define N_VALUES 32768
#define N_EXEC   8192
#define N_READ   2048
#define N_COMP   2048

extern Vm RlVm;

extern ReadFrame ReadFrames[N_READ];
extern CompFrame CompFrames[N_COMP];
extern ExecFrame ExecFrames[N_EXEC];
extern Value     StackSpace[N_VALUES];

// external API
void init_vm(Vm* vm);
void free_vm(Vm* vm);
void sync_vm(Vm* vm);

size_t push_macro_args(Environment* envt, List* form);
size_t push(Value x);
Value  pop(void);
size_t pushn(size_t n);
Value  popn(size_t n);
Value* peek(int i);

struct Context {
  Context* next;

  // GC state
  GcFrame* frames;

  // Rascal execution state
  size_t sp, fp, bp, pp;
  uint16_t* ip;
  Closure* code;

  // C execution state
  jmp_buf buf;
};


// globals
#define N_HEAP  (((size_t)1)<<19)
#define HEAP_LF 0.625

// external API
void unsave_gc_frame(GcFrame* frame);

#define save(n, args...)                             \
  Value __gc_frame_vals[(n)] = { args };             \
  GcFrame __gc_frame cleanup(unsave_gc_frame) = {    \
    .next=RlVm.heap.frames,                          \
    .cnt =(n),                                       \
    .vals=__gc_frame_vals                            \
  }

#define SAFE_ALLOC(func, args...)                           \
  ({                                                        \
    void* __out = func(args);                               \
                                                            \
    if (__out == NULL) {                                    \
      fprintf(stderr,                                       \
              "Out of memory calling %s at %s:%s:%d.\n",    \
              #func,                                        \
              __FILE__,                                     \
              __func__,                                     \
              __LINE__);                                    \
      exit(1);                                              \
    }                                                       \
    __out;                                                  \
  })

#define SAFE_MALLOC(nBytes)           SAFE_ALLOC(malloc, nBytes)
#define SAFE_REALLOC(pointer, nBytes) SAFE_ALLOC(realloc, pointer, nBytes)

#define mark(p, ...)                                        \
  generic((p),                                              \
          Value:mark_val,                                   \
          Value*:mark_vals,                                 \
          void**:mark_objs,                                 \
          default:mark_obj)(p __VA_OPT__(,) __VA_ARGS__)

#define unmark(p, ...)                                      \
  generic((p),                                              \
          Value:unmark_val,                                 \
          Value*:unmark_vals,                               \
          void**:unmark_objs,                               \
          default:unmark_obj)(p __VA_OPT__(,) __VA_ARGS__)

void  mark_val(Value val);
void  mark_obj(void* ptr);
void  mark_vals(Value* vals, size_t n);
void  mark_objs(void** objs, size_t n);

void  unmark_val(Value val);
void  unmark_obj(void* obj);
void  unmark_vals(Value* vals, size_t n);
void  unmark_objs(void** objs, size_t n);

void  add_to_heap(void* p);

void  init_heap(Vm* vm);
void  free_heap(Vm* vm);

void* allocate(Vm* vm, size_t nBytes);
void* duplicate(Vm* vm, void* pointer, size_t n_bytes);
char* duplicates(Vm* vm, char* chars, size_t n_chars);
void* reallocate(Vm* vm, void* pointer, size_t old_size, size_t new_size);
void  deallocate(Vm* vm, void* pointer, size_t n_bytes);

void init_runtime(Vm* vm);
void free_runtime(Vm* vm);
void reset_runtime(Vm* vm);

void save_state(Vm* vm, Context* ctx);
void restore_state(Vm* vm, Context* ctx);

void    raise(const char* fname, const char* fmt, ...);
bool    require(bool test, const char* fname, const char* fmt, ...);
size_t  argco(size_t expected, size_t got, const char* fname);
size_t  vargco(size_t expected, size_t got, const char* fname);
Type*   argtype(Type* expected, Value got, const char* fname);
Type*   argtypes(size_t n, Value got, const char* fname, ...);
size_t  argcos(size_t n, size_t got, const char* fname, ...);
size_t  vargcos(size_t n, size_t got, const char* fname, ...);

#define try                                                             \
  Context _ctx; int l__tr, l__ca;                                       \
  save_state(&RlVm, &_ctx); RlVm.ctx = &_ctx;                           \
  if (!setjmp(_ctx.buf))                                                \
    for (l__tr=1; l__tr; l__tr=0, (void)(RlVm.ctx=_ctx.next))

#define catch                                                   \
  else                                                          \
    for (l__ca=1; l__ca; l__ca=0, restore_state(&RlVm, &_ctx))

#endif
