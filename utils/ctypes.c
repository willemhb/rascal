#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "numutils.h"
#include "ctypes.h"

size_t C_type_size(C_type_t xct) { // total size used, in bytes
  size_t out = 1 << (xct & C_size_mask);

  if ((xct & C_qual_mask) == C_complex)
    out *= 2;

  return out;
}

inline bool C_type_signedp(C_type_t xct) {
  return !!(xct & C_sign_mask);
}

inline bool C_type_intp(C_type_t xct) {
  return (xct & C_qual_mask) == 0;
}

inline bool C_type_uintp(C_type_t xct) {
  return (xct & (C_qual_mask|C_sign_mask)) == 0;
}

inline bool C_type_sintp(C_type_t xct) {
  return (xct & (C_qual_mask|C_sign_mask)) == C_signed;
}

inline bool C_type_floatp(C_type_t xct) {
  return (xct & C_qual_mask) == C_float;
}

inline bool C_type_imagp(C_type_t xct) {
  return (xct & C_qual_mask) == C_imaginary;
}

inline bool C_type_cplxp(C_type_t xct) {
  return (xct & C_qual_mask) == C_complex;
}

inline bool C_type_nump(C_type_t xct) {
  return (xct & C_qual_mask) < C_character;
}

inline bool C_type_charp(C_type_t xct) {
  return (xct & C_qual_mask) == C_character;
}

inline bool C_type_ptrp(C_type_t xct) {
  return (xct & C_qual_mask) == C_pointer;
}

inline bool C_type_voidp(C_type_t xct) {
  return (xct & C_qual_mask) == C_void;
}

C_type_t C_type_common(C_type_t xct, C_type_t yct) {
  int xsize = xct & C_size_mask, ysize = yct & C_size_mask, zsize = max(xsize, ysize);
  int xqual = xct & C_qual_mask, yqual = yct & C_qual_mask, zqual = xqual|yqual;
  int xsign = xct & C_sign_mask, ysign = yct & C_sign_mask, zsign = xsign|ysign;

  if (xsign == C_signed && ysign != C_signed && ysize >= xsize)
    zsize++;

  if (ysign == C_signed && xsign != C_signed && xsize >= ysize)
    zsize++;

  return zqual|zsign|zsize;
}
