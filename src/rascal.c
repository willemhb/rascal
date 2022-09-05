// C modules ------------------------------------------------------------------
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "memutils.h"
#include "rascal.h"

// globals --------------------------------------------------------------------
/*
  VM main registers:
  fun - currently executing closure
  arg - offset of environment
  ctl - program counter
  val - result of last computation

  layout of a stack frame:
  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
  | upv | a-1 | ... | a-n | v-1 | ... | v-n | fun | arg | ctl |
  +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
  ^                                         ^
  |                                         |
  arg                                       caller state

 */

uint Arg, Ctl;
value_t Fun, Val;

stack_t *Stack;

heap_t *Heap;
table_t *Symbols;

gc_frame_t *Saved;
jmp_buf Toplevel;

// runtime utilities ----------------------------------------------------------
void verror(const char *fname, const char *fmt, va_list va)
{
  static const char *errfmt = "%s: error: ";
  fprintf(stderr, errfmt, fname);
  vfprintf(stderr, fmt, va); fprintf(stderr, ".\n" );
  va_end(va);
  longjmp(Toplevel, 1);
}

void error(const char *fname, const char *fmt, ...)
{
  static const char *errfmt = "%s: error: ";
  fprintf(stderr, errfmt, fname);

  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va); fprintf(stderr, ".\n" );
  va_end(va);
  longjmp(Toplevel, 1);
}

// internal type implementations ----------------------------------------------
// stack implementation -------------------------------------------------------
#define N_STACK 4096

#define stack_size(s)   ((s)->cap*sizeof(value_t))
#define stack_data(s)   ((s)->data)
#define stack_sp(s)     ((s)->sp)
#define stack_cap(s)    ((s)->cap)
#define stack_tos(s)    ((s)->data[(s)->sp-1])
#define stack_ref(s,n)  ((s)->data[(n)])
#define stack_peek(s,n) ((s)->data[(s)->sp-1-(n)])

void init_stack(stack_t *s)
{
  stack_sp(s) = 0;
  stack_cap(s) = N_STACK;
  stack_data(s) = malloc_s( stack_size(s) );
}

void finalize_stack(stack_t *s)
{
  free_s( stack_data(s) );
}

void grow_stack(stack_t *s)
{
  stack_cap(s) *= 2;
  stack_data(s) = realloc_s( stack_data(s), stack_size(s) );
}

void shrink_stack(stack_t *s)
{
  stack_cap(s) /= 2;
  stack_data(s) = realloc_s( stack_data(s), stack_size(s) );
}

bool check_grow(stack_t *s, size_t n)
{
  return stack_sp(s) + n >= stack_cap(s);
}

bool check_shrink(stack_t *s, size_t n)
{
  if (stack_cap(s) > N_STACK)
    return stack_sp(s) - n <  stack_cap(s) / 2;

  return false;
}

void stack_push(stack_t *s, value_t x)
{
  if (check_grow(s, 1))
    grow_stack(s);

  stack_data(s)[stack_sp(s)++] = x;
}

value_t stack_pop(stack_t *s)
{
  if (check_shrink(s, 1))
    shrink_stack(s);

  return stack_data(s)[--stack_sp(s)];
}

// heap implementation --------------------------------------------------------
#define N_HEAP 16384
#define GROW_F 0.625

#define heap_used(h)       ((h)->used)
#define heap_available(h)  ((h)->available)
#define heap_alignment(h)  ((h)->alignment)
#define heap_grow(h)       ((h)->grow)
#define heap_grew(h)       ((h)->grew)
#define heap_collecting(h) ((h)->collecting)
#define heap_space(h)      ((h)->space)
#define heap_swap(h)       ((h)->swap)
#define heap_size(h)       (heap_available(h)*heap_alignment(h))
#define heap_allocated(h)  (heap_used(h)*heap_alignment(h))
#define heap_next(h)       (heap_space(h)+heap_allocated(h))

void heap_collect(heap_t *h);

void init_heap(heap_t *h)
{
  heap_used(h) = 0;
  heap_available(h) = N_HEAP;
  heap_alignment(h) = sizeof(cons_t);
  heap_grow(h) = false;
  heap_grew(h) = false;
  heap_collecting(h) = false;
  heap_space(h) = malloc_s(heap_size(h));
  heap_swap(h) = malloc_s(heap_size(h));
}

void finalize_heap(heap_t *h)
{
  free_s( heap_space(h) );
  free_s( heap_swap(h) );
}

void grow_space(heap_t *h)
{
  heap_available(h) *= 2;
  heap_space(h) = realloc_s( heap_space(h), heap_size(h) );
  heap_grew(h) = true;
  heap_grow(h) = false;
}

