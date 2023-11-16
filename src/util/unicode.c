#include "util/unicode.h"


/* External API */
bool is_mb(Encoding enc) {
  bool out;
  
  switch (enc) {
    case BINARY: out = false; break;
    case ASCII:  out = false; break;
    case LATIN1: out = false; break;
    case UTF8:   out = true;  break;
    case UTF16:  out = true;  break;
    case UTF32:  out = false; break;
  }

  return out;
}

size_t enc_nbytes(Encoding enc) {
  size_t out;

  switch (enc) {
    case BINARY: out = 1; break;
    case ASCII:  out = 1; break;
    case LATIN1: out = 1; break;
    case UTF8:   out = 1; break;
    case UTF16:  out = 2; break;
    case UTF32:  out = 4; break;
  }

  return out;
}

CType enc_Ctype(Encoding enc) {
  CType out;

  switch (enc) {
    case BINARY: out = UINT8;  break;
    case ASCII:  out = SINT8;  break;
    case LATIN1: out = UINT8;  break;
    case UTF8:   out = UINT8;  break;
    case UTF16:  out = UINT16; break;
    case UTF32:  out = UINT32; break;
  }

  return out;
}

size_t Ctype_nbytes(CType Ct) {
  size_t out;

  switch (Ct) {
    case VOID:   out = 0; break;
    case UINT8:  out = 1; break;
    case UINT16: out = 2; break;
    case UINT32: out = 4; break;
    case UINT64: out = 8; break;
    case SINT8:  out = 1; break;
    case SINT16: out = 2; break;
    case SINT32: out = 4; break;
    case SINT64: out = 8; break;
    case FLT32:  out = 4; break;
    case FLT64:  out = 8; break;
    case ANYPTR: out = 8; break;
    case FUNPTR: out = 8; break;
  }

  return out;
}
