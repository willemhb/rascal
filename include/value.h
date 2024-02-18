#ifndef rl_value_h
#define rl_value_h

#include "common.h"

/* Definitions & declarations for rascal values. */
// immediate values
typedef word_t Value;
typedef double Number;
typedef bool Boolean;
typedef char Glyph;
typedef int Small;
typedef nullptr_t Nul;

// user object types
typedef struct Object Object;
typedef struct Type Type;
typedef struct String String;
typedef struct Binary Binary;
typedef struct Symbol Symbol;
typedef struct List List;
typedef struct Tuple Tuple;

// internal object types
typedef struct Chunk Chunk;
typedef struct Closure Closure;
typedef struct Native Native;
typedef struct UpValue UpValue;
typedef struct Binding Binding;
typedef struct Objects Objects;
typedef struct Values Values;
typedef struct Objects Objects;
typedef struct TextBuffer TextBuffer;
typedef struct BinaryBuffer BinaryBuffer;
typedef struct Strings Strings;
typedef struct Scope Scope;

// internal function pointer types
typedef rl_status_t (*rl_trace_fn_t)(Object* obj);
typedef rl_status_t (*rl_destruct_fn_t)(Object* obj);
typedef hash_t (*rl_hash_fn_t)(Value x);
typedef bool (*rl_egal_fn_t)(Value x, Value y);
typedef int (*rl_order_fn_t)(Value x, Value y);
typedef rl_status_t (*rl_native_fn_t)(size_t argc, Value* args, Value* buffer);

struct Object {
  Object* next;
  Type* type;
  word_t hash  : 48;
  word_t flags : 11;
  word_t trace :  1; // If true, this object's owned pointers need to be traced (prevents objects like Heap.gray_objects from accidentally being traced)
  word_t free  :  1; // If true, free this object's owned pointers
  word_t sweep :  1; // If true, free this object
  word_t gray  :  1; // gc traced flag
  word_t black :  1; // gc marked flag
};

#define HEADER                                  \
  Object h

struct Type {
  HEADER;

  word_t idno;
  Value value_type; // tag for values of this type
  size_t value_size;
  size_t object_size;

  // lifetime methods
  rl_trace_fn_t trace;
  rl_destruct_fn_t destruct;

  // comparison methods
  rl_hash_fn_t hash;
  rl_egal_fn_t egal;
  rl_order_fn_t order;
};

struct String {
  HEADER;

  char* data;
  size_t count;
};

struct Binary {
  HEADER;

  byte_t* data;
  size_t count;
};

struct Symbol {
  HEADER;

  String* namespace;
  String* name;
  word_t idno;      // non-zero for gensyms, 0 otherwise
};

struct List {
  HEADER;

  Value  head;
  size_t count;
  List*  tail;
};

struct Tuple {
  HEADER;

  Value* data;
  size_t count;
};

struct Chunk {
  HEADER;

  Tuple* vals;
  Binary* instr;
};

struct Closure {
  HEADER;

  Chunk* code;
  Objects* upvals;
};

struct Native {
  HEADER;

  rl_native_fn_t C_function;
};

struct UpValue {
  HEADER;

  UpValue* next;
  Value* location;
  Value  value;
};

struct Binding {
  HEADER;

  Symbol* name;
  Value   value;
  int     scope_type;
  int     binding_type;
  int     offset;
  int     parent_offset;
};

#define DYNAMIC_ARRAY_TYPE(_Type, _X)           \
  struct _Type {                                \
    HEADER;                                     \
    _X* data;                                   \
    size_t count, max_count;                    \
  }

DYNAMIC_ARRAY_TYPE(Objects, Object*);
DYNAMIC_ARRAY_TYPE(Values, Value);
DYNAMIC_ARRAY_TYPE(TextBuffer, char);
DYNAMIC_ARRAY_TYPE(BinaryBuffer, byte_t);

#undef DYNAMIC_ARRAY_TYPE

#define MUTABLE_HASH_TABLE_TYPE(_Type, _K, _V)  \
  typedef struct {                              \
    _K key;                                     \
    _V val;                                     \
  } _Type##Entry;                               \
                                                \
  struct _Type {                                \
    HEADER;                                     \
                                                \
    _Type##Entry* entries;                      \
    size_t count;                               \
    size_t max_count;                           \
  }

MUTABLE_HASH_TABLE_TYPE(Strings, const char*, String*);
MUTABLE_HASH_TABLE_TYPE(Scope, Symbol*, Binding*);

#undef MUTABLE_HASH_TABLE_TYPE

/* tags and masks */
#define QNAN      0x7ff8000000000000UL
#define SIGN      0x8000000000000000UL

#define TAG_BITS  0xffff000000000000UL
#define DATA_BITS 0x0000ffffffffffffUL

#define NUL       0x7ffc000000000000UL
#define BOOL      0x7ffd000000000000UL
#define GLYPH     0x7ffe000000000000UL
#define SMALL     0x7fff000000000000UL
#define OBJECT    0xffff000000000000UL

#define TRUE      0x7ffd000000000001UL // BOOL | 1
#define FALSE     0x7ffd000000000000UL // BOOL | 0

