#include "util/hashing.h"
#include "util/collection.h"
#include "util/unicode.h"

#include "vm/memory.h"
#include "vm/context.h"

#include "val/text.h"
#include "val/table.h"
#include "val/environment.h"

/* Globals */
/* Magic numbers */
#define LOADF 0.625

/* Global state objects */
StringTable StringCache = {
  .data =NULL,
  .cnt  =0,
  .cap  =0,
  .nts  =0
};

GLOBAL_MUT_DICT(ModuleNameCache);
GLOBAL_MUT_DICT(ModulePathCache);
GLOBAL_MUT_DICT(ToplevelModuleDepends);
GLOBAL_MUT_DICT(ToplevelModuleBindings);

Module ToplevelModule = {};

RlContext Context = {
  .string_cache     =&StringCache,
  .module_name_cache=&ModuleNameCache,
  .module_path_cache=&ModulePathCache,
  .toplevel_module  =&ToplevelModule,
  .gensym_counter   =0
};

/* Internal APIs */
/* String Table API */
// dummy string object with no name to use as a tombstone for the global string table
String Tombstone = {
  .obj={
    .type =&StringType,
    .meta =&EmptyDict,
    .flags=BLACK|NOTRACE|NOSWEEP|NOFREE|NOHASH
  },
  .chars=NULL,
  .arity=0
};

static void init_string_table(StringTable* st);
static void free_string_table(StringTable* st);
static void resize_string_table(StringTable* st, size_t new_cnt);
static String** find_string_table_entry(String** ss, size_t c, const char* chs, hash_t h);

static void init_string_table(StringTable* st) {
  st->data = NULL;
  st->cnt  = 0;
  st->cap  = 0;
  st->nts  = 0;
}

static void free_string_table(StringTable* st) {
  deallocate(st->data, 0, false);
  init_string_table(st);
}

static void resize_string_table(StringTable* st, size_t new_cnt) {
  if (new_cnt == 0)
    free_string_table(st);

  else {
    size_t old_cnt, old_cap, new_cap;

    old_cnt = st->cnt;
    old_cap = st->cap;
    new_cap = pad_table_size(old_cnt, new_cnt, old_cap, LOADF);

    if (new_cap != old_cap) {
      String** strings;

      strings = allocate(new_cap * sizeof(String*), false);

      /* rehash */
      if (st->data != NULL) {
        String** src, ** dst;

        st->cnt = 0;
        st->nts = 0;

        for (size_t i=0; i<old_cap; i++) {
          src = &st->data[i];

          if (*src == NULL || *src == &Tombstone)
            continue;

           dst = find_string_table_entry(strings, new_cap, (*src)->chars, (*src)->obj.hash);
          *dst = *src;
          st->cnt++;
        }

        deallocate(st->data, 0, false);
      }

      st->data = strings;
      st->cap  = new_cap;
    }
  }
}

static String** find_string_table_entry(String** strings, size_t c, const char* chars, hash_t h) {
  size_t i, m;
  String** ts;

  m  = c - 1;
  i  = h & m;
  ts = NULL;

  for (;;) {
    String** string = &strings[i];

    if (*string == NULL)
      return ts ? ts : string;
    else if (*string == &Tombstone)
      ts = ts ? : string;
    else if (strcmp(chars, (*string)->chars) == 0)
      return string;
    else
      i = (i + 1) & m;
  }
}


/* External APIs */
/* string interning/uninterning */
String* intern_string(const char* chars, String* obj) {
  hash_t h;
  StringTable* st;
  String** location;

  // the 'obj' argument is an optional statically allocated string to add to the table.
  // this allows special objects like the empty string to be registered in the table.
  
  h = hash_string(chars);
  st = Context.string_cache;

  resize_string_table(st, st->cnt+1);
  location = find_string_table_entry(st->data, st->cap, chars, h);

  if (*location == NULL) {
    
    if (obj == NULL)
      obj = new_str(chars, strlen(chars), h);

    else
      set_obj_hash(obj, h);

    *location = obj;
    st->cnt++;
  } else if (*location == &Tombstone) {
    if (obj == NULL)
      obj = new_str(chars, strlen(chars), h);

    else
      set_obj_hash(obj, h);

    *location = obj;
    st->nts--;
  }

  return *location;
}

void disintern_string(String* s) {
  char* chs;
  hash_t h;
  StringTable* st;
  String** l;
  
  chs = s->chars;
  h   = s->obj.hash;
  st  = Context.string_cache;

  if (st->cap > 0) {
    l   = find_string_table_entry(st->data, st->cap, chs, h);

    if (*l != NULL && *l != &Tombstone) {
      *l = &Tombstone;
      st->nts++;
    }
  }
}
