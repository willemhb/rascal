#ifndef rascal_obj_reader_h
#define rascal_obj_reader_h

#include <stdio.h>

#include "obj/type.h"

// function pointer types
typedef value_t (*read_t)(object_t *stream, char dispatch);

// object types
typedef struct rl_read_t     rl_read_t;
typedef struct dispatch_t    dispatch_t;
typedef struct dispatches_t  dispatches_t;
typedef struct reader_t      reader_t;
typedef struct buffer_t      buffer_t;
typedef struct stream_t      stream_t;

struct rl_read_t
{
  OBJECT

  read_t data;
};

struct dispatch_t
{
  OBJECT

  hash_t    hash;
  char      key;
  object_t *handler;
};

struct dispatches_t
{
  OBJECT

  size_t     length;
  size_t     capacity;
  object_t **dispatches;
};

struct reader_t
{
  OBJECT

  object_t *dispatches; // table of reader macros
  object_t *backrefs;   // table of backrefs (typically null)
  object_t *gensyms;    // table of gensyms (typically null)
  object_t *parent;     // next reader
};

struct buffer_t
{
  OBJECT

  size_t length;
  size_t cap;
  char  *data;
};

struct stream_t
{
  OBJECT

  uintptr_t flags;
  value_t   val;
  FILE     *ios;

  object_t *buffer;
  object_t *reader;
};

#endif
