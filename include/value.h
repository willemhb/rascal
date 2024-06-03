#ifndef rl_value_h
#define rl_value_h

#include "common.h"
#include "util.h"

/* Definitions & declarations for rascal values. */
// value types
typedef word_t        Value;
typedef double        Number;
typedef bool          Boolean;
typedef char          Glyph;
typedef nullptr_t     Nul;

// common object header types
typedef struct Object Object;

// user metaobject types
typedef struct Type   Type;

// user identifier types
typedef struct Symbol Symbol;

// user IO types
typedef struct Port   Port;

// user compound types
typedef struct Pair   Pair;
typedef struct List   List;
typedef struct String String;
typedef struct Binary Binary;
typedef struct Vector Vector;
typedef struct Map    Map;
typedef struct Record Record;

// user environment types
typedef struct Environ Environ;
typedef struct Module  Module;
typedef struct Binding Binding;

// user function types
typedef struct Closure Closure;
typedef struct Native  Native;
typedef struct Generic Generic;

// user control types
typedef struct Control Control;

// user mutable compound types
typedef struct MutPair MutPair;
typedef struct MutList MutList;
typedef struct MutStr  MutStr;
typedef struct MutBin  MutBin;
typedef struct MutVec  MutVec;
typedef struct MutMap  MutMap;

// internal array types
typedef struct Objects Objects;

// internal table types
typedef struct StringCache StringCache;
typedef struct ModuleCache ModuleCache;
typedef struct MethodCache MethodCache;
typedef struct ReadTable   ReadTable;
typedef struct NameSpace   NameSpace;

// internal environment types
typedef struct UpValue UpValue;

// internal node types
typedef struct VecNode VecNode;
typedef struct VecLeaf VecLeaf;
typedef struct MapNode MapNode;
typedef struct MapLeaf MapLeaf;

// internal function types
typedef struct MethodTableRoot MethodTableRoot;
typedef struct MethodTableNode MethodTableNode;
typedef struct MethodTableLeaf MethodTableLeaf;

// internal function pointer types
typedef rl_status_t (*rl_trace_fn_t)(Object* obj);
typedef rl_status_t (*rl_destruct_fn_t)(Object* obj);
typedef rl_status_t (*rl_reader_fn_t)(Port* stream, Value* buffer);
typedef hash_t (*rl_hash_fn_t)(Value x);
typedef bool (*rl_egal_fn_t)(Value x, Value y);
typedef int (*rl_order_fn_t)(Value x, Value y);
typedef rl_status_t (*rl_native_fn_t)(size_t argc, Value* args, Value* buffer);

// enum types
typedef enum Kind {
  BOTTOM_TYPE, // Nothing - no values
  DATA_TYPE,   // common data type - designates a set of Rascal values
  TOP_TYPE     // Any - union of all types
} Kind;

typedef enum Scope {
  LOCAL,   // value is on stack
  UPVALUE, // value is indirected through an upvalue
  MODULE,  // value is stored at module level
} Scope;

// common object header types
#define HEADER                                  \
  Object* next;                                  \
  Type*   type;                                  \
  Map*    meta;                                  \
  word_t  hash  : 48;                            \
  word_t  trace :  1;                            \
  word_t  free  :  1;                            \
  word_t  sweep :  1;                            \
  word_t  gray  :  1;                            \
  word_t  black :  1

struct Object {
  HEADER;

  // bit fields
  word_t flags : 11;

  // data fields
  byte_t data[];
};

// user metaobject types
struct Type {
  HEADER;

  // bit fields
  word_t kind    : 3;
  word_t builtin : 1;

  // unique identifier (basis of hash value)
  word_t idno;

  // layout information
  Value      value_type; // tag for values of this type
  size_t     value_size;
  size_t     object_size;
  NameSpace* slots;

  // constructor
  Object* ctor;

  // lifetime methods
  rl_trace_fn_t    trace_fn;
  rl_destruct_fn_t destruct_fn;

  // comparison methods
  rl_hash_fn_t  hash_fn;
  rl_egal_fn_t  egal_fn;
  rl_order_fn_t order_fn;
};

// user identifier types
struct Symbol {
  HEADER;

  // bit fields
  word_t literal : 1;

  // identifier info
  String* nmspc;
  String* name;
  word_t  idno;      // non-zero for gensyms
};

// user IO types
struct Port {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t input    : 1;
  word_t output   : 1;

  // file pointer
  FILE* ios;
};

