#ifndef data_object_h
#define data_object_h

#include <stdio.h>

#include "value.h"

// C types
struct object {
  object_t* next;       // invasive live objects list
  uint64    hash  : 48;
  uint64    flags :  8;
  uint64    type  :  6;
  uint64    black :  1;
  uint64    gray  :  1;
  ubyte     space[0];
};

typedef enum {
  FROZEN = 0x80,
  HASHED = 0x40,
  STATIC = 0x20
} objfl_t;

#define HEADER object_t obj

// APIs & utilities
#define head(obj)    ((object_t*)(obj))
#define is_object(x) (valtype(x) == OBJECT)
#define as_object(x) rl_asa(x, WVMASK, object_t*)

#define INIT_HEADER(obj, datatype, fl)            \
  do {                                            \
    extern object_t* LiveObjects;                 \
    object_t* __obj  = (object_t*)(obj);          \
                                                  \
    __obj->next  = LiveObjects;                   \
    LiveObjects  = __obj;                         \
    __obj->hash  = 0;                             \
    __obj->flags = fl;                            \
    __obj->type  = datatype;                      \
    __obj->gray  = true;                          \
    __obj->black = false;                         \
  } while (false)

#define MARK(obj)   Mark[head(obj)->type]
#define FREE(obj)   Free[head(obj)->type]
#define SIZEOF(obj) SizeOf[head(obj)->type]

value_t object(void* ox);
type_t objtype(void* ox);
type_t obj_typeof(void* ox);
usize obj_sizeof(void* ox);
bool obj_isa(void* ox, type_t tx);
void obj_mark(void* ox);
void obj_free(void* ox);
bool has_flag(void* ox, flags fl);
bool set_flag(void* ox, flags fl);
bool del_flag(void* ox, flags fl);
bool has_wflag(void* ox, flags fl, flags m);
bool set_wflag(void* ox, flags fl, flags m);
bool del_wflag(void* ox, flags fl, flags m);
void mark_objects(usize n, void* oxs);

// globals
extern void  (*Mark[NTYPES])(void* ox);
extern void  (*Free[NTYPES])(void* ox);
extern usize (*SizeOf[NTYPES])(void* ox);


#endif
