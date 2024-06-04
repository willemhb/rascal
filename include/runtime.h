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

/* buffer types - optimistically use statically allocated buffer, but can use
   heap space in a pinch. Keep track of the static buffer and switch back when
   stack shrinks below a certain size. */

typedef struct ValueBuffer {
  Value*   base;
  Value*   fbase;
  bool     small;
  uint32_t next, max, smax;
} ValueBuffer;

typedef struct ObjectBuffer {
  Object** base;
  Object** fbase;
  bool     small;
  uint32_t next, max, smax;
} ObjectBuffer;

typedef struct TextBuffer {
  char*    base;
  char*    fbase;
  bool     small;
  uint32_t next, max, smax;
} TextBuffer;

typedef struct RFrameBuffer {
  RFrame*  base;
  RFrame*  fbase;
  bool     small;
  uint32_t next, max, smax;
} RFrameBuffer;

typedef struct CFrameBuffer {
  CFrame*  base;
  CFrame*  fbase;
  bool     small;
  uint32_t next, max, smax;
} CFrameBuffer;

typedef struct IFrameBuffer {
  IFrame*  base;
  IFrame*  fbase;
  bool     small;
  uint32_t next, max, smax;
} IFrameBuffer;

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

/* Stack APIs */
// value buffer
rl_status_t init_value_buffer(ValueBuffer* b, Value* fb, uint32_t sm);
rl_status_t free_value_buffer(ValueBuffer* b);
rl_status_t grow_value_buffer(ValueBuffer* b, size_t n);
rl_status_t shrink_value_buffer(ValueBuffer* b, size_t n);
rl_status_t value_buffer_write(ValueBuffer* b, Value* src, size_t n);
rl_status_t value_buffer_push(ValueBuffer* b, Value v);
rl_status_t value_buffer_pushn(ValueBuffer* b, size_t n, ...);
rl_status_t value_buffer_pushv(ValueBuffer* b, size_t n, va_list va);
rl_status_t value_buffer_pop(ValueBuffer* b, Value* r);
rl_status_t value_buffer_popn(ValueBuffer* b, Value* r, bool top, size_t n);

// object buffer
rl_status_t init_object_buffer(ObjectBuffer* b, Object** fb, uint32_t sm);
rl_status_t free_object_buffer(ObjectBuffer* b);
rl_status_t grow_object_buffer(ObjectBuffer* b, size_t n);
rl_status_t shrink_object_buffer(ObjectBuffer* b, size_t n);
rl_status_t object_buffer_write(ObjectBuffer* b, Object** src, size_t n);
rl_status_t object_buffer_push(ObjectBuffer* b, Object* v);
rl_status_t object_buffer_pushn(ObjectBuffer* b, size_t n, ...);
rl_status_t object_buffer_pushv(ObjectBuffer* b, size_t n, va_list va);
rl_status_t object_buffer_pop(ObjectBuffer* b, Object** r);
rl_status_t object_buffer_popn(ObjectBuffer* b, Object** r, bool top, size_t n);

// text buffer
rl_status_t init_text_buffer(TextBuffer* b, char* fb, uint32_t sm);
rl_status_t free_text_buffer(TextBuffer* b);
rl_status_t grow_text_buffer(TextBuffer* b, size_t n);
rl_status_t shrink_text_buffer(TextBuffer* b, size_t n);
rl_status_t text_buffer_write(TextBuffer* b, char* src, size_t n);
rl_status_t text_buffer_push(TextBuffer* b, char v);
rl_status_t text_buffer_pushn(TextBuffer* b, size_t n, ...);
rl_status_t text_buffer_pushv(TextBuffer* b, size_t n, va_list va);
rl_status_t text_buffer_pop(TextBuffer* b, char* r);
rl_status_t text_buffer_popn(TextBuffer* b, char* r, bool top, size_t n);

