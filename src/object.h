#ifndef object_h
#define object_h

#include <stdio.h>

#include "value.h"
#include "type.h"

// C types --------------------------------------------------------------------
#define HEADER                    \
  Obj* next_live;                 \
  uint64 hash   : 48;             \
  uint64 flags  : 11;             \
  uint64 type   :  5

typedef uint64 uhash;
typedef struct Objs Objs;

typedef enum {
  // common flags -------------------------------------------------------------
  BLACK      =0x400,
  GRAY       =0x200,
  HASHED     =0x100,
  FROZEN     =0x080,

  // symbol flags -------------------------------------------------------------
  GENERATED  =0x001,
  LITERAL    =0x002,
  CONSTANT   =0x004,

  // stream flags -------------------------------------------------------------
  TEXT       =0x001,
  BINARY     =0x002,
  INPUT      =0x004,
  OUTPUT     =0x008,
  CREATE     =0x010,
  UPDATE     =0x020,

  // method flags -------------------------------------------------------------
  VARGS      =0x001,

  // bin flags ----------------------------------------------------------------
  ENCODED    =0x001,

  // table flags --------------------------------------------------------------
  EQUAL      =0x001,

  // upval flags --------------------------------------------------------------
  OPEN       =0x001
} ObjFl;

struct Obj {
  HEADER;
  ubyte space[0];
};

struct Sym {
  HEADER;
  char *name;
  uidno idno;
  Val   bind;
  Sym*  left, *right;
};

struct Stream {
  HEADER;
  FILE* ios;
};

struct Func {
  HEADER;
  Sym*    name;
  Mtable* mtable;
  Method* methods;
};

struct Bin {
  HEADER;
  ubyte *array;
  usize len, cap;
};

struct Str {
  HEADER;
  usize len;
  char  chars[];
};

struct List {
  HEADER;
  usize len;
  Val   head;
  List *tail;
};

struct Vec {
  HEADER;
  Val *array;
  usize len, cap;
};

struct Tuple {
  HEADER;
  usize len;
  Val slots[];
};

struct Dict {
  HEADER;
  Tuple** entries;
  usize len, cap;

  union {
    sint8*  o8;
    sint16* o16;
    sint32* o32;
    sint64* o64;
  };

  usize ocap;
};

struct Set {
  HEADER;
  Val *members;
  usize len, cap;

  union {
    sint8*  o8;
    sint16* o16;
    sint32* o32;
    sint64* o64;
  };

  usize ocap;
};

struct Record {
  HEADER;
  Dict    fields;
  Record* rtype;
};

// internal types -------------------------------------------------------------
struct Chunk {
  HEADER;
  Vec*   vals;
  Bin*   code;
  Dict*  locals;
  Dict*  nonlocals;
  Objs*  upvals;
};

struct UpVal {
  HEADER;
  UpVal *next;
  union {
    Val *location;
    Val  value;
  };
};

struct Method {
  HEADER;
  Method* next;
  Dict*   cache;
  Tuple*  signature;
  Val     method;
};

typedef struct Frame Frame;

struct Cntl {
  HEADER;
  usize nvals, nframes;
  Frame* frames;
  Val*   vals;
};

// globals --------------------------------------------------------------------
extern Sym*  SymbolTable;
extern List  EmptyList;
extern Tuple EmptyTuple;
extern Str   EmptyString;

#define KEY  0
#define BIND 1

// API -------------------------------------------------------------------------
// common api ------------------------------------------------------------------
void init_obj(void* self, Type type, flags fl);
void mark_obj(void* self);
void destruct_obj(void* self);

bool has_flag(void* self, flags fl);
bool has_flags(void* self, flags fl);
bool set_flag(void* self, flags fl);
bool del_flag(void* self, flags fl);

// lifetimes ------------------------------------------------------------------
Sym*    get_sym(flags fl, char* name);
Stream* mk_stream(flags fl, FILE* ios);
Func*   mk_func(Sym* name, Mtable* type);
Str*    mk_str(usize n, char* data);
List*   mk_list(Val head, List* tail);
Vec*    mk_vec(usize n, Val* args);
Tuple*  mk_tuple(usize n, Val* args);

Bin*    mk_bin(flags fl, usize n, ubyte* data);
void    init_bin(Bin* self, flags fl, usize n, ubyte* data);
void    free_bin(Bin* self);

Dict*   mk_dict(flags fl, usize n, Val* data);
void    init_dict(Dict* dict, flags fl, usize n, Val* data);
void    free_dict(Dict* dict);

Set*    mk_set(Set* self, flags fl, usize n, Val* vals);
void    init_set(Set* self, flags fl, usize n, Val* vals);
void    free_set(Set* self);

Record* mk_record(Record* type, flags fl, usize n, Val* args);

Chunk*  mk_chunk(flags fl, Vec* vals, Bin* code, Dict* locals, Dict* nonlocals, Objs* upvals);
void    init_chunk(Chunk* self, flags fl, Vec* vals, Bin* code, Dict* locals, Dict* nonlocals, Objs* upvals);
void    free_chunk(Chunk* self);

UpVal*  get_upval(flags fl, Val* location);

Method* mk_method(flags fl, Method* next, Dict* cache, Tuple* sig, Val func);

Cntl*   mk_cntl(flags fl, usize nframes, usize nvals, Frame* frames, Val* vals);

// accessors & mutators -------------------------------------------------------
Val   list_nth(List* list, uint n);
List* list_assoc(List* list, Val k);
List* list_tail(List* list, uint n);

Val   dict_get(Dict* dict, Val key);
Val   dict_set(Dict* dict, Val key, Val val);
Val   dict_del(Dict* dict, Val key);

bool  set_has(Set* set, Val key);
bool  set_add(Set* set, Val key);
bool  set_del(Set* set, Val key);

#endif
