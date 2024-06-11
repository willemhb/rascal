#ifndef rl_value_h
#define rl_value_h

#include "status.h"
#include "util.h"

/* Definitions & declarations for rascal values. */
// value types
typedef word_t    Value;
typedef nullptr_t Nul;
typedef bool      Boolean;
typedef char      Glyph;
typedef uint16_t  Primitive; // A primitive function builtin into the bytecode interpreter
typedef word_t    Arity;     // 48-bit unsigned integer (can hold any valid hash or size)
typedef int       Small;
typedef double    Real;
typedef void*     Pointer;
typedef funcptr_t FuncPtr;

// common object header type
typedef struct Object Object;

// user metaobject types
typedef struct Type Type;

// user identifier types
typedef struct Symbol Symbol;

// user IO types
typedef struct Port Port;

// user big number types
typedef struct Big    Big;
typedef struct Ratio  Ratio;

// user compound types
typedef struct Pair   Pair;
typedef struct List   List;
typedef struct String String;
typedef struct Binary Binary;
typedef struct Vector Vector;
typedef struct Map    Map;

// user compound type aliases
typedef struct Map    Record;
typedef struct Map    Set;

// user environment types
typedef struct Environ Environ;
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

// user mutable compound type aliases
typedef struct MutMap  MutSet;

// internal array types
typedef struct Alist Alist;

// internal table types
typedef struct StringCache StringCache;
typedef struct EnvMap      EnvMap;
typedef struct ReadTable   ReadTable;

// internal environment types
typedef struct UpValue UpValue;

// internal node types
typedef struct VecNode VecNode;
typedef struct MapNode MapNode;

// internal function types
typedef struct MTRoot MTRoot;
typedef struct MTNode MTNode;
typedef struct MTLeaf MTLeaf;

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
  // kinds ordered by heuristic specificity
  BOTTOM_TYPE,         // nothing - no values
  DATA_TYPE,           // a set of instantiable Rascal values
  UNION_TYPE,          // a union type with at least one abstract member
  TOP_TYPE             // union of all types
} Kind;

typedef enum Scope {
  LOCAL_SCOPE,     // value is on stack
  UPVALUE_SCOPE,   // value is indirected through an upvalue
  NAMESPACE_SCOPE, // value is stored in the current namespace environ
  STRUCT_SCOPE,    // value is stored in struct object
  RECORD_SCOPE,    // value is stored in record object
} Scope;

// common object header types
#define HEADER                                   \
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
  Value      value_type;   // tag for values of this type
  size_t     value_size;
  size_t     object_size;

  // type spec fields
  Type*      parent;       // parent type (an abstract type, trait, or Any)
  Environ*   slots;        // 
  Set*       members;      // union or trait members

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

static inline bool is_gs(Symbol* s) {
  return s->idno > 0;
}

// user IO types
struct Port {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t input    : 1;
  word_t output   : 1;
  word_t lispfile : 1;

  // data fields
  FILE* ios;
};

// user numeric types
struct Big {
  HEADER;

  int      sign;
  uint32_t size;
  byte_t  *digits;
};

struct Ratio {
  HEADER;

  Big* numer;
  Big* denom;
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
  List*  tail;
  size_t count;
};

struct String {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;

  // data fields
  char*  chars;
  size_t count;
};

struct Binary {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  byte_t* data;
  size_t  count;
};

struct Vector {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t packed    : 1;

  // data fields
  size_t count;

  union {
    // common Vector
    struct {
      VecNode* root;
      Value    tail[0];
    };

    // packed Vector
    struct {
      Value    slots[0];
    };
  };
};

struct Map {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t fastcmp   : 1;

  // data fields  
  size_t   count;

  union {
    List*    entries; // for small maps (< 16 keys), stored as a sorted map
    MapNode* root;    // larger maps are stored in a HAMT-based structure
  };
};

// user environment types
struct Environ {
  HEADER;

  // bit fields
  word_t scope    : 3;
  word_t bound    : 1;
  word_t captured : 1;

  // data fields
  Symbol*    name;      // Name for this Environ object (may be a namespace, function, or type)
  Environ*   parent;    // the environment within which this environment was defined
  Environ*   template;  // the unbound environment a bound environment was cloned from
  EnvMap*    locals;
  EnvMap*    nonlocals;

  union {
    Alist* upvals;
    MutVec*  values;
  };
};

struct Binding {
  HEADER;

  // bit fields
  word_t scope         : 3;
  word_t final         : 1;
  word_t inited        : 1;
  word_t specializable : 1; // can have methods added
  word_t macro         : 1; // macro name