// reader frame buffer
rl_status_t init_rframe_buffer(RFrameBuffer* b, RFrame* fb, uint32_t sm);
rl_status_t free_rframe_buffer(RFrameBuffer* b);
rl_status_t grow_rframe_buffer(RFrameBuffer* b, size_t n);
rl_status_t shrink_rframe_buffer(RFrameBuffer* b, size_t n);
rl_status_t rframe_buffer_write(RFrameBuffer* b, RFrame* src, size_t n);
rl_status_t rframe_buffer_push(RFrameBuffer* b, RFrame v);
rl_status_t rframe_buffer_pushn(RFrameBuffer* b, size_t n, ...);
rl_status_t rframe_buffer_pushv(RFrameBuffer* b, size_t n, va_list va);
rl_status_t rframe_buffer_pop(RFrameBuffer* b, RFrame* r);
rl_status_t rframe_buffer_popn(RFrameBuffer* b, RFrame* r, bool top, size_t n);

// compiler frame buffer
rl_status_t init_cframe_buffer(CFrameBuffer* b, CFrame* fb, uint32_t sm);
rl_status_t free_cframe_buffer(CFrameBuffer* b);
rl_status_t grow_cframe_buffer(CFrameBuffer* b, size_t n);
rl_status_t shrink_cframe_buffer(CFrameBuffer* b, size_t n);
rl_status_t cframe_buffer_write(CFrameBuffer* b, CFrame* src, size_t n);
rl_status_t cframe_buffer_push(CFrameBuffer* b, CFrame v);
rl_status_t cframe_buffer_pushn(CFrameBuffer* b, size_t n, ...);
rl_status_t cframe_buffer_pushv(CFrameBuffer* b, size_t n, va_list va);
rl_status_t cframe_buffer_pop(CFrameBuffer* b, CFrame* r);
rl_status_t cframe_buffer_popn(CFrameBuffer* b, CFrame* r, bool top, size_t n);

// interpreter frame buffer
rl_status_t init_iframe_buffer(IFrameBuffer* b, IFrame* fb, uint32_t sm);
rl_status_t free_iframe_buffer(IFrameBuffer* b);
rl_status_t grow_iframe_buffer(IFrameBuffer* b, size_t n);
rl_status_t shrink_iframe_buffer(IFrameBuffer* b, size_t n);
rl_status_t iframe_buffer_write(IFrameBuffer* b, IFrame* src, size_t n);
rl_status_t iframe_buffer_push(IFrameBuffer* b, IFrame v);
rl_status_t iframe_buffer_pushn(IFrameBuffer* b, size_t n, ...);
rl_status_t iframe_buffer_pushv(IFrameBuffer* b, size_t n, va_list va);
rl_status_t iframe_buffer_pop(IFrameBuffer* b, IFrame* r);
rl_status_t iframe_buffer_popn(IFrameBuffer* b, IFrame* r, bool top, size_t n);

// stack manipulation generics
#define push(S, v)                              \
  generic((S),                                  \
          HState*:hstate_push,                  \
          RState*:rstate_push,                  \
          CState*:cstate_push,                  \
          IState*:istate_push)(S, v)

#define pushn(S, n, ...)                                        \
  generic((S),                                                  \
          HState*:hstate_pushn,                                 \
          RState*:rstate_pushn,                                 \
          CState*:cstate_pushn,                                 \
          IState*:istate_pushn)(S, n __VA_OPT__(,) __VA_ARGS__)

#define pushf(S, ...)                                           \
  generic((S),                                                  \
          RState*:rstate_pushf,                                 \
          CState*:cstate_pushf,                                 \
          IState*:istate_pushf,)(S __VA_OPT__(,) __VA_ARGS__)

#define pop(S, r)                               \
  generic((S),                                  \
          HState*:hstate_pop,                   \
          Rstate*:rstate_pop,                   \
          CState*:cstate_pop,                   \
          IState*:istate_pop)(S, r)

#define popn(S, r, t, n)                        \
  generic((S),                                  \
          HState*:hstate_popn,                  \
          RState*:rstate_popn,                  \
          CState*:cstate_popn,                  \
          IState*:istate_popn)(S, r, t, n)

/* HState & Heap APIs */
rl_status_t hstate_push(HState* h, Object* o);
rl_status_t hstate_pushn(HState* h, size_t n, ...);
rl_status_t hstate_pop(HState* h, Object** o);
rl_status_t hstate_popn(HState* h, Object** o, bool t, size_t n);

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

#endif