/* Globals */
// type objects
extern Type NumberType, BooleanType, GlyphType, SmallType, NulType,
  TypeType, StringType, BinaryType, SymbolType, ListType, TupleType,
  ChunkType, ClosureType, NativeType, UpValueType,

  ObjectsType, ValuesType, TextBufferType, BinaryBufferType,

  StringsType, ScopeType;

// empty singeltons
extern String EmptyString;

/* APIs */
// tagging/untagging macros & functions
#define tag_of(x) ((x) & TAG_BITS)

#define tag(x)                                  \
  generic((x),                                  \
          Number:tag_num,                       \
          Boolean:tag_bool,                     \
          Glyph:tag_glyph,                      \
          Small:tag_small,                      \
          Nul:tag_nul,                          \
          Object*:tag_obj,                      \
          Type*:tag_obj,                        \
          String*:tag_obj,                      \
          Binary*:tag_obj,                      \
          Symbol*:tag_obj,                      \
          List*:tag_obj,                        \
          Tuple*:tag_obj,                       \
          Chunk*:tag_obj,                       \
          Values*:tag_obj,                      \
          BinaryBuffer*:tag_obj,                \
          Strings*:tag_obj,                     \
          Scope*:tag_obj                        \
          )(x)

#define as_obj(x)                               \
  generic((x),                                  \
          Value:val_as_obj,                     \
          default:ptr_as_obj)(x)

#define type_of(x)                              \
  generic((x),                                  \
          Value:type_of_val,                    \
          Object*:type_of_obj,                  \
          default:type_of_ptr)(x)

Value   tag_num(Number n);
Value   tag_bool(Boolean b);
Value   tag_glyph(Glyph g);
Value   tag_small(Small s);
Value   tag_nul(Nul n);
Value   tag_obj(void* p);

bool    is_num(Value x);
bool    is_bool(Value x);
bool    is_nul(Value x);
bool    is_obj(Value x);

Object* val_as_obj(Value v);
Object* ptr_as_obj(void* p);

Number  as_num(Value v);
Boolean as_bool(Value v);
Nul     as_nul(Value v);

Type*   type_of_val(Value v);
Type*   type_of_obj(Object* o);
Type*   type_of_ptr(void* p);

// type implementations
// object type (takes care of common initialization/allocation tasks)
Object* new_obj(Type* type);
void    init_obj(Type* type, Object* obj, bool in_heap);

// string type
String* get_str(const char* chars);
String* new_str(const char* chars, size_t count, hash_t h);

rl_status_t str_ref( Glyph* result, String* str, size_t n);

// binary type
Binary* new_bin(size_t count, byte_t* data);

// tuple type
Tuple* new_tuple(size_t count, Value* data);

// chunk type
Chunk* new_chunk(Values* vals, BinaryBuffer* instr);

// binding type
Binding* new_bind(Symbol* name, int scope_type, int binding_type, int offset, int parent_offset);

// describe macros
#define DYNAMIC_ARRAY_IMPL(_Type, _X, _type)                      \
  _Type* new_##_type(void);                                       \
  void init_##_type(_Type* _type);                                \
  void free_##_type(_Type* _type);                                \
  void resize_##_type(_Type* _type, size_t new_count);            \
  void _type##_push(_Type* _type, _X x);                          \
  void _type##_pushn(_Type* _type, size_t n, ...);                \
  void _type##_write(_Type* _type, size_t n, _X* source);         \
  rl_status_t _type##_pop(_X* result, _Type* _type);              \
  rl_status_t _type##_popn(_X* result, _Type* _type, size_t n)

DYNAMIC_ARRAY_IMPL(Objects, Object*, objects);
DYNAMIC_ARRAY_IMPL(Values, Value, values);
DYNAMIC_ARRAY_IMPL(TextBuffer, char, text_buf);
DYNAMIC_ARRAY_IMPL(BinaryBuffer, byte_t, bin_buf);

#undef DYNAMIC_ARRAY_IMPL

#define MUTABLE_HASH_TABLE_IMPL(_Type, _K, _V, _type)                   \
  _Type* new_##_type(void);                                             \
  void init_##_type(_Type* _type);                                      \
  void free_##_type(_Type* _type);                                      \
  void resize_##_type(_Type* _type, size_t new_count);                  \
  _V _type##_intern(_Type* _type, _K key, void* data);                  \
  rl_status_t _type##_get(_V* result, _Type* _type, _K key);            \
  rl_status_t _type##_set(_V* result, _Type* _type, _K key, _V val);    \
  rl_status_t _type##_put(_V* result, _Type* _type, _K key, _V val);    \
  rl_status_t _type##_add(_V* result, _Type* _type, _K key, _V val);    \
  rl_status_t _type##_has(_Type* _type, _K key);                        \
  rl_status_t _type##_pop(_V* result, _Type* _type, _K key);            \
  _Type* join_##_type##s(_Type* _type##_x, _Type* _type##_y)

MUTABLE_HASH_TABLE_IMPL(Strings, const char*, String*, strings);
MUTABLE_HASH_TABLE_IMPL(Scope, Symbol*, Binding*, scope);

#undef MUTABLE_HASH_TABLE_IMPL

#endif
