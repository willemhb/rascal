#include "vm/reader.h"


/* Globals */
char IText[MAX_STACK] = {};
byte IBins[MAX_STACK] = {};

MStr TBuffer = {
  .type     = &MStrType,
  .trace    = true,
  .free     = true,
  .sweep    = false,
  .gray     = true,
  
  .encoding = ASCII,
  .hasmb    = false,
  
  .algo     = RESIZE_STACK,
  .data     = IText,
  ._static  = IText,
  .cnt      = 0,
  .maxc     = MAX_STACK,
  .maxs     = MAX_STACK
};

MBin IBuffer = {
  .type     = &MStrType,
  .trace    = true,
  .free     = true,
  .sweep    = false,
  .gray     = true,
  
  .eltype   = UINT8,
  
  .algo     = RESIZE_STACK,
  .data     = IBins,
  ._static  = IBins,
  .cnt      = 0,
  .maxc     = MAX_STACK,
  .maxs     = MAX_STACK
};
