#include <string.h>
#include <stdio.h>

#include "object.h"
#include "metaobject.h"
#include "text.h"
#include "atom.h"
#include "interpreter.h"

#include "memory.h"
#include "number.h"
#include "hashing.h"

// lifetime API ---------------------------------------------------------------
int init_object(void* self, void* ini) {
  extern object_t* LiveObjects;

  assert(self);
  assert(ini);

  object_t* oself = self;
  object_init_t* oini = ini;

  oself->next   = LiveObjects;
  LiveObjects   = oself;
  oself->hash   = oini->hash;
  oself->flags  = oini->flags;
  oself->hashed = oini->hashed;
  oself->frozen = oini->frozen;
  oself->type   = oini->type->type.idno & 0x3f;
  oself->gray   = true;
  oself->black  = false;

  int out = 0;

  if (oini->type->init)
    out = oini->type->init(self, ini);

  return out;
}

void mark_object(void* self) {
  if (!self)
    return;

  object_t* obj = self;

  if (obj->black)
    return;

  obj->black = true;

  data_type_t* type = (data_type_t*)type_of(obj);

  if (type->trace)
    push_gray(obj);

  else
    obj->gray = true;
}

void free_object(void* self) {
  if (!self)
    return;
  
  object_t* obj     = self;
  usize dealloc     = rl_size_of(obj);
  data_type_t* type = (data_type_t*)type_of(obj);

  if (type->free)
    type->free(obj);

  deallocate(obj, dealloc);
}

// traversal utilities --------------------------------------------------------
void mark_objects(usize n, object_t** objs) {
  for (usize i=0; i<n; i++)
    mark_object(objs[i]);
}

void mark_values(usize n, value_t* vals) {
  for (usize i=0; i<n; i++)
    mark_value(vals[i]);
}

void print_values(usize n, value_t* vals, port_t* ios, bool paired) {
  for (usize i=0; i<n; i++) {
    if (paired && vals[i] == NOTFOUND)
      continue;
      
    rl_print(vals[i], ios);
    
    if (i + 1 < n) {
      if (paired && i > 0 && i % 2 == 0)
        rl_putc(',', ios);
      
      rl_putc(' ', ios);
    }
  }
}

void print_objects(usize n, object_t** objs, port_t* ios, bool paired) {
  for (usize i=0; i<n; i++) {
    if (paired && objs[i] == NULL)
      continue;

    rl_print(objs[i], ios);

    if (i + 1 < n) {
      if (paired && i > 0 && i % 2 == 0)
        rl_putc(',', ios);
    
      rl_putc(' ', ios);
    }
  }
}

int compare_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs) {
  usize maxc = MAX(xn, yn);

  int o;

  for (usize i=0; i<maxc; i++) {
    if ((o=rl_compare(xobjs[i], yobjs[i])))
      return o;
  }

  return 0 - (xn < yn) + (xn > yn);
}

int compare_values(usize xn, value_t* xvals, usize yn, value_t* yvals) {
  usize maxc = MAX(xn, yn);

  int o;

  for (usize i=0; i<maxc; i++) {
    if ((o=rl_compare(xvals[i], yvals[i])))
      return o;
  }

  return 0 - (xn < yn) + (xn > yn);
}

bool equal_objects(usize xn, object_t** xobjs, usize yn, object_t** yobjs) {
  if (xn != yn)
    return false;

  for (usize i=0; i<xn; i++) {
    if (!rl_equal(xobjs[i], yobjs[i]))
      return false;
  }

  return true;
}

bool equal_values(usize xn, value_t* xvals, usize yn, value_t* yvals) {
  if (xn != yn)
    return false;

  for (usize i=0; i<xn; i++) {
    if (!rl_equal(xvals[i], yvals[i]))
      return false;
  }

  return true;
}

uhash hash_objects(usize nx, object_t** objs) {
  uhash accum = 0;

  for (usize i=0; i<nx; i++)
    accum = mix_2_hashes(accum, rl_hash(objs[i]));

  return accum;
}

uhash hash_values(usize nx, value_t* vals) {
  uhash accum = 0;

  for (usize i=0; i<nx; i++)
    accum = mix_2_hashes(accum, rl_hash(vals[i]));

  return accum;
}

