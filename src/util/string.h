#ifndef util_string_h
#define util_string_h

#include <stdarg.h>

#include "common.h"

// generic memory comparison --------------------------------------------------
int  u8cmp(uint8* xs, uint8* ys, usize n);
int  u16cmp(uint16* xs, uint16* ys, usize n);
int  u32cmp(uint32* xs, uint32* ys, usize n);
long u64cmp(uint64* xs, uint64* ys, usize n);

// create a string of indeterminate size --------------------------------------
char* strfmt(char* fmt, ...);
char* vstrfmt(char* fmt, va_list va);

#endif
