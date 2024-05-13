#include "runtime.h"


/* Globals. */
#define INITIAL_HEAP_SIZE (1 << 19) // 2^16 * sizeof(Value)
#define MAXIMUM_HEAP_SIZE MAX_ARITY
#define STACK_SIZE (1 << 16)

Objects GrayObjects = {
  .h = {
    .next = NULL,
    .type = &ObjectsType,
    .hash = 0,
    .flags = 0,
    .trace = false,
    .free = true,
    .sweep = false,
    .gray = true,
    .black = false
  },

  .data = NULL,
  .count = 0,
  .max_count = 0
};

HeapState Heap = {
  .fp = NULL,
  .size = INITIAL_HEAP_SIZE,
  .max_size = MAXIMUM_HEAP_SIZE,
  .live_objects = NULL,
  .gray_objects = &GrayObjects
};

ReaderFrame ReaderFrameStack[STACK_SIZE] = {};

ReadTable ReaderTable = {
  .h = {
    .next = NULL,
    .type = &ReadTableType,
    .hash = 0,
    .flags = 0,
    .trace = false,
    .free = true,
    .sweep = false,
    .gray = true,
    .black = false
  },

  .entries = NULL,
  .count = 0,
  .max_count = 0
};

ReaderState Reader = {
  .fp = ReaderFrameStack,
  .read_table = &ReaderTable,
  .input = NULL,
  .buffer = NULL,
  .exprs = NULL
};

CompilerFrame CompilerFrameStack[STACK_SIZE] = {};

CompilerState Compiler = {
  .fp = CompilerFrameStack,
  .locals = NULL,
  .upvalues = NULL,
  .values = NULL,
  .code = NULL
};

InterpreterFrame InterpreterFrameStack[STACK_SIZE] = {};
Value ValueStack[STACK_SIZE] = {};

// global variables
Scope Globals = {
  .h = {
    .next = NULL,
    .type = &ScopeType,
    .flags = GLOBAL_SCOPE,
    .trace = true,
    .free = true,
    .sweep = false,
    .gray = true,
    .black = false
  },

  .entries = NULL,
  .count = 0,
  .max_count = 0
};

InterpreterState Interpreter = {
  .fp = InterpreterFrameStack,
  .globals = &Globals,
  .upvals = NULL,
  .code = NULL,
  .pc = NULL,
  .sp = ValueStack,
  .bp = ValueStack
};

/* Error APIs. */
rl_status_t rl_error(rl_status_t code, const char* fname, const char* fmt, ...);

/* HeapState API. */
// preserving values
void unpreserve(HeapFrame* frame) {
  Heap.fp = frame->next;
}

// memory management
rl_status_t allocate(void** buf, size_t n_bytes, bool use_heap);
rl_status_t reallocate(void** buf, size_t old_n_bytes, size_t new_n_bytes, bool use_heap);
rl_status_t duplicate(const void* ptr, void** buf, size_t n_bytes, bool use_heap);
rl_status_t deallocate(void* ptr, size_t n_bytes, bool use_heap);

// GC entry point
rl_status_t collect_garbage(void) {
  return OKAY;
}

// lifetime
rl_status_t init_heap_state(HeapState* heap_state) {
  (void)heap_state;
  return OKAY;
}

rl_status_t mark_heap_state(HeapState* heap_state) {
  (void)heap_state;
  return OKAY;
}

rl_status_t free_heap_state(HeapState* heap_state) {
  
}

/* ReaderState API */
// lifetime
rl_status_t init_reader_state(ReaderState* reader_state) {
  (void)reader_state;
  return OKAY;
}

rl_status_t mark_reader_state(ReaderState* reader_state) {
  
}

rl_status_t free_reader_state(ReaderState* reader_state) {
  
}

/* CompilerState API */

// lifetime
rl_status_t init_compiler_state(CompilerState* compiler_state) {
  (void)compiler_state;
  return OKAY;
}

rl_status_t mark_compiler_state(CompilerState* compiler_state) {
  
}

rl_status_t free_compiler_state(CompilerState* compiler_state) {
  
}

/* InterpreterState API */

// lifetime
rl_status_t init_interpreter_state(InterpreterState* interpreter_state) {
  (void)interpreter_state;
  return OKAY;
}

rl_status_t mark_interpreter_state(InterpreterState* interpreter_state) {
  
}

rl_status_t free_interpreter_state(InterpreterState* interpreter_state) {
  
}
