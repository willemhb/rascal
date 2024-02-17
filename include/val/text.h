#ifndef rl_val_text_h
#define rl_val_text_h

#include "util/unicode.h"

#include "val/object.h"

/* Declarations & APIs for text, binary, and string types. */

/* C types */
struct String {
  HEADER;
  char*  chars;
  size_t arity;
};

struct Binary {
  HEADER;
  byte_t* bytes;
  size_t  arity;
};

/* Globals */
/* Types */
extern Type StringType, BinaryType;

/* Empty singletons */
extern String EmptyString;
extern Binary EmptyBinary;

/* External APIs */
/* String API */
#define is_str(x) has_type(x, &StringType)
#define as_str(x) as(String*, untag48, x)

String*  get_str(const char* data, size_t n);
String*  new_str(const char* data, size_t n, hash_t h);
Value    str_ref(String* str, size_t n);

/* Binary API */
#define is_bin(x) has_type(x, &BinaryType)
#define as_bin(x) as(Binary*, untag48, x)

Binary* new_bin(const byte_t* data, size_t n);
Value   bin_ref(Binary* bin, size_t n);

#endif
