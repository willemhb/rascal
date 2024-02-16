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
  byte_t* data;
  size_t  arity;
};

struct MutStr {
  HEADER;
  char* chars;
  size_t cnt;
  size_t cap;
};

struct MutBin {
  HEADER;
  byte_t* data;
  size_t cnt;
  size_t cap;
};

/* Globals */
/* Types */
extern Type StringType, MutStrType, MutBinType, GlyphType;

/* Empty singletons */
extern String EmptyString;
extern Binary EmptyBinary;

/* External APIs */
/* String API */
#define is_str(x)   has_type(x, &StringType)
#define as_str(x)   as(String*, untag48, x)

Encoding str_enc(String* str);
String*  intern_string(char* data, size_t n);
String*  new_str(char* data, size_t n);
Value    str_ref(String* str, size_t n);
String*  str_set(String* str, size_t n, Glyph gl);
String*  str_add(String* str, Glyph gl);
String*  str_write(String* str, size_t n, void *gls);
String*  str_del(String* str, size_t n);

/* MutStr API */
#define is_mstr(x)  has_type(x, &MutStrType)
#define as_mstr(x)  as(MutStr*, untag48, x)

Encoding mstr_enc(MutStr* mstr);
String*  new_mstr(Encoding enc);
Value    mstr_ref(MutStr* mstr, size_t n);
void     mstr_set(MutStr* mstr, size_t n, Glyph gl);
size_t   mstr_add(MutStr* mstr, Glyph gl);
size_t   mstr_write(MutStr* mstr, size_t n, void *gls);
size_t   mstr_del(MutStr* mstr, size_t n);

/* Binary API */
#define is_bin(x)  has_type(x, &BinaryType)
#define as_bin(x)  as(Binary*, untag48, x)

CType  bin_Ctype(Binary* bin);
size_t bin_elsize(Binary* bin);
Value  bin_ref(Binary* bin, size_t n);

/* MutBin API */
#define is_mbin(x) has_type(x, &MutBinType)
#define as_mbin(x) as(MutBin*, untag48, x)

CType  mbin_Ctype(MutBin* mbin);
size_t mbin_elsize(MutBin* mbin);
Value  mbin_ref(MutBin* mbin, size_t n);
void   mbin_set(MutBin* mbin, size_t n, uint64_t x);
size_t mbin_add(MutBin* mbin, size_t n, uint64_t x);
size_t mbin_write(MutBin* mbin, size_t n, void* d);


#endif
