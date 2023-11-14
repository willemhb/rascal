#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* C types */
// object flag types
// allocation flags
typedef enum {
  BLACK  =0x80000000u,
  GRAY   =0x40000000u,
  NOTRACE=0x20000000u,
  NOSWEEP=0x10000000u,
  NOFREE =0x08000000u,
  HASHED =0x04000000u,
  EDITP  =0x02000000u,
} MemFl;

// common object header
struct Obj {
  Obj*    next;   // invasive live objects list
  hash_t  hash;   // cached hash code 
  Type*   type;   // type information
  Dict*   meta;   // metadta
  flags_t flags;  // misc flags
  flags_t memfl;  // memory flags
  byte_t  data[];
};

/* external APIs */
// convenience macros
#define HEADER struct Obj obj

#define is_obj(x)     has_tag(x, OBJ_TAG)
#define as_obj(x)     as(Obj*, untag48, x)
#define is_black(x)   get_mfl(x, BLACK)
#define is_gray(x)    get_mfl(x, GRAY)
#define is_notrace(x) get_mfl(x, NOTRACE)
#define is_nosweep(x) get_mfl(x, NOSWEEP)
#define is_nofree(x)  get_mfl(x, NOFREE)
#define is_hashed(x)  get_mfl(x, HASHED)
#define is_editp(x)   get_mfl(x, EDITP)

// casts, predicates, accessors, bit twiddling
Type*  type_of_obj(void* obj);
size_t size_of_obj(void* obj);
bool   has_type_obj(void* obj, Type* type);
void   mark_obj(void* obj);
void   trace_obj(void* obj);
bool   get_mfl_obj(void* obj, flags_t mfl);
bool   set_mfl_obj(void* obj, flags_t mfl);
bool   del_mfl_obj(void* obj, flags_t mfl);
bool   get_fl_obj(void* obj, flags_t fl);
bool   set_fl_obj(void* obj, flags_t fl);
bool   del_fl_obj(void* obj, flags_t fl);
Dict*  get_meta_dict_obj(void* obj);
Dict*  set_meta_dict_obj(void* obj, Dict* meta);
Value  get_meta_obj(void* obj, Value key);
Value  set_meta_obj(void* obj, Value key, Value val);
Dict*  join_meta_obj(void* obj, Dict* meta);

// lifetime methods
void* new_obj(Type* type, flags_t flags, flags_t memfl, size_t extra);
void* clone_obj(void* obj);
void  init_obj(void* slf, Type* type, flags_t flags, flags_t memfl);
void  finalize_obj(void* obj);
void  free_obj(void* obj);

#endif
