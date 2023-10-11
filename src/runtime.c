#include "util/io.h"

#include "vm.h"
#include "value.h"
#include "memory.h"
#include "runtime.h"

// external API
void initContext(Context* ctx) {
  ctx->ctx = NULL;
}

void freeContext(Context* ctx) {
  ctx->ctx = NULL;
}

void resetContext(Context* ctx) {
  freeContext(ctx);
}

void saveState(Vm* vm, ErrorContext* ctx) {
  assert(ctx != NULL);
  ctx->sp   = vm->interpreter.sp;
  ctx->ip   = vm->interpreter.ip;
  ctx->code = vm->interpreter.code;
  ctx->next = vm->context.ctx;
}

void restoreState(Vm* vm, ErrorContext* ctx) {
  assert(ctx != NULL);
  vm->interpreter.sp   = ctx->sp;
  vm->interpreter.ip   = ctx->ip;
  vm->interpreter.code = ctx->code;
  vm->context.ctx      = ctx->next;
}

static void print_error(const char* fname, const char* fmt, va_list va) {
  fprintf(stderr, "error in %s: ", fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
}

#define SIGNAL_ERROR(fname, fmt)                \
  do {                                          \
    va_list _va;                                \
    va_start(_va, fmt);                         \
    print_error(fname, fmt, _va);               \
    va_end(_va);                                \
    longjmp(RlVm.context.ctx->buf, 1);          \
  } while (false)

void raise(const char* fname, const char* fmt, ...) {
  SIGNAL_ERROR(fname, fmt);
}

bool require(bool test, const char* fname, const char* fmt, ...) {
  if (!test)
    SIGNAL_ERROR(fname, fmt);

  return test;
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

Type argtype(Type expected, Value got, const char* fname) {
  static const char* fmt = "expected value of type %s, got value of type %s";
  Type gotType = valueType(got);
  require(expected == gotType, fmt, fname, nameOfType(expected), nameOfType(gotType));
  return gotType;
}

Type argtypes(size_t n, Value got, const char* fname, ...) {
  static const char* fmt1 = "expected type was %s, actual type was %s";
  static const char* fmt2 = "expected type was %s or %s, actual type was %s";
  static const char* fmt3 = "expected type was %s, %s, or %s, actual type was %s";
  static const char* fmt4 = "expected type was %s, %s, %s, or %s, actual type was %s";

  assert(n > 0);
  va_list va;
  va_start(va, fname);
  Type gt = rascalType(got);

  switch (n) {
    case 1: {
      Type a    = va_arg(va, Type);
      bool test = gt == a;
      va_end(va);
      require(test, fname, fmt1, nameOfType(a), nameOfType(gt));
      break;
    }

    case 2: {
      Type a    = va_arg(va, Type);
      Type b    = va_arg(va, Type);
      bool test = gt == a || gt == b;
      va_end(va);
      require(test, fname, fmt2, nameOfType(a), nameOfType(b), nameOfType(gt));
      break;
    }

    case 3: {
      Type a    = va_arg(va, Type);
      Type b    = va_arg(va, Type);
      Type c    = va_arg(va, Type);
      bool test = gt == a || gt == b || gt == c;
      va_end(va);
      require(test, fname, fmt3,
              nameOfType(a), nameOfType(b), nameOfType(c), nameOfType(gt));
      break;
    }

    case 4: {
      Type a    = va_arg(va, Type);
      Type b    = va_arg(va, Type);
      Type c    = va_arg(va, Type);
      Type d    = va_arg(va, Type);
      bool test = gt == a || gt == b || gt == c || gt == d;
      va_end(va);
      require(test, fname, fmt4,
              nameOfType(a), nameOfType(b), nameOfType(c), nameOfType(d), nameOfType(gt));
      break;
    }

    default: {
      FILE* buffer = tmpfile();
      bool test = false;
      
      for (size_t i=0; !test && i<n; i++) {
        Type opt = va_arg(va, Type);
        test  = gt == opt;

        if (!test) {
          if (i == 0)
            fprintf(buffer, "expected type was %s", nameOfType(opt));
          
        else if (i+1 < n)
          fprintf(buffer, ", %s", nameOfType(opt));
          
        else
          fprintf(buffer, ",or %s", nameOfType(opt));
        }
      }
      
      va_end(va);

      if (!test) {
        fprintf(buffer, ", actual type was %s", nameOfType(gt));
        char* msg = readFile("<tmp>", buffer);
        fprintf(stderr, "error in %s: %s.\n", fname, msg);
        deallocate(NULL, msg, 0);
        longjmp(RlVm.context.ctx->buf, 1);
      } else {
        fclose(buffer);
      }

      break;
    }
  }

  return gt;
}

size_t argcos(size_t n, size_t got, const char* fname, ...) {
  static const char* fmt1 = "expected %zu arguments, got %zu";
  static const char* fmt2 = "expected %zu or %zu arguments, got %zu";
  static const char* fmt3 = "expected %zu, %zu, or %zu arguments, got %zu";
  static const char* fmt4 = "expected %zu, %zu, %zu, or %zu arguments, got %zu";

  assert(n > 0);
  va_list va;
  va_start(va, fname);

  switch (n) {
    case 1: {
      size_t a  = va_arg(va, size_t);
      bool test = got == a;
      va_end(va);
      require(test, fname, fmt1, a, got);
      break;
    }

    case 2: {
      size_t a  = va_arg(va, size_t);
      size_t b  = va_arg(va, size_t);
      bool test = got == a || got == b;
      va_end(va);
      require(test, fname, fmt2, a, b, got);
      break;
    }

    case 3: {
      size_t a  = va_arg(va, size_t);
      size_t b  = va_arg(va, size_t);
      size_t c  = va_arg(va, size_t);
      bool test = got == a || got == b || got == c;
      va_end(va);
      require(test, fname, fmt3, a, b, c, got);
      break;
    }

    case 4: {
      size_t a  = va_arg(va, size_t);
      size_t b  = va_arg(va, size_t);
      size_t c  = va_arg(va, size_t);
      size_t d  = va_arg(va, size_t);
      bool test = got == a || got == b || got == c || got == d;
      va_end(va);
      require(test, fname, fmt4, a, b, c, d, got);
      break;
    }

    default: {
      FILE* buffer = tmpfile();
      bool test = false;
      
      for (size_t i=0; !test && i<n; i++) {
        size_t opt = va_arg(va, size_t);
        test  = got == opt;
        
        if (!test) {
          if (i == 0)
            fprintf(buffer, "expected %zu", opt);
          
        else if (i+1 < n)
          fprintf(buffer, ", %zu", opt);
          
        else
          fprintf(buffer, ",or %zu arguments", opt);
        }
      }
      
      va_end(va);

      if (!test) {
        fprintf(buffer, ", got %zu", got);
        char* msg = readFile("<tmp>", buffer);
        fprintf(stderr, "error in %s: %s.\n", fname, msg);
        deallocate(NULL, msg, 0);
        longjmp(RlVm.context.ctx->buf, 1);
      } else {
        fclose(buffer);
      }

      break;
    }
  }

  return got;
}


size_t vargcos(size_t n, size_t got, const char* fname, ...) {
  static const char* fmt1 = "expected at least %zu arguments, got %zu";
  static const char* fmt2 = "expected %zu or at least %zu arguments, got %zu";
  static const char* fmt3 = "expected %zu, %zu, or at least %zu arguments, got %zu";
  static const char* fmt4 = "expected %zu, %zu, %zu, or at least %zu arguments, got %zu";

  assert(n > 0);
  va_list va;
  va_start(va, fname);

  switch (n) {
    case 1: {
      size_t a  = va_arg(va, size_t);
      bool test = got >= a;
      va_end(va);
      require(test, fname, fmt1, a, got);
      break;
    }

    case 2: {
      size_t a  = va_arg(va, size_t);
      size_t b  = va_arg(va, size_t);
      bool test = got == a || got >= b;
      va_end(va);
      require(test, fname, fmt2, a, b, got);
      break;
    }

    case 3: {
      size_t a  = va_arg(va, size_t);
      size_t b  = va_arg(va, size_t);
      size_t c  = va_arg(va, size_t);
      bool test = got == a || got == b || got >= c;
      va_end(va);
      require(test, fname, fmt3, a, b, c, got);
      break;
    }

    case 4: {
      size_t a  = va_arg(va, size_t);
      size_t b  = va_arg(va, size_t);
      size_t c  = va_arg(va, size_t);
      size_t d  = va_arg(va, size_t);
      bool test = got == a || got == b || got == c || got >= d;
      va_end(va);
      require(test, fname, fmt4, a, b, c, d, got);
      break;
    }

    default: {
      FILE* buffer = tmpfile();
      bool test = false;
      
      for (size_t i=0; !test && i<n; i++) {
        size_t opt = va_arg(va, size_t);

        if (i+1 < n)
          test = got >= opt;

        else
          test = got == opt;
        
        if (!test) {
          if (i == 0)
            fprintf(buffer, "expected %zu", opt);
          
        else if (i+1 < n)
          fprintf(buffer, ", %zu", opt);
          
        else
          fprintf(buffer, ",or at least %zu arguments", opt);
        }
      }
      
      va_end(va);

      if (!test) {
        fprintf(buffer, ", got %zu", got);
        char* msg = readFile("<tmp>", buffer);
        fprintf(stderr, "error in %s: %s.\n", fname, msg);
        deallocate(NULL, msg, 0);
        longjmp(RlVm.context.ctx->buf, 1);
      } else {
        fclose(buffer);
      }

      break;
    }
  }

  return got;
}

#undef SIGNAL_ERROR