  // data fields
  Binding* captures;   // the binding captured by this binding (if any)
  Environ* environ;    // the environment in which the binding was *originally* created
  Symbol*  name;       // name under which this binding was created in *original* environment
  size_t   offset;     // location (may be on stack, in upvalues, or directly in environment)
  Type*    constraint; // type constraint for this binding
  Value    initval;    // default initval (only used for object scopes)
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
  MutMap* cache;
  MTRoot* root;
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

// mutable array types
#define DYNAMIC_ARRAY(X)                        \
  word_t algo : 1;                              \
  X* data;                                      \
  X* _static;                                   \
  size_t count, max_count, max_static

struct MutStr {
  HEADER;

  // bit fields
  word_t encoding : 4;
  word_t hasmb    : 1;

  // data fields
  DYNAMIC_ARRAY(char);
};

struct MutBin {
  HEADER;

  // bit fields
  word_t eltype : 4;

  // data fields
  DYNAMIC_ARRAY(uint8_t);
};

struct MutVec {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(Value);
};

// internal array types
struct Alist {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(Object*);
};

#undef DYNAMIC_ARRAY

// mutable table types
#define MUTABLE_ENTRY(K, V)                     \
  K key;                                        \
  V val

#define MUTABLE_TABLE(E)                        \
  word_t loadfactor : 5;                        \
  word_t fastcmp    : 1;                        \
                                                \
  E* entries;                                   \
  size_t count, max_count

typedef struct {
  MUTABLE_ENTRY(Value, Value);
} MMEntry;

struct MutMap {
  HEADER;

  MUTABLE_TABLE(MMEntry);
};

// internal table types
typedef struct {
  char*   key;
  String* val;
} SCEntry;

struct StringCache {
  HEADER;

  MUTABLE_TABLE(SCEntry);
};

typedef struct {
  MUTABLE_ENTRY(Symbol*, Binding*);
} EMEntry;

struct EnvMap {
  HEADER;

  MUTABLE_TABLE(EMEntry);
};

#undef MUTABLE_TABLE
#undef MUTABLE_ENTRY

struct ReadTable {
  HEADER;

  ReadTable* parent;
  funcptr_t  dispatch[256]; // common readers
  funcptr_t  intrasym[256]; // intra-symbol readers
};

// internal environment types
struct UpValue {
  HEADER;

  // bit fields
  word_t closed : 1;

  // data fields
  UpValue* next_upv;

  union {
    Value* location;
    Value  value;
  };
};

static inline Value* deref_upval(UpValue* upv) {
  if ( upv->closed )
    return &upv->value;

  return upv->location;
}

// internal object node types
struct VecNode {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t depth     : 4;
  word_t transient : 1;

  uint32_t count, max_count;

  // data fields
  union {
    Object** children;
    Value    slots[0];
  };
};

struct MapNode {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t depth     : 4;
  word_t transient : 1;

  // data fields
  size_t   bitmap;

  Object** children;
};

/* tags and masks */
#define QNAN       0x7ff8000000000000UL
#define SIGN       0x8000000000000000UL

#define TAG_BITS   0xffff000000000000UL
#define WTAG_BITS  0xffffffff00000000UL
#define WDATA_BITS 0x00000000ffffffffUL
#define DATA_BITS  0x0000ffffffffffffUL

// wide tags
#define REAL      0x0000000000000000UL // dummy tag
#define ARITY     0x7ffc000000000000UL // arity value
#define CPTR      0x7ffd000000000000UL
#define FPTR      0x7ffe000000000000UL
#define OBJECT    0x7fff000000000000UL
#define LITTLE    0xffff000000000000UL // 32-bit value with wide tag

#define NUL       0xffff000000000000UL
#define BOOL      0xffff000100000000UL
#define GLYPH     0xffff000200000000UL
#define PRIMITIVE 0xffff000300000000UL
#define SMALL     0xffff000400000000UL
#define SENTINEL  0xffffffff00000000UL

#define TRUE      0x7ffd000000000001UL // BOOL | 1
#define FALSE     0x7ffd000000000000UL // BOOL | 0

// sentinel values, shouldn't be visible in user code
#define NOTHING   0xffffffff00000000UL // SENTINEL | 0

/* Globals */
// type objects
extern Type NulType, BooleanType, GlyphType,

  ArityType, SmallType, RealType,

  PointerType, FuncPtrType,

  TypeType,

  SymbolType,

  PortType,

  PairType, ListType,

  StringType, BinaryType,

  VectorType, MapType,

  EnvironType, BindingType,

  ClosureType, NativeType, GenericType,

  ControlType,

  MutPairType, MutListType,

  MutStrType, MutBinType,

  MutVecType, MutMapType,

  AlistType,

  StringCacheType, ModuleCacheType, MethodCacheType, ReadTableType, EnvMapType,

  UpValueType,

  VecNodeType, VecLeafType, MapNodeType, MapLeafType,

  MTRootType, MTNodeType, MTLeafType,

