#include <stdarg.h>

#include "util/io.h"
#include "util/number.h"
#include "util/hashing.h"

#include "lang/envt.h"
#include "lang/equal.h"

#include "vm/error.h"
#include "vm/memory.h"
#include "vm/envt.h"

#include "val/symbol.h"
#include "val/type.h"
#include "val/vector.h"

// external API
void save_state(ErrFrame* err) {
  assert(err != NULL);

  err->next     = Ctx.err;
  err->gcframes = Ctx.h.frames;
  err->r        = Ctx.r;
  err->c        = Ctx.c;
  err->i        = Ctx.i;
}

void restore_state(ErrFrame* err) {
  assert(err != NULL);

  Ctx.err      = err->next;
  Ctx.h.frames = err->gcframes;
  Ctx.r        = err->r;
  Ctx.c        = err->c;
  Ctx.i        = err->i;
}

static void vprint_error(const char* fname, const char* fmt, va_list va) {
  fprintf(stderr, "error in %s: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
}

static void print_error(const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vprint_error(fname, fmt, va);
  va_end(va);
}

static void rl_longjmp(int status) {
  if (Ctx.err == NULL) {
    fprintf(stderr, "Exiting due to unhandled runtime error.\n");
    exit(1);
  } else {
    close_upvals(0); // protectively close upvalues, we don't know what will happen to the stack
    longjmp(Ctx.err->Cstate, status);
  }
}

#define SIGNAL_ERROR(fname, fmt)                \
  do {                                          \
    va_list _va;                                \
    va_start(_va, fmt);                         \
    vprint_error(fname, fmt, _va);              \
    va_end(_va);                                \
    rl_longjmp(1);                              \
  } while (false)

void error(const char* fname, const char* fmt, ...) {
  SIGNAL_ERROR(fname, fmt);
}

bool require(bool test, const char* fname, const char* fmt, ...) {
  if (!test)
    SIGNAL_ERROR(fname, fmt);

  return test;
}

size_t bound_lt(size_t max, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` of size %zu";
  
  require(got > -1 && (size_t)got < max, fname, fmt, got, obname, max);

  return got;
}

size_t bound_le(size_t max, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` of size %zu";

  require(got > -1 && (size_t)got <= max, fname, fmt, got, obname, max);

  return got;
}

size_t bound_gt(size_t min, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` with minimum index %zu";

  require(got > -1 && (size_t)got > min, fname, fmt, got, obname, min);

  return got;
}

size_t bound_ge(size_t min, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` with minimum index %zu";

  require(got > -1 && (size_t)got >= min, fname, fmt, got, obname, min);

  return got;
}

size_t bound_gl(size_t min, size_t max, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` with valid indices between %zu and %zu";
  
  require(got > -1 && (size_t)got > min && (size_t)got < max, fname, fmt, got, obname, min, max);
  
  return got;
}

size_t bound_gel(size_t min, size_t max, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` with valid indices between %zu and %zu";

  require(got > -1 && (size_t)got >= min && (size_t)got < max, fname, fmt, got, obname, min, max);

  return got; 
}

size_t bound_gle(size_t min, size_t max, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` with valid indices between %zu and %zu";

  require(got > -1 && (size_t)got > min && (size_t)got <= max, fname, fmt, got, obname, min, max);

  return got; 
}

size_t bound_gele(size_t min, size_t max, long got, const char* fname, const char* obname) {
  static const char* fmt = "index %l out of bounds for `%s` with valid indices between %zu and %zu";

  require(got > -1 && (size_t)got >= min && (size_t)got <= max, fname, fmt, got, obname, min, max);

  return got; 
}

size_t argco(size_t expected, size_t got, const char* fname) {
  static const char* fmt = "expected exactly %zu arguments, got %zu";
  require(expected == got, fname, fmt, expected, got);
  return got;
}

size_t vargco(size_t expected, size_t got, const char* fname) {
  static const char* fmt = "expected at least %zu arguments, got %zu";
  require(got >= expected, fname, fmt, expected, got);
  return got;
}

Type* argtype(Type* expect, Value got, const char* fname) {
  static const char* fmt = "expected value of type %s, got value of type %s";
  Type* got_type;

  got_type = type_of(got);
  require(is_instance(expect, got_type),
          fmt, fname, expect->name->name, got_type->name->name);
  return got_type;
}

Type* argtypes(size_t expected, Value got, const char* fname, ...) {
  static const char* fmt1 = "expected value of type %s, got value of type %s";
  static const char* fmt2 = "expected value of type %s or %s, got value of type %s";
  static const char* fmt3 = "expected value of type %s, %s, or %s, got value of type %s";

  va_list va;
  bool okay;
  FILE* tmp;
  Type* tg, *tx, *ty, *tz;

  va_start(va, fname);
  okay = true;
  tg = type_of(got);

  if (expected == 1) {
    tx = va_arg(va, Type*);
    va_end(va);
    require(is_instance(tg, tx), fname, fmt1, tx->name->name, tg->name->name);
  } else if (expected == 2) {
    tx = va_arg(va, Type*);
    ty = va_arg(va, Type*);
    va_end(va);
    require(is_instance(tg, tx) || is_instance(tg, ty), fname, fmt2,
            tx->name->name, ty->name->name, tg->name->name);
  } else if (expected == 3) {
    tx = va_arg(va, Type*);
    ty = va_arg(va, Type*);
    tz = va_arg(va, Type*);
    va_end(va);
    require(is_instance(tg, tx) || is_instance(tg, ty) || is_instance(tg, tz),
            fname, fmt3, tx->name->name, ty->name->name,
            tz->name->name, tg->name->name);
  } else {
    tmp = tmpfile();
    okay = false;

    assert(tmp != NULL);

    for (size_t i=0; !okay && i<expected; i++) {
      tx   = va_arg(va, Type*);
      okay = is_instance(tg, tx);

      if (!okay) {
        if (i == 0)
          fprintf(tmp, "expected a value of type %s", tx->name->name);

        else if (i + 1 < expected)
          fprintf(tmp, ", %s", tx->name->name);

        else
          fprintf(tmp, ", or %s, got a value of type %s",
                  tx->name->name, tg->name->name);
      }
    }

    if (!okay) {
      char* msg = read_file("<tmp>", tmp);
      print_error(fname, msg);
      deallocate(NULL, msg, 0);
      rl_longjmp(1);
    }
  }

  return tg;
}

size_t argcos(size_t expected, size_t got, const char* fname, ...) {
  static const char* fmt1 = "expected %zu arguments, got %zu";
  static const char* fmt2 = "expected %zu or %zu arguments, got %zu";
  static const char* fmt3 = "expected %zu, %zu, or %zu arguments, got %zu";

  va_list va;
  bool okay;
  FILE* tmp;
  size_t ex, ey, ez;

  va_start(va, fname);
  okay = true;

  if (expected == 1) {
    ex = va_arg(va, size_t);
    va_end(va);
    require(got == ex, fname, fmt1, ex, got);
  } else if (expected == 2) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got == ey, fname, fmt2, ex, ey, got);
  } else if (expected == 3) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    ez = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got == ey || got == ez, fname, fmt3, ex, ey, ez, got);
  } else {
    tmp = tmpfile();
    okay = false;

    assert(tmp != NULL);

    for (size_t i=0; !okay && i<expected; i++) {
      ex   = va_arg(va, size_t);
      okay = got == ex;

      if (!okay) {
        if (i == 0)
          fprintf(tmp, "expected %zu", ex);

        else if (i + 1 < expected)
          fprintf(tmp, ", %zu", ex);

        else
          fprintf(tmp, ", or %zu arguments, got  %zu", ex, got);
      }
    }

    if (!okay) {
      char* msg = read_file("<tmp>", tmp);
      print_error(fname, msg);
      deallocate(NULL, msg, 0);
      rl_longjmp(1);
    }
  }

  return got;
}


size_t vargcos(size_t expected, size_t got, const char* fname, ...) {
  static const char* fmt1 = "expected at least %zu arguments, got %zu";
  static const char* fmt2 = "expected %zu or at least %zu arguments, got %zu";
  static const char* fmt3 = "expected %zu, %zu, or at least %zu arguments, got %zu";

  va_list va;
  bool okay;
  FILE* tmp;
  size_t ex, ey, ez;

  va_start(va, fname);
  okay = true;

  if (expected == 1) {
    ex = va_arg(va, size_t);
    va_end(va);
    require(got >= ex, fname, fmt1, ex, got);
  } else if (expected == 2) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got >= ey, fname, fmt2, ex, ey, got);
  } else if (expected == 3) {
    ex = va_arg(va, size_t);
    ey = va_arg(va, size_t);
    ez = va_arg(va, size_t);
    va_end(va);
    require(got == ex || got == ey || got >= ez, fname, fmt3, ex, ey, ez, got);
  } else {
    tmp = tmpfile();
    okay = false;

    assert(tmp != NULL);

    for (size_t i=0; !okay && i<expected; i++) {
      ex   = va_arg(va, size_t);
      okay = i + 1 == expected ? got >= ex : got == ex;

      if (!okay) {
        if (i == 0)
          fprintf(tmp, "expected %zu", ex);

        else if (i + 1 < expected)
          fprintf(tmp, ", %zu", ex);

        else
          fprintf(tmp, ", or at least %zu arguments, got  %zu", ex, got);
      }
    }

    if (!okay) {
      char* msg = read_file("<tmp>", tmp);
      print_error(fname, msg);
      deallocate(NULL, msg, 0);
      rl_longjmp(1);
    }
  }

  return got;
}

#undef SIGNAL_ERROR

/* Initialization. */
void vm_init_error(void) {
  Ctx.err = NULL;
}
