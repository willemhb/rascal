#include <string.h>

#include "obj/memory.h"
#include "obj/symbol.h"
#include "obj/type.h"
#include "vm/memory.h"

// parameters
static const size_t init_alloccap  = 2097152; // sizeof(cons_t) * (1<<16)
static const size_t objects_minc   = 512;
static const size_t behaviors_minc = 64;

// global memory managers
extern objects_t Grays;
extern behaviors_t Inits, Marks, Unmarks, Finalizers;

heap_t Heap =
  {
   .obj = { .dtype=&HeapType, .hdrtag=HDR_BITS },

   .grays=&Grays.obj,
   .inits=&Inits.obj,
   .marks=&Marks.obj,
   .unmarks=&Unmarks.obj,
   .finalizers=&Finalizers.obj,

   .allocated=0,
   .alloccap =init_alloccap,

   .read_barrier=false,
   .write_barrier=false
  };

objects_t Grays =
  {
   
  };


// type implementations for global memory managers
