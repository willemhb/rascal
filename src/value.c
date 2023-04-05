#include "value.h"


valtype_t valtype(value_t x) {
  switch (x & WTMASK) {
    case IMMTAG: return x >> 32 & 0xff;
    case FIXTAG: return FIXNUM;
    case PTRTAG: return POINTER;
    case OBJTAG: return OBJECT;
    default:     return REAL;
  }
}

value_t object(void* px) {
  return ((uword)px & WVMASK) | OBJTAG;
}

value_t fixnum(fixnum_t fx) {
  return (fx & WVMASK) | FIXNUM;
}

value_t sint(sint32_t ix) {
  return (uword)ix | SINT32TAG;
}

value_t real(real_t rx) {
  return ((ieee64_t)rx).word;
}

value_t pointer(pointer_t px) {
  return ((uword)px & WVMASK) | OBJTAG;
}

value_t boolean(boolean_t bx) {
  return bx ? TRUE : FALSE;
}

value_t ascii(ascii_t ax) {
  return (uword)ax | ASCIITAG;
}

