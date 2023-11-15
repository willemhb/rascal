#include "vm/context.h"

#include "val/envt.h"

/* Globals */
#define N_VALUES 65536
#define N_FRAMES 16384

/* Value stack and call stack. */
Value     ValueStack[N_VALUES];
ExecFrame CallStack[N_FRAMES];

/* External API */
Value* rl_peek(int n) {
  if (n < 0)
    n += Ctx.sp;

  return &ValueStack[n];
}

void rl_push(Value val) {
  ValueStack[Ctx.sp++] = val;
}

Value rl_pop(void) {
  return ValueStack[--Ctx.sp];
}

UpValue* get_upval(size_t i) {
  /* 
   * Find or create an open upvalue 
   */

  Value*    location = &ValueStack[Ctx.bp+i];
  UpValue** upvals = &Ctx.upvals;

  while (*upvals != NULL) {
    if ((*upvals)->location == location)
      break;

    else if ((*upvals)->location < location) {
      *upvals = mk_upval(location, *upvals);
      break;
    }
    else
      upvals = &(*upvals)->next;
  }

  if (*upvals == NULL)
    *upvals = mk_upval(location, NULL);

  return *upvals;
}

void close_upvals(size_t bp) {
  /* 
   * Preserve all upvalues whose stack position is greater than or equal to 
   * the given base pointer. 
   */

  Value* base = &ValueStack[bp];

  while (Ctx.upvals != NULL && Ctx.upvals->location >= base) {
    UpValue* tmp = Ctx.upvals;
    Ctx.upvals = tmp->next;
    tmp->value = *tmp->location;
    tmp->location = NULL;
    tmp->next = NULL;
  }
}
