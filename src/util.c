#include <string.h>

#include "util.h"


// miscellaneous numeric utilities
word_t ceil2(word_t w);

// file system utilities
const char* home_dir(void);
size_t abs_path(const char* path, char* buffer, size_t buffer_size);

// string utilities
size_t strsz(const char* s) {
  return strlen(s) + 1;
}

bool   streq(const char* sx, const char* sy) {
  return strcmp(sx, sy) == 0;
}

size_t strcnt(const char* s);

// collection utilities
size_t adjust_stack_size(size_t new_count, size_t max_count);
size_t adjust_alist_size(size_t old_count, size_t new_count, size_t max_count);
size_t adjust_table_size(double load_factor, size_t new_count, size_t max_count);

// hashing utilities
hash_t hash_word(word_t w);
hash_t hash_chars(const char* data);
hash_t hash_bytes(const byte_t* data, size_t count);
hash_t mix_hashes(hash_t hx, hash_t hy);
