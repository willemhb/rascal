#ifndef types_text_h
#define types_text_h

#include <stdio.h>

#include "types/object.h"

/* text, character, binary, and io types */

/* C types */
// stream type ----------------------------------------------------------------
typedef enum StreamFl {
  INPUT    =1,
  OUTPUT   =2,
  BINARY   =4,
  TEXT     =8,
  CREATE   =16,
  UPDATE   =32
} StreamFl;

struct Stream {
  Obj   obj;
  FILE* ios;
};

// bin type -------------------------------------------------------------------
typedef enum BinFl {
  ENCODED=1,
} BinFl;

struct Bin {
  Obj obj;

  void *array;
  uint  count, cap;
  uhash hash;
};

/* globals */
extern Bin EmptyString;
extern Stream Stdin, Stdout, Stderr;

/* API */
// stream api -----------------------------------------------------------------
Stream* sopen(flags fl, Val fname);
int     sclose(Stream* stream);
bool    seof(Stream* stream);
int     sputc(Stream* stream, int c);
int     sgetc(Stream* stream);
int     sungetc(Stream* stream, int c);
int     speekc(Stream* stream);
int     sfmt(Stream* stream, char* fmt, ...);
int     vsfmt(Stream* stream, char* fmt, va_list va);

// bin api --------------------------------------------------------------------
Bin*  bytes(uint n, ubyte* bytes);
Bin*  string(char* chars);
Bin*  bytecode(uint n, uint16* code);

bool  is_string(Bin* bin);

void  resize_bin(Bin* self, uint n);
void *bin_peep(Bin* self, int i);
ubyte bin_ref(Bin* self, int i);
ubyte bin_set(Bin* self, int i, ubyte byte);
usize bin_write(Bin* self, usize n, void* data);

// initialization -------------------------------------------------------------
void text_init(void);

#endif
