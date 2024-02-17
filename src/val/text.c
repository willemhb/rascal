#include "util/hashing.h"
#include "util/number.h"

#include "vm/memory.h"
#include "vm/context.h"

#include "val/type.h"
#include "val/symbol.h"
#include "val/text.h"

/* Globals */
/* Magic numbers */
#define MAX_INTERNED_SIZE 1024

/* Metaobjects */
void   finalize_str(void* obj);
void*  clone_str(void* obj);
hash_t hash_str(Value x);
bool   egal_strs(Value x, Value y);
int    order_strs(Value x, Value y);

INIT_OBJECT_TYPE(String,
                 .finalizefn=finalize_str,
                 .clonefn   =clone_str,
                 .hashfn    =hash_str,
                 .egalfn    =egal_strs,
                 .ordfn     =order_strs);

void   finalize_bin(void* obj);
void*  clone_bin(void* obj);
hash_t hash_bin(Value x);
bool   egal_bins(Value x, Value y);
int    order_bins(Value x, Value y);

INIT_OBJECT_TYPE(Binary,
                 .finalizefn=finalize_bin,
                 .clonefn   =clone_bin,
                 .hashfn    =hash_bin,
                 .egalfn    =egal_bins,
                 .ordfn     =order_bins);

/* Global singletons */
String EmptyString = {
  .obj={
    .type =&StringType,
    .meta =&EmptyDict,
    .flags=GRAY|NOSWEEP|NOFREE
  },
  .chars="",
  .arity=0
};

Binary EmptyBinary = {
  .obj={
    .type =&BinaryType,
    .meta =&EmptyDict,
    .flags=GRAY|NOSWEEP|NOFREE
  },
  .bytes=NULL,
  .arity=0
};

/* Internal APIs */
/* String lifetime and comparison methods */
void finalize_str(void* obj) {
  String* str = obj;

  deallocate(str->chars, 0, false);
}

void*  clone_str(void* obj) {
  // how/where to check whether this string is interned, and how to handle that?
  String* out = duplicate(obj, sizeof(String), true);
  out->chars  = duplicates(out->chars, out->arity, false);

  return out;
}

hash_t hash_str(Value x) {
  String* s = as_str(x);

  return hash_string(s->chars);
}

bool egal_strs(Value x, Value y) {
  String* sx = as_str(x), * sy = as_str(y);

  // avoid costly strcmp if arities aren't the same
  return sx->arity == sy->arity && strcmp(sx->chars, sy->chars) == 0;
}

int order_strs(Value x, Value y) {
  String* sx = as_str(x), * sy = as_str(y);

  return strcmp(sx->chars, sy->chars);
}

void finalize_bin(void* obj) {
  Binary* bin = obj;

  deallocate(bin->bytes, 0, false);
}

void* clone_bin(void* obj) {
  Binary* bin = duplicate(obj, sizeof(Binary), true);
  bin->bytes  = duplicate(bin->bytes, bin->arity, false);

  return bin;
}

hash_t hash_bin(Value x) {
  Binary* bin = as_bin(x);

  return hash_bytes(bin->bytes, bin->arity);
}

bool egal_bins(Value x, Value y) {
  Binary* bx = as_bin(x), * by = as_bin(y);

  if (bx->arity == by->arity) {
    if (bx == 0)
      return true;

    return memcmp(bx->bytes, by->bytes, bx->arity) == 0;
  }

  return false;
}

int order_bins(Value x, Value y) {
  Binary* bx = as_bin(x), * by = as_bin(y);

  if (bx->arity == 0)
    return -1;

  else if (by->arity == 0)
    return 1;

  else
    return memcmp(bx->bytes, by->bytes, min(bx->arity, by->arity));
}


/* External APIs */
String* get_str(const char* data, size_t n) {
  if (n <= MAX_INTERNED_SIZE)
    return intern_string(data, NULL);

  return new_str(data, n, 0);
}

String* new_str(const char* data, size_t n, hash_t h) {
  String* out;
  char* buffer;

  if (h == 0)
    h = hash_string(data);

  buffer     = allocate(n+1, false);
  out        = new_obj(&StringType, 0, 0);
  out->chars = buffer;
  out->arity = n;

  strncpy(buffer, data, n);
  set_obj_hash(out, h);

  return out;
}

Value str_ref(String* str, size_t n) {
  assert(n < str->arity);

  return tag(str->chars[n]);
}

Binary* new_bin(const byte_t* data, size_t n) {
  if (n == 0)
    return &EmptyBinary;

  byte_t* buffer = duplicate(data, n, false);
  Binary* out    = new_obj(&BinaryType, 0, 0);
  out->bytes     = buffer;
  out->arity     = n;

  return out;
}

Value bin_ref(Binary* bin, size_t n) {
  assert(n < bin->arity);

  return tag(bin->bytes[n]);
}
