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
  FROZEN   = 0x80,
  HASHED   = 0x40,
  STATIC   = 0x20
} objfl_t;

#define HEADER object_t obj

// APIs & utilities
#define head(x)      ((object_t*)(x))
#define is_object(x) (value_type(x) == OBJECT)
#define as_object(x) ((object_t*)(((value_t)(x))&WVMASK))

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

// globals
extern usize ValueSize[NTYPES];
extern usize (*SizeOf[NTYPES])(void* obj);
extern void  (*Trace[NTYPES])(void* obj);
extern void  (*Free[NTYPES])(void* obj);

// API & utilities
value_t object(void* obj);

type_t object_type(void* obj);
void mark_object(void* obj);
void free_object(void* obj);

bool object_hasfl(void* obj, flags fl);
bool object_setfl(void* obj, flags fl);
bool object_delfl(void* obj, flags fl);

// toplevel initialization
void object_init(void);

#endif
