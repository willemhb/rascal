#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* C types */
// common object header
typedef enum {
  BLACK  =0x80000000U,
  GRAY   =0x40000000U,
  NOTRACE=0x20000000U,
  NOSWEEP=0x10000000U,
  NOFREE =0x08000000U,
  EDITP  =0x04000000U, // for HAMT types, indicates that the object can be modified in-place
} MemFl;

struct Obj {
  Obj*    next;   // invasive live objects list
  hash_t  hash;   // cached hash code 
  Type*   type;   // type information
  Dict*   meta;   // metadta
  flags_t flags;  // misc flags
  flags_t memfl;  // memory flags
  byte_t  data[];
};

/* API */
// convenience macros
#define HEADER struct Obj obj

// casts, predicates, accessors, bit twiddling
bool   is_obj(Value x);
Obj*   as_obj(Value x);
size_t size_of_obj(Obj* obj);
bool   get_obj_fl(Obj* obj, flags_t fl);
bool   set_obj_fl(Obj* obj, flags_t fl);
bool   del_obj_fl(Obj* obj, flags_t fl);
bool   get_obj_mfl(Obj* obj, flags_t mfl);
bool   set_obj_mfl(Obj* obj, flags_t mfl);
bool   del_obj_mfl(Obj* obj, flags_t mfl);
bool   obj_is_black(Obj* obj);
bool   obj_is_gray(Obj* obj);
bool   obj_is_notrace(Obj* obj);
bool   obj_is_nosweep(Obj* obj);
bool   obj_is_nofree(Obj* obj);
void   obj_mark_black(Obj* obj);
void   obj_unmark_black(Obj* obj);
void   obj_mark_gray(Obj* obj);
void   obj_unmark_gray(Obj* obj);
void   mark_obj(Obj* obj);
void   unmark_obj(Obj* obj);

// metadata
Dict* get_obj_meta(Obj* obj);
void set_obj_meta(Obj* obj, Dict* meta);
void put_obj_meta(Obj* obj, Value key, Value val);
void merge_obj_meta(Obj* obj, Dict* meta);

// lifetime methods
Obj* new_obj(Type* type, flags_t flags, flags_t memfl, size_t extra);
Obj* clone_obj(Obj* obj);
void init_obj(Obj* slf, Type* type, flags_t flags, flags_t memfl);
void trace_obj(Obj* obj);
void finalize_obj(Obj* obj);
void free_obj(Obj* obj);

#endif
