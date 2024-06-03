#ifndef rl_runtime_h
#define rl_runtime_h

#include "value.h"

/* Definitions and declarations for internal state objects & functions (memory management, vm, &c). */
/* C types */
/* frame types */
typedef struct HeapFrame   HeapFrame;
typedef struct ReaderFrame ReaderFrame;
typedef struct CompilerFrame CompilerFrame;
typedef struct InterpreterFrame InterpreterFrame;

/* // state types */
typedef struct HeapState HeapState;
typedef struct ReaderState ReaderState;
typedef struct CompilerState CompilerState;
typedef struct InterpreterState InterpreterState;

/* frame types */
struct HeapFrame {
  HeapFrame* next;
  size_t count;
  Value* values;
};

struct InterpreterFrame {
  Closure*  code;
  Value*    bp;
  uint16_t* pc;
  size_t    fsize;
};

struct CompilerFrame {
  Scope* locals;
  Scope* upvalues;
  MutVec* values;
  MutBin* code;
};

struct ReaderFrame {
  Port* input;
  MutStr* buffer;
  MutVec* exprs;
};

/* state types */
struct HeapState {
  HeapFrame* fp; // Live objects in the C stack that may not be visible from the roots (eg, when an intermediate object is created inside a C function).
  size_t size, max_size;
  Object* live_objects;
  Objects* gray_objects; // objects that have been marked but not traced
};

struct ReaderState {
  ReaderFrame* fp;
  ReadTable* read_table;
  Port* input;
  MutStr* buffer;
  MutVec* exprs;
};

struct CompilerState {
  CompilerFrame* fp;
  Scope* locals, * upvalues;
  MutVec* values;
  MutBin* code;
};

struct InterpreterState {
  InterpreterFrame* fp; // call stack (next free CallFrame)
  Scope* globals; // global variables
  UpValue* upvals; // List of open upvalues
  Closure* code; // currently executing code object
  uint16_t* pc; // program counter for currently executing code object
  Value* sp, * bp; // stack pointer and base pointer
};

/* Globals */
extern HeapState Heap;
extern ReaderState Reader;
extern CompilerState Compiler;
extern InterpreterState Interpreter;

/* External APIs */
/* Error APIs */
rl_status_t rl_error(rl_status_t code, const char* fname, const char* fmt, ...);

/* HeapState API */
// preserving values
void unpreserve(HeapFrame* frame);

#define preserve(n, vals...)                                    \
  Value __heap_frame_vals__[(n)] = { vals };                    \
  HeapFrame __heap_frame__ cleanup(unpreserve) =                \
    { .next=Heap.fp, .count=(n), .values=__heap_frame_vals__ }; \
  Heap.fp=&__heap_frame__

// memory management
rl_status_t allocate(void** buf, size_t n_bytes, bool use_heap);
rl_status_t reallocate(void** buf, size_t old_n_bytes, size_t new_n_bytes, bool use_heap);
rl_status_t duplicate(const void* ptr, void** buf, size_t n_bytes, bool use_heap);
rl_status_t deallocate(void* ptr, size_t n_bytes, bool use_heap);
rl_status_t collect_garbage(void);

// lifetime
rl_status_t init_heap_state(HeapState* heap_state);
rl_status_t mark_heap_state(HeapState* heap_state);
rl_status_t free_heap_state(HeapState* heap_state);

/* ReaderState API */
// lifetime
rl_status_t init_reader_state(ReaderState* reader_state);
rl_status_t mark_reader_state(ReaderState* reader_state);
rl_status_t free_reader_state(ReaderState* reader_state);

/* CompilerState API */

// lifetime
rl_status_t init_compiler_state(CompilerState* compiler_state);
rl_status_t mark_compiler_state(CompilerState* compiler_state);
rl_status_t free_compiler_state(CompilerState* compiler_state);

/* InterpreterState API */

// lifetime
rl_status_t init_interpreter_state(InterpreterState* interpreter_state);
rl_status_t mark_interpreter_state(InterpreterState* interpreter_state);
rl_status_t free_interpreter_state(InterpreterState* interpreter_state);

#endif
