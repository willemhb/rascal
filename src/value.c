#include <string.h>

#include "value.h"
#include "lang.h"
#include "interpreter.h"
#include "runtime.h"
#include "util.h"

// tagging dispatch
Value tag_num(Number n) {
  union {
    Number n;
    Value v;
  } ieee = { .n = n };

  return ieee.v;
}

Value tag_bool(Boolean b) {
  return b ? TRUE : FALSE;
}

Value tag_glyph(Glyph g) {
  return ((Value)g) | GLYPH;
}

Value tag_small(Small s) {
  return ((Value)s) | SMALL;
}

Value tag_nul(Nul n) {
  (void)n;

  return NUL;
}

Value tag_obj(void* p) {
  return ((Value)p) | OBJECT;
}

// value type predicates
bool is_num(Value x) {
  return (x & QNAN) != QNAN;
}

bool is_bool(Value x) {
  return tag_of(x) == BOOL;
}

bool is_nul(Value x) {
  return x == NUL;
}

bool is_obj(Value x) {
  return tag_of(x) == OBJECT;
}

// untagging
Object* val_as_obj(Value v) {
  assert(is_obj(v));

  return (Object*)(v & DATA_BITS);
}

Object* ptr_as_obj(void* p) {
  return p;
}

Number as_num(Value v) {
  assert(is_num(v));

  union {
    Number n;
    Value v;
  } ieee = { .v = v };

  return ieee.n;
}

Boolean as_bool(Value v) {
  assert(is_bool(v));

  return v == TRUE;
}

Nul as_nul(Value v) {
  assert(is_nul(v));

  return NULL;
}

// type_of dispatch
Type* type_of_val(Value v) {
  switch (tag_of(v)) {
    case BOOL:   return &BooleanType;
    case NUL:    return &NulType;
    case OBJECT: return type_of(as_obj(v));
    default:     return &NumberType;
  }
}

Type* type_of_obj(void* p) {
  assert(p != NULL);

  return ((Object*)p)->type;
}

Type* type_of_ptr(void* p) {
  return type_of_obj(p);
}

// type implementations & APIs
// object type (takes care of common initialization/allocation tasks)
Object* new_obj(Type* type) {
  Object* out;

  save(1, tag(type));

  allocate((void**)&out, type->object_size, true);

  init_obj(type, out, true);

  unsave(1);

  return out;
}

void init_obj(Type* type, Object* obj, bool in_heap) {
  obj->type = type;
  obj->gray = true;

  if (in_heap) {
    obj->trace = true;
    obj->sweep = true;
    obj->next = Heap.live_objects;
    Heap.live_objects = obj;
  }
}

// type type
hash_t hash_type(Value v) {
  Type* t = (Type*)as_obj(v);

  return hash_word(t->idno);
}

int order_types(Value x, Value y) {
  Type* tx = (Type*)as_obj(x), * ty = (Type*)as_obj(y);

  return 0 - (tx->idno < ty->idno) + (tx->idno > ty->idno);
}

Type TypeType = {
  .h={
    .next=NULL,
    .type=&TypeType,
    .gray=true,
    .trace=true
  },

  .idno=1,
  .value_type=OBJECT,
  .value_size=sizeof(Object*),
  .object_size=sizeof(Type),
  .hash=hash_type,
  .order=order_types
};

// string type
#define STR_LOADF         0.625
#define MAX_INTERNED_SIZE 1024

Strings StringCache = {
  .h={
    .type=&StringsType,
    .free=true
  },
  .entries=NULL,
  .count=0,
  .max_count=0
};

String* get_str(const char* chars) {
  String* intern(Strings* strings, StringsEntry* entry,
                 const char* chars, hash_t hash, void* data) {
    (void)strings;

    struct { size_t n; }* parms = data;

    String* out = new_str(chars, parms->n, hash);

    entry->key = out->data;
    entry->val = out;

    return out;
  }

  String* out;

  if (*chars == '\0')
    out = &EmptyString;

  else {
    size_t n = strcnt(chars);

    if (n <= MAX_INTERNED_SIZE) {
      struct { size_t n; } data = { n };
      out = strings_intern(&StringCache, chars, intern, &data);
    }
    
    else
      out = new_str(chars, n, 0);
  }

  return out;
}

String* new_str(const char* chars, size_t count, hash_t h) { 
  if (count == 0)
    count = strcnt(chars);

  if (h == 0)
    h = hash_chars(chars) & DATA_BITS;

  String* out = (String*)new_obj(&StringType);
  duplicate(chars, (void**)&out->data, count+1, false);
  
  out->h.hash = h;
  out->count = count;

  return out;
}

