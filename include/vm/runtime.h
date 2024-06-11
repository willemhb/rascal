#ifndef rl_runtime_h
#define rl_runtime_h

#include "status.h"
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

/* frame types */
struct HFrame {
  HFrame* next;
  size_t  count;
  Value*  values;
};

/* state types */
struct HState {
  HFrame*      frames; // Live objects in the C stack that may not be visible from the roots (eg, when an intermediate object is created inside a C function).
  size_t       size, max_size;
  Object*      live_objects;
  Alist*       grays;
};

struct RState {
  Alist        frames;
  MutVec       stack;
  MutBin       buffer;
  ReadTable*   rt;
  MutMap*      gs;
  Port*        input;
};

struct CState {
  Alist        frames;
  MutVec       stack;
  List*        form;
  Environ*     envt;
  MutVec*      constants;
  MutBin*      code;
};

struct IState {
  Alist        frames;
  MutVec       stack;
  UpValue*     upvals; // List of open upvalues
  Closure*     code;   // currently executing code object
  uint16_t*    pc;     // program counter for currently executing code object
  size_t       cp;     // catch pointer
  size_t       hp;     // handle pointer
  size_t       bp;     // base pointer
  size_t       fp;     // frame pointer
};

/* Global State objects */
extern HState Heap;
extern RState Reader;
extern CState Compiler;
extern IState Interpreter;

/* Global table objects */
extern Environ     Globals;    // global environment
extern StringCache Strings;    // global string cache
extern MutMap      NameSpaces; // cache of namespaces mapped to their names
extern MutMap      Modules;    // cache of files
extern MutMap      Unions;     // cache of union signatures
extern ReadTable   BaseRt;     // base read table

/* External APIs */
/* Stack APIs */
// state generics (a lot of stack manipulation)
#define smark(S)                                \
  generic((S),                                  \
          HState*:hstate_mark,                  \
          RState*:rstate_mark,                  \
          CState*:cstate_mark,                  \
          IState*:istate_mark)(S)

#define spush(S, v)                             \
  generic((S),                                  \
          HState*:hstate_push,                  \
          RState*:rstate_push,                  \
          CState*:cstate_push,                  \
          IState*:istate_push)(S, v)

#define swrite(S, s, n)                         \
  generic((S),                                  \
          RState*:rstate_write,                 \
          CState*:cstate_write,                 \
          IState*:istate_write)(S, s, n)

#define spushn(S, n, ...)                                       \
  generic((S),                                                  \
          HState*:hstate_pushn,                                 \
          RState*:rstate_pushn,                                 \
          CState*:cstate_pushn,                                 \
          IState*:istate_pushn)(S, n __VA_OPT__(,) __VA_ARGS__)

#define spushf(S, ...)                                          \
  generic((S),                                                  \
          RState*:rstate_pushf,                                 \
          CState*:cstate_pushf,                                 \
          IState*:istate_pushf)(S __VA_OPT__(,) __VA_ARGS__)

#define spopf(S, ...)                                       \
  generic((S),                                              \
          RState*:rstate_popf,                              \
          CState*:cstate_popf,                              \
          IState*:istate_popf)(S __VA_OPT__(,) __VA_ARGS__)

#define swritef(S, ...)                                         \
  generic((S),                                                  \
          RState*:rstate_writef,                                \
          CState*:cstate_writef,                                \
          IState*:istate_writef)(S __VA_OPT__(,) __VA_ARGS__)

#define swriteb(S, s, ...)                                              \
  generic((S),                                                          \
          RState*:generic((s),                                          \
                          char:rstate_writec,                           \
                          char*:rstate_writecs),                        \
          CState*:generic((s),                                          \
                          uint16_t:cstate_writeo,                       \
                          uint16_t*:cstate_writeos),                    \
          )(S, s __VA_OPT__(,) __VA_ARGS__)

#define spop(S, r)                              \
  generic((S),                                  \
          HState*:hstate_pop,                   \
          RState*:rstate_pop,                   \
          CState*:cstate_pop,                   \
          IState*:istate_pop)(S, r)

#define spopn(S, r, t, n)                       \
  generic((S),                                  \
          HState*:hstate_popn,                  \
          RState*:rstate_popn,                  \
          CState*:cstate_popn,                  \
          IState*:istate_popn)(S, r, t, n)

/* HState & Heap APIs */
rl_status_t hstate_mark(HState* h);
rl_status_t hstate_push(HState* h, Object* o);
rl_status_t hstate_pushn(HState* h, size_t n, ...);
rl_status_t hstate_pop(HState* h, Object** o);
rl_status_t hstate_popn(HState* h, Object** o, bool t, size_t n);

// preserving values
void unpreserve(HFrame* frame);

#define preserve(n, vals...)                                    \
  Value __heap_frame_vals__[(n)] = { vals };                    \
  HFrame __heap_frame__ cleanup(unpreserve) =                   \
    { .next=Heap.fp, .count=(n), .values=__heap_frame_vals__ }; \
  Heap.fp=&__heap_frame__

// memory management
rl_status_t allocate(void** b, size_t n, bool h);
rl_status_t reallocate(void** b, size_t p, size_t n, bool h);
rl_status_t duplicate(const void* p, void** b, size_t n, bool h);
rl_status_t deallocate(void* p, size_t n, bool h);
rl_status_t collect_garbage(void);

/* RState and reader APIs */
rl_status_t rstate_mark(RState* s);
rl_status_t rstate_push(RState* s, Value v);
rl_status_t rstate_write(RState* s, Value* vs, size_t n);
rl_status_t rstate_pushn(RState* s, size_t n, ...);
rl_status_t rstate_pushf(RState* s, Port* i, ReadTable* rt, MutMap* gs);
rl_status_t rstate_writef(RState* s, RFrame* f, size_t n);
rl_status_t rstate_popf(RState* s);
rl_status_t rstate_writec(RState* s, char c);
rl_status_t rstate_writecs(RState* s, char* cs, size_t n);
rl_status_t rstate_pop(RState* s, Value* b);
rl_status_t rstate_popn(RState* s, Value* b, bool t, size_t n);

/* CState and compiler APIs */
rl_status_t cstate_mark(CState* s);
rl_status_t cstate_pop(CState* s, Value* b);

/* IState and interpreter APIs */
rl_status_t istate_mark(IState* s);
rl_status_t istate_pop(IState* s, Value* b);

#endif