  BottomType, TopType;

// empty singeltons
extern String EmptyString;
extern List   EmptyList;
extern Vector EmptyVector;
extern Map    EmptyMap;

// standard ports
extern Port StdIn, StdOut, StdErr;

// parsed command line arguments
extern List* ClArgs;
extern Set*  ClFlags;
extern Map*  ClOpts;

/* APIs */
// tagging/untagging macros & functions
static inline Value tag_of(Value x) {
  return (x & LITTLE) == LITTLE ? x & WTAG_BITS : x & TAG_BITS;
}

static inline Value untag(Value x) {
  return (x & LITTLE) == LITTLE ? x & WDATA_BITS : x & DATA_BITS;
}

#define tag(x)                                  \
  generic((x),                                  \
          Nul:tag_nul,                          \
          Boolean:tag_bool,                     \
          Glyph:tag_glyph,                      \
          Small:tag_small,                      \
          Real:tag_real,                        \
          Pointer:tag_ptr,                      \
          FuncPtr:tag_fptr,                     \
          Object*:tag_obj,                      \
          Type*:tag_obj,                        \
          Symbol*:tag_obj,                      \
          Port*:tag_obj,                        \
          Pair*:tag_obj,                        \
          List*:tag_obj,                        \
          String*:tag_obj,                      \
          Binary*:tag_obj,                      \
          Vector*:tag_obj,                      \
          Map*:tag_obj,                         \
          Record*:tag_obj                       \
          )(x)

#define as_obj(x)                               \
  generic((x),                                  \
          Value:val_as_obj,                     \
          default:ptr_as_obj)(x)

#define type_of(x)                              \
  generic((x),                                  \
          Value:type_of_val,                    \
          default:type_of_obj)(x)

#define has_type(x, t)                          \
  generic((x),                                  \
          Value:val_has_type,                   \
          default:obj_has_type)(x, t)

#define mark(x)                                 \
  generic((x),                                  \
          Value:mark_val,                       \
          default:mark_obj)(x)

// tagging methods
Value tag_nul(Nul n);
Value tag_bool(Boolean b);
Value tag_glyph(Glyph g);
Value tag_small(Small s);
Value tag_real(Real n);
Value tag_ptr(Pointer p);
Value tag_fptr(FuncPtr f);
Value tag_obj(void* p);

// casting methods
Nul     as_nul(Value x);
Boolean as_bool(Value x);
Glyph   as_glyph(Value x);
Small   as_small(Value x);
Real    as_real(Value x);
Pointer as_ptr(Value x);
FuncPtr as_fptr(Value x);

// object cast methods
Object* val_as_obj(Value v);
Object* ptr_as_obj(void* p);

// type_of methods
Type* type_of_val(Value v);
Type* type_of_obj(void* p);

// has_type methods
bool  val_has_type(Value v, Type* t);
bool  obj_has_type(void* p, Type* t);

// mark methods
rl_status_t mark_val(Value v);
rl_status_t mark_obj(void* o);

// trace methods
rl_status_t trace(void* o);

/* Value APIs */
// symbols
#define qualify(s, ns)                          \
  generic((ns),                                 \
          char*:cstr_qualify,                   \
          String*:str_qualify,                  \
          Symbol*:sym_qualify)(s, ns)

Symbol* mk_sym(String* ns, String* n, bool gs);

// qualify methods
Symbol* cstr_qualify(Symbol* s, char* cstr);
Symbol* str_qualify(Symbol* s, String* str);
Symbol* sym_qualify(Symbol* s, Symbol* ns);

// strings
String* get_str(char* chars);

// binaries

// environ
#define define(n, v, e)                         \
  generic((n),                                  \
          char*:cstr_define,                    \
          Symbol*:sym_define)(n, v, e)

// define methods
size_t cstr_define(char* n, Value i, Environ* e);
size_t sym_define(Symbol* n, Value i, Environ* e);

// mutable arrays
#define MUTABLE_ARRAY(T, t, X, ...)                                     \
  rl_status_t init_##t(T* a, X* s, size_t ss __VA_OPT__(,) __VA_ARGS__); \
  rl_status_t free_##t(T* a);                                           \
  rl_status_t grow_##t(T* a, size_t n);                                 \
  rl_status_t shrink_##t(T* a, size_t n);                               \
  rl_status_t write_##t(T* a, X* s, size_t n);                          \
  rl_status_t t##_push(T* a, X x);                                      \
  rl_status_t t##_pushn(T* a, size_t n, ...);                           \
  rl_status_t t##_pushv(T* a, size_t n, va_list va);                    \
  rl_status_t t##_pop(T* a, X* r);                                      \
  rl_status_t t##_popn(T* a, X* r, bool e, size_t n)

MUTABLE_ARRAY(MutVec, mvec, Value);
MUTABLE_ARRAY(Alist, alist, void*);
MUTABLE_ARRAY(MutBin, mbin, uint8_t, CType eltype);
MUTABLE_ARRAY(MutStr, mstr, char, CType encoding);

#undef MUTABLE_ARRAY

// mutable tables

#endif