// user compound types
struct Pair {
  HEADER;

  Value car;
  Value cdr;
};

struct List {
  HEADER;

  Value  head;
  size_t count;
  List*  tail;
};

struct String {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;

  // data fields
  char* chars;
  size_t count;
};

struct Binary {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  union {
    void*     data;
    uint8_t*  i8;
    uint16_t* i16;
  };

  size_t count;
};

struct Vector {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t packed    : 1;

  // data fields
  VecNode* root;
  size_t   count;
  Value    tail[];
};

struct Map {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t fastcmp   : 1;

  // data fields
  MapNode* root;
  size_t   count;
};

struct Record {
  HEADER;

  // data fields
  Map* slots;
};

// user environment types
struct Environ {
  HEADER;

  // bit fields
  word_t scope    : 2;
  word_t bound    : 1;
  word_t captured : 1;

  // data fields
  Environ*   parent;
  NameSpace* locals;
  NameSpace* nonlocals;

  union {
    Objects* upvals; // bound function environments
    MutVec*  values; // bound module environments
  };
};

struct Module {
  HEADER;

  // bit fields
  word_t compiled : 1;
  word_t executed : 1;

  // data fields
  // initial layout identical to closure, allowing module to be safely cast to closure
  Binary*  code;
  Vector*  vals;
  Environ* envt;

  // data fieldsread from module spec
  Symbol* name;    // (module <name> ...)
  List*   imports; // (import <imports*>)
  List*   exports; // (export <exports*>)
  List*   body;    // (begin <body*>)

  // result of module body execution
  Value result;
};

struct Binding {
  HEADER;

  // bit fields
  word_t scope       : 2;
  word_t exported    : 1;
  word_t final       : 1;
  word_t inited      : 1;
  word_t multimethod : 1; // rebinding behaves like multi-method
  word_t macro       : 1; // macro name

  // data fields
  Binding* captures;   // the binding captured by this binding (if any)
  Environ* environ;    // the environment in which the binding was *originally* created
  Symbol*  name;       // name under which this binding was created in *original* environment
  size_t   offset;     // location (may be on stack, in upvalues, or directly in environment)
  Type*    constraint; // type constraint for this binding
  Value    initval;    // default initval (only used for object scope)
};

// user function types
struct Closure {
  HEADER;

  // data fields
  Binary*  code;
  Vector*  vals;
  Environ* envt;
};

struct Native {
  HEADER;

  rl_native_fn_t C_function;
};

struct Generic {
  HEADER;

  // data fields
  MethodCache*     cache;
  MethodTableRoot* methods;
};

// user control types
typedef struct IFrame IFrame;

struct Control {
  HEADER;

  // data fields
  IFrame* frames;   // preserved stack frames
  size_t  fcount;   // frame count
  MutVec* sbuffer;  // buffer for stack values
};

// user mutable compound types
struct MutPair {
  HEADER;

  Value car;
  Value cdr;
};

struct MutList {
  HEADER;

  // data fields
  Value    head;
  MutList* tail; // no count is stored, since it's impossible to ensure its accuracy with a mutable tail
};

struct MutStr {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;
  word_t algo     : 1;

  // data fields
  char* characters;
  size_t count, max_count;
};

struct MutBin {
  HEADER;

  // bit fields
  word_t eltype : 4;
  word_t algo   : 1;

  // data fields
  union {
    void*     data;
    uint8_t*  i8;
    uint16_t* i16;
  };

  size_t count, max_count;
};

struct MutVec {
  HEADER;

  // bit fields
  word_t algo : 1;

  // data fields
  Value* data;
  size_t count, max_count;
};

typedef struct {
  Value key;
  Value val;
} MapEntry;

struct MutMap {
  HEADER;

  // bit fields
  word_t loadfactor : 5;
  word_t fastcmp    : 1;

  // data fields
  MapEntry* entries;
  size_t count, max_count;
};

// internal array types
struct Objects {
  HEADER;

  // bit fields
  word_t algo : 1;

  // data fields
  Object** objs;
  size_t count, max_count;
};

// internal table types
typedef struct {
  char*   key;
  String* val;
} SCEntry;

struct StringCache {
  HEADER;

  // data fields
  SCEntry* entries;
  size_t count, max_count;
};

typedef struct {
  String* key;
  Module* val;
} MCEntry;

struct ModuleCache {
  HEADER;

  // data fields
  SCEntry* entries;
  size_t count, max_count;
};

