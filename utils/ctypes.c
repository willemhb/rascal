#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "numutils.h"
#include "ctypes.h"

size_t Ctype_size(Ctype_t xct) { // total size used, in bytes
  size_t out = 1 << (xct & C_size_mask);

  if ((xct & C_qual_mask) == C_complex)
    out *= 2;

  return out;
}

inline bool Ctype_signedp(Ctype_t xct) {
  return !!(xct & C_sign_mask);
}

inline bool Ctype_intp(Ctype_t xct) {
  return (xct & C_qual_mask) == 0;
}

inline bool Ctype_uintp(Ctype_t xct) {
  return (xct & (C_qual_mask|C_sign_mask)) == 0;
}

inline bool Ctype_sintp(Ctype_t xct) {
  return (xct & (C_qual_mask|C_sign_mask)) == C_signed;
}

inline bool Ctype_floatp(Ctype_t xct) {
  return (xct & C_qual_mask) == C_float;
}

inline bool Ctype_imagp(Ctype_t xct) {
  return (xct & C_qual_mask) == C_imaginary;
}

inline bool Ctype_cplxp(Ctype_t xct) {
  return (xct & C_qual_mask) == C_complex;
}

inline bool Ctype_nump(Ctype_t xct) {
  return (xct & C_qual_mask) < C_character;
}

inline bool Ctype_charp(Ctype_t xct) {
  return (xct & C_qual_mask) == C_character;
}

inline bool Ctype_ptrp(Ctype_t xct) {
  return (xct & C_qual_mask) == C_pointer;
}

inline bool Ctype_voidp(Ctype_t xct) {
  return (xct & C_qual_mask) == C_void;
}

Ctype_t Ctype_common(Ctype_t xct, Ctype_t yct) {
  int xsize = xct & C_size_mask, ysize = yct & C_size_mask, zsize = max(xsize, ysize);
  int xqual = xct & C_qual_mask, yqual = yct & C_qual_mask, zqual = xqual|yqual;
  int xsign = xct & C_sign_mask, ysign = yct & C_sign_mask, zsign = xsign|ysign;

  if (xsign == C_signed && ysign != C_signed && ysize >= xsize)
    zsize++;

  if (ysign == C_signed && xsign != C_signed && xsize >= ysize)
    zsize++;

  return zqual|zsign|zsize;
}
