#ifndef rl_vm_reader_h
#define rl_vm_reader_h

#include "val/value.h"

/* Global state and external APIs used by the reader. */

/* Globals */
extern Value     ReaderValues[];
extern ReadFrame ReaderFrames[];

/* External API */
Value* peek_reader_val(int n);
void   push_reader_val(Value val);
Value  pop_reader_val(void);

ReadFrame* peek_reader_frame(int n);
void       push_reader_frame(void);
void       pop_reader_frame(void);

ReadFn get_readfn(Glyph gl);
size_t reader_accumgl(Glyph gl);
char*  reader_token(void);

void vm_mark_reader(void);

/* Initialization */
void vm_init_reader(void);

#endif
