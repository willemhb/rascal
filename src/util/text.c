#include <stdlib.h>
#include <string.h>

#include "util/str.h"
#include "util/memory.h"


size_t ct_size(CType bt) {
  size_t r;
  
  switch ( bt ) {
    case VOID:             r = 0; break;
    case SINT8...UTF8:     r = 1; break;
    case SINT16...UTF16:   r = 2; break;
    case SINT32...UTF32:   r = 4; break;
    case SINT64...FLOAT64: r = 8; break;
  }

  return r;
}

bool ct_is_encoded(CType bt) {
  bool r;

  switch ( bt ) {
    default:           r = false; break;
    case ASCII...UTF8: r = true;  break;
    case UTF16:        r = true;  break;
    case UTF32:        r = true;  break;
  }

  return r;
}

bool ct_is_multibyte(CType bt) {
  bool r;

  switch ( bt ) {
    default:    r = false; break;
    case UTF8:  r = true;  break;
    case UTF16: r = true;  break;
  }

  return r;
}

bool ct_is_unsigned(CType bt) {
  bool r;

  switch ( bt ) {
    default:     r = false; break;
    case UINT8:  r = true;  break;
    case UINT16: r = true;  break;
    case UINT32: r = true;  break;
    case UINT64: r = true;  break;
  }

  return r;
}


bool ct_is_signed(CType bt) {
  bool r;

  switch ( bt ) {
    default:     r = false; break;
    case SINT8:  r = true;  break;
    case SINT16: r = true;  break;
    case SINT32: r = true;  break;
    case SINT64: r = true;  break;
  }

  return r;  
}

bool ct_is_float(CType bt) {
  bool r = bt == FLOAT32 || bt == FLOAT64;

  return r;
}


// string utilities
void cleanup_str(char** s) {
  if ( *s )
    s_free(*s);
}

char* str(char* ini, size_t n) {
  char* r;
  
  if ( ini == NULL )
    r = s_malloc(n+1, '\0');

  else {
    if ( n == 0 )
      n = strlen(ini);

    r = s_malloc(n+1, '\0');

    strncpy(r, ini, n);
  }

  return r;
}

bool streql(char* sx, char* sy) {
  return strcmp(sx, sy) == 0;
}
