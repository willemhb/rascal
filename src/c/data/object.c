#include "data/object.h"

#include "runtime/memory.h"

// APIs & utilities
value_t object(void* ox) {
  return ((uword)ox) | OBJTAG;
}

type_t object_type(void* obj) {
  assert(obj);

  return head(obj)->type;
}

usize object_size(void* obj) {
  assert(obj);

  return ObjectSize[object_type(obj)](obj);
}

void mark_object(void* obj) {
  if (obj == NULL)
    return;

  if (head(obj)->black)
    return;

  head(obj)->black = true;

  if (TraceObject[head(obj)->type])
    push_gray(obj);

  else
    head(obj)->gray = false;
}

void free_object(void* obj) {
  assert(obj);

  if (FreeObject[object_type(obj)])
    FreeObject[object_type(obj)](obj);

  deallocate(obj, object_size(obj));
}

bool object_hasfl(void* obj, flags fl) {
  assert(obj);

  return flagp(head(obj)->flags, fl);
}

bool object_setfl(void* obj, flags fl) {
  assert(obj);

  bool out = !flagp(head(obj)->flags, fl);
  head(obj)->flags |= fl;

  return out;
}

bool object_delfl(void* obj, flags fl) {
  assert(obj);

  bool out = flagp(head(obj)->flags, fl);
  head(obj)->flags &= ~fl;

  return out;
}

// toplevel initialization & globals
usize (*ObjectSize[NTYPES])(void* obj);
void  (*TraceObject[NTYPES])(void* obj);
void  (*FreeObject[NTYPES])(void* obj);

void object_init(void) {
  extern usize symbol_size(void* obj);
  extern usize list_size(void* obj);
  extern usize binary_size(void* obj);
  extern usize table_size(void* obj);
  extern usize vector_size(void* obj);
  extern usize function_size(void* obj);
  extern usize chunk_size(void* obj);
  extern usize closure_size(void* obj);
  extern usize upvalue_size(void* obj);
  extern usize variable_size(void* obj);
  extern usize environment_size(void* obj);
  
  ObjectSize[SYMBOL]      = symbol_size;
  ObjectSize[LIST]        = list_size;
  ObjectSize[BINARY]      = binary_size;
  ObjectSize[TABLE]       = table_size;
  ObjectSize[VECTOR]      = vector_size;
  ObjectSize[FUNCTION]    = function_size;
  ObjectSize[CHUNK]       = chunk_size;
  ObjectSize[CLOSURE]     = closure_size;
  ObjectSize[UPVALUE]     = upvalue_size;
  ObjectSize[VARIABLE]    = variable_size;
  ObjectSize[ENVIRONMENT] = environment_size;

  extern void trace_symbol(void* obj);
  extern void trace_list(void* obj);
  extern void trace_table(void* obj);
  extern void trace_vector(void* obj);
  extern void trace_function(void* obj);
  extern void trace_chunk(void* obj);
  extern void trace_closure(void* obj);
  extern void trace_upvalue(void* obj);
  extern void trace_variable(void* obj);
  extern void trace_environment(void* obj);
  
  TraceObject[SYMBOL]      = trace_symbol;
  TraceObject[LIST]        = trace_list;
  TraceObject[BINARY]      = NULL;
  TraceObject[TABLE]       = trace_table;
  TraceObject[VECTOR]      = trace_vector;
  TraceObject[FUNCTION]    = trace_function;
  TraceObject[CHUNK]       = trace_chunk;
  TraceObject[CLOSURE]     = trace_closure;
  TraceObject[UPVALUE]     = trace_upvalue;
  TraceObject[VARIABLE]    = trace_variable;
  TraceObject[ENVIRONMENT] = trace_environment;

  extern void free_symbol(void* obj);
  extern void free_binary(void* obj);
  extern void free_table(void* obj);
  extern void free_vector(void* obj);
  
  FreeObject[SYMBOL]      = free_symbol;
  FreeObject[LIST]        = NULL;
  FreeObject[BINARY]      = free_binary;
  FreeObject[TABLE]       = free_table;
  FreeObject[VECTOR]      = free_vector;
  FreeObject[FUNCTION]    = NULL;
  FreeObject[CHUNK]       = NULL;
  FreeObject[CLOSURE]     = NULL;
  FreeObject[UPVALUE]     = NULL;
  FreeObject[VARIABLE]    = NULL;
  FreeObject[ENVIRONMENT] = NULL;
}