rl_status_t str_ref(Glyph* result, String* str, size_t i) {
  rl_status_t status = i < str->count ? OKAY : RUNTIME_ERROR;

  if (status == OKAY && result)
    *result = str->data[i];

  return status;
}


// symbol type
Symbol* get_sym(const char* namespace, const char* name, bool gensym) {
  String* ns = get_str(namespace);
  save(1, tag(ns));
  String* n = get_str(name);
  unsave(1);
  return new_sym(ns, n, gensym);
}

Symbol* new_sym(String* namespace, String* name, bool gensym) {
  static word_t gensym_counter = 1;

  save(2, tag(namespace), tag(name));

  Symbol* out = (Symbol*)new_obj(&SymbolType);

  out->namespace = namespace;
  out->name = name;
  out->idno = gensym ? gensym_counter++ : 0;

  /* compute hash */
  hash_t ns_hash = namespace->h.hash,
    n_hash = name->h.hash,
    sym_hash = mix_hashes(ns_hash, n_hash);

  if (out->idno)
    sym_hash = mix_hashes(sym_hash, hash_word(out->idno));

  unsave(2);

  return out;
}

// chunk type
Chunk* new_chunk(MutVec* vals, MutBin* instr) {
  size_t n_saved = save(2, tag(vals), tag(instr));
  Chunk* out = (Chunk*)new_obj(&ChunkType);

  n_saved += save(1, tag(out));

  out->instr = new_bin(instr->count, instr->data);
  out->vals = new_tuple(vals->count, vals->data);

  unsave(n_saved);

  return out;
}

#define DYNAMIC_ARRAY_IMPL(_Type, _X, _V, _type, _is_string)            \
  _Type* new_##_type(void) {                                            \
    _Type* out = (_Type*)new_obj(&_Type##Type);                         \
    init_##_type(out);                                                  \
    return out;                                                         \
  }                                                                     \
                                                                        \
  void init_##_type(_Type* _type) {                                     \
    _type->data = NULL;                                                 \
    _type->count = 0;                                                   \
    _type->max_count = 0;                                               \
  }                                                                     \
                                                                        \
  void free_##_type(_Type* _type) {                                     \
    deallocate(_type->data, 0, false);                                  \
    init_##_type(_type);                                                \
  }                                                                     \
                                                                        \
  void resize_##_type(_Type* _type, size_t new_count) {                 \
    if (new_count == 0)                                                 \
      free_##_type(_type);                                              \
                                                                        \
    else {                                                              \
      size_t old_max = _type->max_count;                                \
      size_t old_count = _type->count;                                  \
      size_t new_max = adjust_alist_size(old_count + _is_string,        \
                                         new_count + _is_string,        \
                                         old_max);                      \
                                                                        \
      if (old_max != new_max) {                                         \
        reallocate((void**)&_type->data,                                \
                   old_max * sizeof(_X),                                \
                   new_max * sizeof(_X),                                \
                   false);                                              \
        _type->max_count = new_max;                                     \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  void _type##_push(_Type* _type, _X x) {                               \
    resize_##_type(_type, _type->count+1);                              \
    _type->data[_type->count++] = x;                                    \
  }                                                                     \
                                                                        \
  void _type##_pushn(_Type* _type, size_t n, ...) {                     \
    _X buffer[n];                                                       \
                                                                        \
    va_list va;                                                         \
    va_start(va, n);                                                    \
                                                                        \
    for (size_t i=0; i<n; i++)                                          \
      buffer[i] = va_arg(va, _V);                                       \
                                                                        \
    va_end(va);                                                         \
                                                                        \
    _type##_write(_type, n, buffer);                                    \
  }                                                                     \
                                                                        \
  void _type##_write(_Type* _type, size_t n, _X* source) {              \
    resize_##_type(_type, _type->count+n);                              \
                                                                        \
    _X* dest = _type->data + _type->count;                              \
                                                                        \
    memcpy(dest, source, n * sizeof(_X));                               \
    _type->count += n;                                                  \
  }                                                                     \
                                                                        \
  rl_status_t _type##_pop(_X* result, _Type* _type) {                   \
    rl_status_t status = _type->count > 0 ? OKAY : RUNTIME_ERROR;       \
                                                                        \
    if (status == OKAY) {                                               \
      size_t new_count = _type->count - 1;                              \
      if (result)                                                       \
        *result = _type->data[new_count];                               \
      resize_##_type(_type, new_count);                                 \
      _type->count = new_count;                                         \
    }                                                                   \
                                                                        \
    return status;                                                      \
  }                                                                     \
                                                                        \
  rl_status_t _type##_popn(_X* result, _Type* _type, size_t n) {        \
    rl_status_t status = n <= _type->count ? OKAY : RUNTIME_ERROR;      \
                                                                        \
    if (status == OKAY) {                                               \
      size_t new_count = _type->count - n;                              \
                                                                        \
      if (result)                                                       \
        *result = _type->data[new_count];                               \
                                                                        \
      resize_##_type(_type, new_count);                                 \
      _type->count = new_count;                                         \
    }                                                                   \
                                                                        \
    return status;                                                      \
  }

