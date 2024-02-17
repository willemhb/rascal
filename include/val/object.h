#ifndef rl_val_object_h
#define rl_val_object_h

#include "val/value.h"

/* C types */
// object flag types
// memory flags
typedef enum {
  BLACK   =0x001ul, // GC mark bit
  GRAY    =0x002ul, // GC trace bit
  NOTRACE =0x004ul, // skip tracing this object
  NOSWEEP =0x008ul, // don't free this object (allocated statically or in C stack)
  NOFREE  =0x010ul, // don't free this object's owned data (allocated statically or in C stack)
  NOHASH  =0x020ul, // don't cache this object's hash (hash field does not store own hash)
  NORESIZE=0x040ul, // don't resize this object's owned data
  HASHED  =0x080ul, // valid hash
  EDITP   =0x100ul, // object is open for updating
} MemFl;

/* common object header */
struct Obj {
  Obj*     next;   // invasive live objects list
  hash_t   hash;   // cached hash code 
  Type*    type;   // type information
  Dict*    meta;   // language metadata (visible to users)
  wflags_t flags;  // runtime metadata (not visible to users, mostly memory information)
  byte_t   data[];
};

/* Mutable dynamic array for storing untagged objects.

   Internal use only (for now). */

struct Objects {
  Obj    obj;
  Obj**  data;
  size_t cnt;
  size_t cap;
};

/* globals */
extern Type ObjectsType;

/* external APIs */
/* Common Obj API */
#define HEADER struct Obj obj

#define is_obj(x)     has_tag(x, OBJ_TAG)
#define as_obj(x)     as(Obj*, untag48, x)
#define is_black(x)   get_mfl(x, BLACK)
#define is_gray(x)    get_mfl(x, GRAY)
#define is_notrace(x) get_mfl(x, NOTRACE)
#define is_nosweep(x) get_mfl(x, NOSWEEP)
#define is_nofree(x)  get_mfl(x, NOFREE)
#define is_hashed(x)  get_mfl(x, HASHED)
#define is_nohash(x)  get_fml(x, NOHASH)
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
void*  new_obj(Type* type, wflags_t fl, size_t extra);
void*  clone_obj(void* obj);
void   init_obj(void* slf, Type* type, wflags_t fl, size_t extra);
void   finalize_obj(void* obj);
void   dealloc_obj(void* obj);
void   set_obj_hash(void* slf, hash_t h);

// Objects API
Objects* new_objects(void);
void     init_objects(Objects* arr);
void     free_objects(Objects* arr);
void     resize_objects(Objects* arr, size_t new_cnt);
size_t   objects_push(Objects* arr, Obj* obj);
size_t   objects_write(Objects* arr, size_t n, Obj** data);
size_t   objects_pushn(Objects* arr, size_t n, ...);
Obj*     objects_pop(Objects* arr);
Obj*     objects_popn(Objects* arr, size_t n);

#endif
