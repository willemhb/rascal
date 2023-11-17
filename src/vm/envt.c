#include "vm/memory.h"
#include "vm/interpreter.h"
#include "vm/envt.h"

#include "val/envt.h"
#include "val/table.h"

/* Globals */
/* global variables */
MutDict Globals = {
  .obj={
    .type =&MutDictType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|GRAY,
    .flags=FASTHASH,
  },
  .data=NULL,
  .cnt =0,
  .cap =0,
  .nts =0
};

/* value metadata */
MutDict MetaData = {
  .obj={
    .type =&MutDictType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|GRAY,
    .flags=FASTHASH,
  },
  .data=NULL,
  .cnt =0,
  .cap =0,
  .nts =0
};

/* External API */
UpValue* get_upval(size_t i) {
  /* 
   * Find or create an open upvalue
   */

  Value*    location = &InterpreterValues[Ctx.i.bp+i];
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

  Value* base = &InterpreterValues[bp];

  while (Ctx.upvals != NULL && Ctx.upvals->location >= base) {
    UpValue* tmp = Ctx.upvals;
    Ctx.upvals = tmp->next;
    tmp->value = *tmp->location;
    tmp->location = NULL;
    tmp->next = NULL;
  }
}

void vm_mark_envt(void) {
  /* Mark global objects and metadata. */
  mark(Ctx.globals);
  mark(Ctx.meta);
}

/* Initialization */
void vm_init_envt(void) {
  Ctx.globals = &Globals;
  Ctx.meta    = &MetaData;
  Ctx.upvals  = NULL;
}