DYNAMIC_ARRAY_IMPL(Objects, Object*, Object*, objects, false);
DYNAMIC_ARRAY_IMPL(MutVec, Value, Value, mut_vec, false);
DYNAMIC_ARRAY_IMPL(MutStr, char, int, mut_str, true);
DYNAMIC_ARRAY_IMPL(MutBin, byte_t, int, mut_bin, false);

#undef DYNAMIC_ARRAY_IMPL

#define DEFAULT_LOAD_FACTOR 0.625

#define MUTABLE_HASH_TABLE_IMPL(_Type, _K, _V, _type,                   \
                                _hash, _rehash, _compare,               \
                                _loadf, _NoKey, _NoVal)                 \
  _Type* new_##_type(void) {                                            \
    _Type* out = (_Type*)new_obj(&_Type##Type);                         \
    init_##_type(out);                                                  \
    return out;                                                         \
  }                                                                     \
                                                                        \
  void init_##_type(_Type* _type) {                                     \
    _type->entries = NULL;                                              \
    _type->count = 0;                                                   \
    _type->max_count = 0;                                               \
  }                                                                     \
                                                                        \
  void free_##_type(_Type* _type) {                                     \
    deallocate(_type->entries, 0, false);                               \
    init_##_type(_type);                                                \
  }                                                                     \
                                                                        \
  static _Type##Entry* find_##_type##_entry(_Type##Entry* entries,      \
                                            size_t max_count,           \
                                            _K key,                     \
                                            hash_t key_hash) {          \
    size_t i, m;                                                        \
    _Type##Entry* entry, * ts = NULL;                                   \
    m = max_count - 1;                                                  \
    i = key_hash & m;                                                   \
                                                                        \
    for (;;) {                                                          \
      entry = &entries[i];                                              \
                                                                        \
      if (entry->key == _NoKey) {                                       \
        return ts ? ts : entry;                                         \
      } else if (entry->val == _NoVal) { /* tombstone */                \
        ts = ts ? : entry;                                              \
        i = (i + 1) & m;                                                \
      } else if (_compare(key, entry->key)) {                           \
        return entry;                                                   \
      } else {                                                          \
        i = (i + 1) & m;                                                \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  void resize_##_type(_Type* _type, size_t new_count) {                 \
    if (new_count == 0)                                                 \
      free_##_type(_type);                                              \
    else {                                                              \
      size_t old_max = _type->max_count;                                \
      size_t new_max = adjust_table_size(_loadf, new_count, old_max);   \
                                                                        \
      if (new_max != old_max) {                                         \
        _Type##Entry* old_entries = _type->entries, * new_entries;      \
                                                                        \
        allocate((void**)&new_entries,                                  \
                 new_max * sizeof(_Type##Entry),                        \
                 false);                                                \
                                                                        \
        for (size_t i=0; i<new_max; i++)                                \
          new_entries[i] = (_Type##Entry) { _NoKey, _NoVal };           \
                                                                        \
        if (old_entries != NULL) {                                      \
          /* rehash */                                                  \
          _type->count = 0;                                             \
                                                                        \
          for (size_t i=0; i<old_max; i++) {                            \
            _Type##Entry* old_entry = &old_entries[i];                  \
                                                                        \
            /* tombstone or unused */                                   \
            if (old_entry->key == _NoKey || old_entry->val == _NoVal)   \
              continue;                                                 \
                                                                        \
            hash_t hash = _rehash(old_entry);                           \
            _K key = old_entry->key;                                    \
            _V val = old_entry->val;                                    \
            _Type##Entry* new_entry = find_##_type##_entry(new_entries, \
                                                           new_max,     \
                                                           key,         \
                                                           hash);       \
            new_entry->key = key;                                       \
            new_entry->val = val;                                       \
            _type->count++;                                             \
          }                                                             \
                                                                        \
          deallocate(old_entries, 0, false);                            \
        }                                                               \
                                                                        \
        _type->entries = new_entries;                                   \
        _type->max_count = new_max;                                     \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  _V _type##_intern(_Type* _type, _K key,                               \
                   _type##_intern_t intern, void* data) {               \
    size_t count = _type->count;                                        \
    resize_##_type(_type, count+1);                                     \
    size_t max_count = _type->max_count;                                \
    _Type##Entry* entries = _type->entries;                             \
    hash_t hash = _hash(key);                                           \
    _Type##Entry* entry = find_##_type##_entry(entries,                 \
                                               max_count,               \
                                               key,                     \
                                               hash);                   \
                                                                        \
    if (entry->val == _NoVal) {                                         \
      bool inc_count = entry->key == _NoKey;                            \
      intern(_type, entry, key, hash, data);                            \
                                                                        \
      if (inc_count)                                                    \
        _type->count++;                                                 \
    }                                                                   \
                                                                        \
    return entry->val;                                                  \
  }                                                                     \
                                                                        \
  rl_status_t _type##_get(_V* result, _Type* _type, _K key) {           \
                                                                        \
    rl_status_t status = NOTFOUND;                                      \
    if (_type->entries != NULL) {                                       \
      hash_t hash = _hash(key);                                         \
      _Type##Entry* entries = _type->entries;                           \
      size_t max_count = _type->max_count;                              \
      _Type##Entry* entry = find_##_type##_entry(entries,               \
                                                 max_count,             \
                                                 key,                   \
                                                 hash);                 \
                                                                        \
      if (entry->val != _NoVal) {                                       \
        status = OKAY;                                                  \
                                                                        \
        if (result)                                                     \
          *result = entry->val;                                         \
      }                                                                 \
    }                                                                   \
                                                                        \
    return status;                                                      \
  }                                                                     \
                                                                        \
  rl_status_t _type##_set(_V* result, _Type* _type, _K key, _V val) {   \
    rl_status_t status = NOTFOUND;                                      \
                                                                        \
    if (_type->entries != NULL) {                                       \
      hash_t hash = _hash(key);                                         \
      _Type##Entry* entries = _type->entries;                           \
      size_t max_count = _type->max_count;                              \
      _Type##Entry* entry = find_##_type##_entry(entries,               \
                                                 max_count,             \
                                                 key,                   \
                                                 hash);                 \
                                                                        \
      if (entry->val != _NoVal) {                                       \
        status = OKAY;                                                  \
                                                                        \
        if (result)                                                     \
          *result = entry->val;                                         \
                                                                        \
        entry->val = val;                                               \
      }                                                                 \
    }                                                                   \
                                                                        \
    return status;                                                      \
    }                                                                   \
                                                                        \
  rl_status_t _type##_put(_V* result, _Type* _type, _K key, _V val) {   \
    rl_status_t status = NOTFOUND;                                      \
                                                                        \
    size_t count = _type->count;                                        \
    resize_##_type(_type, count+1);                                     \
                                                                        \
    if (_type->entries != NULL) {                                       \
      hash_t hash = _hash(key);                                         \
      _Type##Entry* entries = _type->entries;                           \
      size_t max_count = _type->max_count;                              \
      _Type##Entry* entry = find_##_type##_entry(entries,               \
                                                 max_count,             \
                                                 key,                   \
                                                 hash);                 \
                                                                        \
      if (entry->val != _NoVal) {                                       \
        status = OKAY;                                                  \
                                                                        \
        if (result)                                                     \
          *result = entry->val;                                         \
                                                                        \
        entry->val = val;                                               \
      }                                                                 \
    }                                                                   \
                                                                        \
    return status;                                                      \
  }

static inline hash_t hash_strings_key(const char* chars) {
  return hash_chars(chars) & DATA_BITS;
}

static inline hash_t rehash_strings_entry(StringsEntry* entry) {
  return entry->val->h.hash;
}

static inline bool compare_strings_keys(const char* chars_x, const char* chars_y) {
  return streq(chars_x, chars_y);
}

MUTABLE_HASH_TABLE_IMPL(Strings, const char*, String*, strings,
                        hash_strings_key, rehash_strings_entry, compare_strings_keys,
                        DEFAULT_LOAD_FACTOR, NULL, NULL);

static inline hash_t hash_mut_dict_key(Value val) {
  return rl_hash(val);
}

static inline hash_t rehash_mut_dict_entry(MutDictEntry* entry) {
  return rl_hash(entry->key);
}

static inline bool compare_mut_dict_keys(Value x, Value y) {
  return rl_egal(x, y);
}

MUTABLE_HASH_TABLE_IMPL(MutDict, Value, Value, mut_dict,
                        hash_mut_dict_key, rehash_mut_dict_entry, compare_mut_dict_keys,
                        DEFAULT_LOAD_FACTOR, NOTHING, NOTHING);

#undef MUTABLE_HASH_TABLE_IMPL