void grow_swap(heap_t *h)
{
  heap_swap(h) = realloc_s( heap_swap(h), heap_size(h) );
  heap_grew(h) = false;
  heap_grow(h) = false;
}

void heap_resize(heap_t *h)
{
  if (heap_grow(h))
    grow_space(h);

  else if (heap_grew(h))
    grow_swap(h);
}

void check_resize(heap_t *h)
{
  heap_grow(h) = heap_used(h) >= heap_available(h) * GROW_F;
}

bool check_collect(heap_t *h, size_t n)
{
  return !heap_collecting(h) &&
          heap_allocated(h) + n < heap_size(h);
}

void *alloc_from(heap_t *h, size_t n)
{
  size_t a = aligned( n, heap_alignment(h) );

  if (check_collect(h, a))
    heap_collect(h);

  void *out = heap_next(h);
  heap_used(h) += n / heap_alignment(h);
  memset(out, 0, a);
  return out;
}

// misc runtime helpers -------------------------------------------------------
void gc_frame_cleanup(gc_frame_t *g)
{
  assert(g);
  Saved = g->next;
}

// external type implementations ----------------------------------------------
// descriptive macros ---------------------------------------------------------
#define tag_test(type, mask, tag)		\
  bool is##type(value_t x)			\
  {						\
    return (x&mask) == tag;			\
  }

#define value_test(type, value)			\
  bool is##type(value_t x)			\
  {						\
    return x == value;				\
  }

// cons implementation --------------------------------------------------------
value_t cons(value_t ca, value_t cd)
{
  preserve(2, &ca, &cd);

  cons_t *out = alloc_from(Heap, sizeof(cons_t));
  out->car = ca; out->cdr = cd;
  return tagptr(out, LIST);
}

bool iscons(value_t x)
{
  return !!pbits(x) && hitag(x) == LIST;
}

tag_test(list,HITAG,LIST)
value_test(nil,NIL)

// symbol implementation ------------------------------------------------------

// function implementation ----------------------------------------------------
#define fun_code(x) getf(function, x, code)
#define fun_vals(x) getf(function, x, vals)
#define fun_envt(x) getf(function, x, envt)

// string implementation ------------------------------------------------------
#define str_length(x) getf(string, x, length)
#define str_space(x) (&getf(string, x, space)[0])
#define str_instr(x) ((short*)str_space(x))

#define instructions(x) ((short*)(&(x)->space[0]))

// vector implementation ------------------------------------------------------

// table implementation -------------------------------------------------------

// interpreter ----------------------------------------------------------------
typedef enum
  {
   NUM_BUILTINS
  } builtin_t;

typedef enum
  {
   // exit point ------------------------------------------------------------
   OP_HALT=NUM_BUILTINS,

   // stack manipulation ----------------------------------------------------
   OP_PUSH, OP_POP,
   
   // load/store instructions -----------------------------------------------
   OP_LOAD_VALUE,

   OP_LOAD_LOCAL, OP_STORE_LOCAL,
   
   OP_LOAD_GLOBAL, OP_STORE_GLOBAL,
   
   OP_LOAD_UPVALUE, OP_STORE_UPVALUE,
   
   // capture and closure instructions --------------------------------------
   OP_CLOSURE,
   
   OP_CAPTURE_LOCAL, OP_CAPTURE_UPVALUE,
   
   // control flow ----------------------------------------------------------
   OP_JUMP, OP_JUMP_TRUE, OP_JUMP_FALSE,
   
   // function calls --------------------------------------------------------
   OP_CALL, OP_RETURN,
   
   NUM_INSTRUCTIONS
  } opcode_t;

value_t eval(value_t bytecode)
{
  static void *labels[NUM_INSTRUCTIONS] =
    {
     [OP_HALT] = &&op_halt,

     [OP_PUSH] = &&op_push, [OP_POP] = &&op_pop,

     [OP_CLOSURE] = &&op_closure,
     [OP_CAPTURE_LOCAL] = &&op_capture_local, [OP_CAPTURE_UPVALUE] = &&op_capture_upvalue,
    };

  string_t *code;
  vector_t *vals;

  ushort op; short argx, argy;

  assert(Fun == UNBOUND);

 do_call:
  code = pval(fun_code(bytecode));
  vals = pval(fun_vals(bytecode));
  
 do_dispatch:
  op = instructions(code)[Ctl++];

  goto *labels[op];

 op_push:
  stack_push(Stack, Val);
  goto do_dispatch;

 op_pop:
  Val = stack_pop(Stack);
  goto do_dispatch;

 op_load_local:
  Val = stack_ref(Stack, Arg+argx);
  goto do_dispatch;

 op_store_local:
  stack_ref(Stack, Arg+argx) = Val;
  goto do_dispatch;

 op_halt:
  return Val;
}

// compiler -------------------------------------------------------------------
