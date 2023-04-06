#include "value.h"
#include "object.h"

valtype_t valtype(value_t x) {
  switch (x & WTMASK) {
    case IMMTAG: return x >> 32 & 0xff;
    case FIXTAG: return FIXNUM;
    case PTRTAG: return POINTER;
    case OBJTAG: return OBJECT;
    default:     return REAL;
  }
}

objtype_t objtype(object_t* ox) {
  return ox->type;
}

value_t object(void* px) {
  return ((uword)px & WVMASK) | OBJTAG;
}

value_t real(real_t rx) {
  return ((ieee64_t)rx).word;
}

value_t fixnum(fixnum_t fx) {
  return (fx & WVMASK) | FIXNUM;
}

value_t sint(sint32_t ix) {
  return (uword)ix | SINT32TAG;
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

type_t* val_type_of(value_t vx) {
  valtype_t vt = valtype(vx);

  if (vt == OBJECT)
    return obj_type_of(as_object(vx));

  return Types[vt];
}

type_t* obj_type_of(object_t* ox) {
  assert(ox);

  return Types[ox->type];
}

bool val_isa(value_t x, type_t* tx) {
  uint64 idno = tx->idno;

  if (idno < BOTTOM)
    return rltype_of(x) == tx;

  return idno != BOTTOM;
}