// internal object node types
struct VecNode {
  HEADER;

  // bit fields
  word_t offset    : 5;
  word_t transient : 1;

  Object** children;
  uint32_t count, max_count;
};

struct VecLeaf {
  HEADER;

  Value slots[64];
};

struct MapNode {
  HEADER;

  Object** children;
  size_t   bitmap;
};

struct MapLeaf {
  HEADER;

  MapLeaf* next;
  Value    key;
  Value    val;
};

// mutable object types

// internal object types

// function types

struct NameSpace {
  HEADER;

  NameSpace*   parent; // parent namespace
  Scope*       locals; // local names
  Scope*       upvals; // locally referenced upvalues (unused at module level)
  Environment* module; // shortcut to module environment
};

struct UpValue {
  HEADER;

  UpValue* next;

  union {
    Value* location;
    Value  value;
  };
};

/* tags and masks */
#define QNAN      0x7ff8000000000000UL
#define SIGN      0x8000000000000000UL

#define TAG_BITS  0xffff000000000000UL
#define DATA_BITS 0x0000ffffffffffffUL

#define NUL       0x7ffc000000000000UL
#define BOOL      0x7ffd000000000000UL
#define GLYPH     0x7ffe000000000000UL
#define SMALL     0x7fff000000000000UL
#define CPTR      0xfffc000000000000UL
#define FPTR      0xfffd000000000000UL
#define OBJECT    0xfffe000000000000UL
#define SENTINEL  0xffff000000000000UL // not a proper value, used as an internal marker

#define TRUE      0x7ffd000000000001UL // BOOL | 1
#define FALSE     0x7ffd000000000000UL // BOOL | 0

// sentinel values, shouldn't be visible in user code
#define NOTHING   0xffff000000000000UL // SENTINEL | 0

/* Globals */
// type objects
extern Type NumberType, BooleanType, GlyphType, NulType,

  TypeType, StringType, SymbolType, ListType, PortType,

  ChunkType, ClosureType, NativeType, UpValueType, BindingType,

  ObjectsType, MutVecType, MutStrType, MutBinType, MutDictType,

  ReadTableType, ScopeType, StringsType;

// empty singeltons
extern String EmptyString;

// standard ports
extern Port StdIn, StdOut, StdErr;

/* APIs */
// tagging/untagging macros & functions
#define tag_of(x) ((x) & TAG_BITS)

#define tag(x)                                  \
  generic((x),                                  \
          Number:tag_num,                       \
          Boolean:tag_bool,                     \
          Glyph:tag_glyph,                      \
          Nul:tag_nul,                          \
          Object*:tag_obj,                      \
          Type*:tag_obj,                        \
          String*:tag_obj,                      \
          Symbol*:tag_obj,                      \
          List*:tag_obj,                        \
          Port*:tag_obj,                        \
          Chunk*:tag_obj,                       \
          MutVec*:tag_obj,                      \
          MutBin*:tag_obj,                      \
          Strings*:tag_obj                      \
          )(x)

#define as_obj(x)                               \
  generic((x),                                  \
          Value:val_as_obj,                     \
          default:ptr_as_obj)(x)

#define type_of(x)                              \
  generic((x),                                  \
          Value:type_of_val,                    \
          Object*:type_of_obj,                  \
          Type*:type_of_obj,                    \
          String*:type_of_obj,                  \
          Symbol*:type_of_obj,                  \
          List*:type_of_obj,                    \
          Chunk*:type_of_obj,                   \
          Closure*:type_of_obj                  \
          )(x)

Value   tag_num(Number n);
Value   tag_bool(Boolean b);
Value   tag_glyph(Glyph g);
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
Type*   type_of_obj(void* p);

// type implementations
// object type (takes care of common initialization/allocation tasks)
Object* new_obj(Type* type);
void    init_obj(Type* type, Object* obj, bool in_heap);
void    mark_obj(void* p);
void    free_obj(void* p);
void    sweep_obj(void* p);

// string type
String* get_str(const char* chars);
String* new_str(const char* chars, size_t count, hash_t h);
rl_status_t str_ref( Glyph* result, String* str, size_t n);

// symbol type
Symbol* get_sym(const char* name, bool gensym);
Symbol* new_sym(String* name, bool gensym);

// chunk type
Chunk* new_chunk(MutVec* vals, MutBin* instr);

// binding type
Binding* new_bind(Symbol* name, ScopeKind scope_kind, int offset);

#endif
