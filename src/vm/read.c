#include "vm/memory.h"
#include "vm/error.h"
#include "vm/read.h"

#include "val/ptr.h"
#include "val/text.h"
#include "val/stream.h"
#include "val/table.h"
#include "val/vector.h"

/* Global state and external APIs used by the reader. */

/* Globals */
MutDict ReadTable = {
  .obj={
    .type =&MutDictType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|GRAY,
    .flags=FASTHASH,
  },
  .data=NULL,
  .cnt =0,
  .cap =0,
  .nts =0,
};

MutVec ReadStack = {
  .obj={
    .type =&MutVecType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|GRAY,
  },
  .data=NULL,
  .cnt =0,
  .cap =0,
};

MutStr ReadBuf = {
  .obj={
    .type =&MutStrType,
    .meta =&EmptyDict,
    .memfl=NOSWEEP|GRAY,
    .flags=ASCII,
  },
  .c8 =NULL,
  .cnt=0,
  .cap=0,
};

#define N_VALUES 65536
#define N_FRAMES 16384

/* Value stack and call stack. */
Value     ReaderValues[N_VALUES];
ReadFrame ReaderFrames[N_FRAMES];

/* internal API */
static void mark_reader_frame(ReadFrame* f) {
  mark(f->rt);
  mark(f->gs);
  mark(f->buf);
  mark(f->src);
}

static void mark_reader_frames(void) {
  for (size_t i=0; i<Ctx.r.sp; i++)
    mark_reader_frame(&ReaderFrames[i]);
}

/* External API */
Value* peek_reader_val(int n) {
  if (n < 0)
    n += Ctx.r.sp;
  
  bound_gel(0, Ctx.r.sp, n, "<runtime:peek-reader-val>", "<reader:stack>");
  
  return &ReaderValues[n];
}

void push_reader_val(Value val) {
  bound_lt(N_VALUES, Ctx.r.sp, "<runtime:push-reader-val>", "<reader:stack>");
  ReaderValues[Ctx.r.sp++] = val;
}

Value pop_reader_val(void) {
  bound_gt(0, Ctx.r.sp, "<runtime:pop-reader-val>", "<reader:stack>");
  return ReaderValues[--Ctx.r.sp];
}

ReadFrame* peek_reader_frame(int n) {
  if (n < 0)
    n += Ctx.r.fp;

  bound_gel(0, Ctx.r.fp, n, "<runtime:pop-reader-frame>", "<reader:frames>");

  return &ReaderFrames[n];
}

void push_reader_frame(void) {
  bound_lt(N_FRAMES, Ctx.r.fp, "<runtime:push-reader-frame>", "<reader:frames>");

  ReaderFrames[Ctx.r.fp++] = (ReadFrame) {
    .rt =Ctx.r.rt,
    .gs =Ctx.r.gs,
    .buf=Ctx.r.buf,
    .src=Ctx.r.src
  };
}

void pop_reader_frame(void) {
  bound_gt(0, Ctx.r.fp, "<runtime:pop-reader-frame>", "<reader:frames>");

  ReadFrame* frame = &ReaderFrames[--Ctx.r.fp];

  Ctx.r.rt =frame->rt;
  Ctx.r.gs =frame->gs;
  Ctx.r.buf=frame->buf;
  Ctx.r.src=frame->src;
}

ReadFn get_readfn(Glyph dispatch) {
  Value disp   = tag(dispatch);
  Value readfn = mdict_get(Ctx.r.rt, disp);

  if (readfn == NOTHING)
    return NULL;

  return (ReadFn)as_fptr(readfn);
}

size_t reader_accumgl(Glyph gl) {
  return mstr_add(Ctx.r.buf, gl);
}

char* reader_token(void) {
  return Ctx.r.buf->c8;
}

void vm_mark_reader(void) {
  mark(Ctx.r.rt);
  mark(Ctx.r.gs);
  mark(Ctx.r.src);
  mark(Ctx.r.buf);

  mark_reader_frames();
}

/* Initialization */
void vm_init_reader(void) {
  Ctx.r = (Reader) {
    .rt =&ReadTable,
    .gs =NULL,
    .src=&Ins,
    .buf=&ReadBuf,
    .sp =0,
    .fp =0
  };
}
