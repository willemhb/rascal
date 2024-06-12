#include <string.h>
#include <stdlib.h>

#include "util.h"

// BinaryType utilities
size_t bt_size(BinaryType bt) {
  size_t out;

  switch ( bt ) {
    case VOID:               out = 0; break;
    case SINT8 ... UTF8:     out = 1; break;
    case SINT16 ... UTF16:   out = 2; break;
    case SINT32 ... UTF32:   out = 4; break;
    case SINT64 ... U32PTR:  out = 8; break;
  }

  return out;
}

bool bt_is_encoded(BinaryType bt) {
  bool out;

  switch ( bt ) {
    case ASCII:
    case LATIN1:
    case UTF8:
    case UTF16:
    case UTF32:
      out = true; break;

    default:
      out = false; break;
  }

  return out;
}



// miscellaneous numeric utilities
word_t ceil2(word_t w);

// file system utilities
const char* home_dir(void) {
  return getenv("HOME");
}

size_t abs_path(const char* path, char* buffer, size_t buffer_size);

// string utilities
size_t strsz(const char* s) {
  return strlen(s) + 1;
}

bool   streq(const char* sx, const char* sy) {
  return strcmp(sx, sy) == 0;
}

size_t strcnt(const char* s) {
  return strlen(s) + 1;
}

// collection utilities
size_t adjust_stack_size(size_t new_count, size_t max_count);
size_t adjust_alist_size(size_t old_count, size_t new_count, size_t max_count);
size_t adjust_table_size(double load_factor, size_t new_count, size_t max_count);

// hashing utilities
