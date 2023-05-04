#include "data/object.h"

#include "runtime/memory.h"

// APIs & utilities
value_t object(void* ox) {
  return ((uword)ox) | OBJTAG;
}

type_t objtype(void* ox) {
  assert(ox);

  return head(ox)->type;
}

type_t obj_typeof(void* ox) {
  return objtype(ox);
}

usize obj_sizeof(void* ox) {
  assert(ox);

  return SIZEOF(ox)(ox);
}

bool obj_isa(void* ox, type_t tx) {
  if (tx == TOP)
    return true;

  if (tx == BOTTOM)
    return false;

  return objtype(ox) == tx;
}

void obj_mark(void* ox) {
  if (ox && !head(ox)->black) {
    head(ox)->black = true;

    if (MARK(ox))
      push_gray(ox);

    else
      head(ox)->gray = false;
  }
}

void obj_free(void* ox) {
  if (!ox)
    return;

  usize to_free = rl_sizeof(ox);

  if (FREE(ox))
    FREE(ox)(ox);

  if (!has_flag(ox, STATIC))
    deallocate(ox, to_free);
}

bool has_flag(void* ox, flags fl) {
  assert(ox);

  return flagp(head(ox)->flags, fl);
}

bool set_flag(void* ox, flags fl) {
  assert(ox);

  bool out = !flagp(head(ox)->flags, fl);
  head(ox)->flags |= fl;

  return out;
}

bool del_flag(void* ox, flags fl) {
  assert(ox);

  bool out = flagp(head(ox)->flags, fl);
  head(ox)->flags &= ~fl;

  return out;
}

bool has_wflag(void* ox, flags fl, flags m) {
  assert(ox);

  return wflagp(head(ox)->flags, fl, m);
}

bool set_wflag(void* ox, flags fl, flags m) {
  assert(ox);

  bool out = !wflagp(head(ox)->flags, fl, m);
  head(ox)->flags &= ~m;
  head(ox)->flags |= fl;

  return out;
}

bool del_wflag(void* ox, flags fl, flags m) {
  assert(ox);

  bool out = wflagp(head(ox)->flags, fl, m);
  head(ox)->flags &= ~m;
  return out;
}

void set_hash(void* ox, uhash h) {
  assert(ox);

  head(ox)->hash   = h;
  head(ox)->flags |= HASHED;
}

void mark_objects(usize n, void* oxs) {
  void **objects = oxs;

  for (usize i=0; i<n; i++)
    obj_mark(objects[i]);
}

// globals
extern void mark_symbol(void* ox);
extern void mark_list(void* ox);
extern void mark_table(void* ox);
extern void mark_vector(void* ox);
extern void mark_function(void* ox);
extern void mark_native(void* ox);
extern void mark_namespace(void* ox);
extern void mark_environment(void* ox);
extern void mark_chunk(void* ox);
extern void mark_closure(void* ox);
extern void mark_variable(void* ox);
extern void mark_upvalue(void* ox);

void (*Mark[NTYPES])(void* ox) = {
  [SYMBOL]      = mark_symbol,
  [LIST]        = mark_list,
  [TABLE]       = mark_table,
  [VECTOR]      = mark_vector,
  [FUNCTION]    = mark_function,
  [NATIVE]      = mark_native,
  [NAMESPACE]   = mark_namespace,
  [ENVIRONMENT] = mark_environment,
  [CHUNK]       = mark_chunk,
  [CLOSURE]     = mark_closure,
  [VARIABLE]    = mark_variable,
  [UPVALUE]     = mark_upvalue
};

extern void free_symbol(void* ox);
extern void free_binary(void* ox);
extern void free_table(void* ox);
extern void free_vector(void* ox);

void (*Free[NTYPES])(void* ox) = {
  [SYMBOL] = free_symbol,
  [BINARY] = free_binary,
  [TABLE]  = free_table,
  [VECTOR] = free_vector
};

extern usize sizeof_symbol(void* ox);
extern usize sizeof_binary(void* ox);
extern usize sizeof_list(void* ox);
extern usize sizeof_table(void* ox);
extern usize sizeof_vector(void* ox);
extern usize sizeof_function(void* ox);
extern usize sizeof_native(void* ox);
extern usize sizeof_namespace(void* ox);
extern usize sizeof_environment(void* ox);
extern usize sizeof_chunk(void* ox);
extern usize sizeof_closure(void* ox);
extern usize sizeof_variable(void* ox);
extern usize sizeof_upvalue(void* ox);

usize (*SizeOf[NTYPES])(void* ox) = {
  [SYMBOL]      = sizeof_symbol,
  [BINARY]      = sizeof_binary,
  [LIST]        = sizeof_list,
  [TABLE]       = sizeof_table,
  [VECTOR]      = sizeof_vector,
  [FUNCTION]    = sizeof_function,
  [NATIVE]      = sizeof_native,
  [NAMESPACE]   = sizeof_namespace,
  [ENVIRONMENT] = sizeof_environment,
  [CHUNK]       = sizeof_chunk,
  [CLOSURE]     = sizeof_closure,
  [VARIABLE]    = sizeof_variable,
  [UPVALUE]     = sizeof_upvalue
};
