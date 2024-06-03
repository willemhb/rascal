#ifndef rl_runtime_h
#define rl_runtime_h

#include "value.h"

/* Definitions and declarations for internal state objects & functions (memory management, vm, &c). */
/* C types */
/* frame types */
typedef struct HFrame HFrame;
typedef struct RFrame RFrame;
typedef struct CFrame CFrame;
typedef struct IFrame IFrame;

/* state types */
typedef struct HState HState;
typedef struct RState RState;
typedef struct CState CState;
typedef struct IState IState;

#define BUFFER_TYPE(T, _T)                       \
  typedef struct T##Buffer {                     \
    _T* base;                                    \
    _T* fast;                                    \
    size_t next, max, fmax;                      \
  } T##Buffer

BUFFER_TYPE(Value, Value);
BUFFER_TYPE(Text, char);
BUFFER_TYPE(Object, Object*);
BUFFER_TYPE(RFrame, RFrame);
BUFFER_TYPE(CFrame, CFrame);
BUFFER_TYPE(IFrame, IFrame);

#undef BUFFER_TYPE

#define BUFFER_API(T, _T, t)                                            \
  rl_status_t init_##t##_buffer(T##Buffer* b, _T* f, size_t fm);        \
  rl_status_t free_##t##_buffer(T##Buffer* b);                          \
  rl_status_t resize_##t##_buffer(T##Buffer* b, size_t* n);             \
  rl_status_t t##_buffer_push(T##Buffer* b, _T t, size_t* r);           \
  rl_status_t t##_buffer_pop(T##Buffer* b, _T* r);                      \
  rl_status_t t##_buffer_pushn(T##Buffer* b, bool t, size_t* n, ...);   \
  rl_status_t t##_buffer_popn(T##Buffer* b, _T* r, bool t, size_t n);   \
  rl_status_t t##_buffer_write(T##Buffer* b, _T* s, bool t, size_t* n)

BUFFER_API(Value, Value, value);
BUFFER_API(Text, char, text);
BUFFER_API(Object, Object*, object);
BUFFER_API(RFrame, RFrame, rframe);
BUFFER_API(CFrame, CFrame, cframe);
BUFFER_API(IFrame, IFrame, iframe);

#undef BUFFER_API

/* frame types */
struct HFrame {
  HFrame* next;
  size_t  count;
  Value*  values;
};

struct IFrame {
  Closure*  code;
  uint16_t* pc;
  size_t    bp;
  size_t    fsize;
};

struct CFrame {
  List*    form;
  Environ* envt;
  MutVec*  values;
  MutBin*  code;
};

struct RFrame {
  Port*      input;
  ReadTable* rt;
};

/* state types */
struct HState {
  HFrame*      frames; // Live objects in the C stack that may not be visible from the roots (eg, when an intermediate object is created inside a C function).
  size_t       size, max_size;
  Object*      live_objects;
  ObjectBuffer grays;
};

struct RState {
  RFrameBuffer frames;
  ValueBuffer  stack;
  TextBuffer   buffer;
  ReadTable*   read_table;
  Gensyms*     gensyms;
  Port*        input;
};

struct CState {
  CFrameBuffer frames;
  ValueBuffer  stack;
  List*        form;
  Environ*     envt;
  MutVec*      constants;
  MutBin*      code;
};

struct IState {
  IFrameBuffer frames;
  ValueBuffer  stack;
  UpValue*     upvals; // List of open upvalues
  Closure*     code; // currently executing code object
  uint16_t*    pc; // program counter for currently executing code object
};

/* Global State objects */
extern HState Heap;
extern RState Reader;
extern CState Compiler;
extern IState Interpreter;

/* Global table objects */
extern Environ     Globals; // global environment
extern ModuleCache Modules; // global module cache
extern StringCache Strings; // global string cache
extern UnionCache  Unions;  // global union cache
extern ReadTable   BaseRt;  // base read table

/* External APIs */
/* Error APIs */
rl_status_t rl_error(rl_status_t code, const char* fname, const char* fmt, ...);

/* HState API */
// preserving values
void unpreserve(HFrame* frame);

#define preserve(n, vals...)                                    \
  Value __heap_frame_vals__[(n)] = { vals };                    \
  HFrame __heap_frame__ cleanup(unpreserve) =                \
    { .next=Heap.fp, .count=(n), .values=__heap_frame_vals__ }; \
  Heap.fp=&__heap_frame__

// memory management
rl_status_t allocate(void** buf, size_t n_bytes, bool use_heap);
rl_status_t reallocate(void** buf, size_t old_n_bytes, size_t new_n_bytes, bool use_heap);
rl_status_t duplicate(const void* ptr, void** buf, size_t n_bytes, bool use_heap);
rl_status_t deallocate(void* ptr, size_t n_bytes, bool use_heap);
rl_status_t collect_garbage(void);

// stack manipulation
#define push(S, v)                              \
  generic((S),                                  \
          RState*:rstate_push,                  \
          CState*:cstate_push,                  \
          IState*:istate_push)(S, v)

#define pop(S, v, r)                            \
  generic((S),                                  \
          Rstate*:rstate_pop,                   \
          CState*:cstate_pop,                   \
          IState*:istate_pop)(S, v, r)

#define pushn(S, v, n, ...)                     \
  generic((S),)

#endif
