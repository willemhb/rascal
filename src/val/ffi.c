#include <dlfcn.h>
#include <ffi.h>
#include <string.h>

#include "val/ffi.h"
#include "val/str.h"
#include "val/sym.h"
#include "val/list.h"
#include "val/port.h"
#include "val/primitive.h"
#include "vm.h"
#include "vm/memory.h"
#include "vm/error.h"

// Forward declarations
void print_lib_handle(Port* ios, Expr x);
void print_foreign_fn(Port* ios, Expr x);
void trace_lib_handle(RlState* rls, void* ptr);
void trace_foreign_fn(RlState* rls, void* ptr);
void free_lib_handle(RlState* rls, void* ptr);

// Type objects
Type LibHandleType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_LIBHANDLE,
  .obsize   = sizeof(LibHandle),
  .print_fn = print_lib_handle,
  .trace_fn = trace_lib_handle,
  .free_fn  = free_lib_handle
};

Type ForeignFnType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_FOREIGNFN,
  .obsize   = sizeof(ForeignFn),
  .print_fn = print_foreign_fn,
  .trace_fn = trace_foreign_fn
};

// LibHandle implementation
LibHandle* mk_lib_handle(RlState* rls, void* handle, Str* path) {
  LibHandle* lh = mk_obj(rls, &LibHandleType, 0);
  lh->handle = handle;
  lh->path = path;
  return lh;
}

LibHandle* mk_lib_handle_s(RlState* rls, void* handle, Str* path) {
  LibHandle* out = mk_lib_handle(rls, handle, path);
  stack_push(rls, tag_obj(out));
  return out;
}

void close_lib_handle(RlState* rls, LibHandle* lh) {
  (void)rls;
  if (lh->handle != NULL) {
    dlclose(lh->handle);
    lh->handle = NULL;
  }
}

void print_lib_handle(Port* ios, Expr x) {
  LibHandle* lh = as_lib_handle(x);
  pprintf(ios, "<lib-handle:%s>", str_val(lh->path));
}

void trace_lib_handle(RlState* rls, void* ptr) {
  LibHandle* lh = ptr;
  mark_obj(rls, lh->path);
}

void free_lib_handle(RlState* rls, void* ptr) {
  LibHandle* lh = ptr;
  close_lib_handle(rls, lh);
}

// ForeignFn implementation
ForeignFn* mk_foreign_fn(RlState* rls, void* fn, Str* name, LibHandle* lib) {
  ForeignFn* ff = mk_obj(rls, &ForeignFnType, 0);
  ff->fn = fn;
  ff->name = name;
  ff->lib = lib;
  return ff;
}

ForeignFn* mk_foreign_fn_s(RlState* rls, void* fn, Str* name, LibHandle* lib) {
  ForeignFn* out = mk_foreign_fn(rls, fn, name, lib);
  stack_push(rls, tag_obj(out));
  return out;
}

void print_foreign_fn(Port* ios, Expr x) {
  ForeignFn* ff = as_foreign_fn(x);
  pprintf(ios, "<foreign-fn:%s>", str_val(ff->name));
}

void trace_foreign_fn(RlState* rls, void* ptr) {
  ForeignFn* ff = ptr;
  mark_obj(rls, ff->name);
  mark_obj(rls, ff->lib);
}

// Type keyword symbol mapping
FfiTypeCode sym_to_ffi_type(RlState* rls, Sym* sym) {
  char* name = sym_val(sym);

  if (strcmp(name, ":void") == 0)   return FFI_T_VOID;
  if (strcmp(name, ":int") == 0)    return FFI_T_INT;
  if (strcmp(name, ":double") == 0) return FFI_T_DOUBLE;
  if (strcmp(name, ":ptr") == 0)    return FFI_T_PTR;
  if (strcmp(name, ":str") == 0)    return FFI_T_STR;

  eval_error(rls, "unknown FFI type: %s", name);
  return FFI_T_VOID; // unreachable
}

ffi_type* ffi_type_code_to_libffi(FfiTypeCode tc) {
  switch (tc) {
    case FFI_T_VOID:   return &ffi_type_void;
    case FFI_T_INT:    return &ffi_type_sint;
    case FFI_T_DOUBLE: return &ffi_type_double;
    case FFI_T_PTR:    return &ffi_type_pointer;
    case FFI_T_STR:    return &ffi_type_pointer;
    default:           return &ffi_type_void;
  }
}

// Marshal Lisp value to C value
// Returns pointer to storage (caller provides buffer)
static void* marshal_to_c(RlState* rls, FfiTypeCode tc, Expr arg, void* buf, char** str_storage) {
  switch (tc) {
    case FFI_T_INT: {
      int* p = buf;
      *p = (int)as_num_s(rls, arg);
      return p;
    }
    case FFI_T_DOUBLE: {
      double* p = buf;
      *p = as_num_s(rls, arg);
      return p;
    }
    case FFI_T_PTR: {
      void** p = buf;
      *p = as_ptr(arg);
      return p;
    }
    case FFI_T_STR: {
      Str* s = as_str_s(rls, arg);
      // Duplicate string - caller is responsible for freeing str_storage
      *str_storage = duplicates(rls, str_val(s));
      void** p = buf;
      *p = *str_storage;
      return p;
    }
    default:
      eval_error(rls, "cannot marshal void to C");
      return NULL;
  }
}

// Marshal C return value to Lisp
static Expr marshal_from_c(RlState* rls, FfiTypeCode tc, void* ret_val) {
  switch (tc) {
    case FFI_T_VOID:
      return NUL;
    case FFI_T_INT: {
      int v = *(int*)ret_val;
      return tag_num((Num)v);
    }
    case FFI_T_DOUBLE: {
      double v = *(double*)ret_val;
      return tag_num(v);
    }
    case FFI_T_PTR: {
      void* v = *(void**)ret_val;
      return tag_ptr(v);
    }
    case FFI_T_STR: {
      char* v = *(char**)ret_val;
      if (v == NULL) return NUL;
      Str* s = mk_str(rls, v);
      return tag_obj(s);
    }
    default:
      return NUL;
  }
}

// Main FFI call implementation
Expr ffi_do_call(RlState* rls, ForeignFn* ffn, FfiTypeCode ret_type,
                 int argc, FfiTypeCode* arg_types, Expr* args) {

  require(rls, ffn->fn != NULL, "foreign function pointer is NULL");

  // Set up ffi_cif
  ffi_cif cif;
  ffi_type* ret_ffi = ffi_type_code_to_libffi(ret_type);
  ffi_type** arg_ffis = NULL;
  void** arg_vals = NULL;
  char** str_storage = NULL;  // For strings we need to free

  // Storage for argument values (union of largest types)
  typedef union { int i; double d; void* p; } ArgStorage;
  ArgStorage* arg_store = NULL;

  if (argc > 0) {
    arg_ffis = allocate(rls, argc * sizeof(ffi_type*));
    arg_vals = allocate(rls, argc * sizeof(void*));
    arg_store = allocate(rls, argc * sizeof(ArgStorage));
    str_storage = allocate(rls, argc * sizeof(char*));

    for (int i = 0; i < argc; i++) {
      arg_ffis[i] = ffi_type_code_to_libffi(arg_types[i]);
      str_storage[i] = NULL;
      arg_vals[i] = marshal_to_c(rls, arg_types[i], args[i],
                                  &arg_store[i], &str_storage[i]);
    }
  }

  // Prepare the call
  ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc, ret_ffi, arg_ffis);

  if (status != FFI_OK) {
    // Cleanup on error
    if (str_storage) {
      for (int i = 0; i < argc; i++)
        if (str_storage[i]) release(rls, str_storage[i], 0);
      release(rls, str_storage, argc * sizeof(char*));
    }
    if (arg_ffis) release(rls, arg_ffis, argc * sizeof(ffi_type*));
    if (arg_vals) release(rls, arg_vals, argc * sizeof(void*));
    if (arg_store) release(rls, arg_store, argc * sizeof(ArgStorage));

    eval_error(rls, "ffi_prep_cif failed");
  }

  // Storage for return value
  union { int i; double d; void* p; char* s; } ret_val;

  // Make the call
  ffi_call(&cif, FFI_FN(ffn->fn), &ret_val, arg_vals);

  // Convert result
  Expr result = marshal_from_c(rls, ret_type, &ret_val);

  // Cleanup - IMPORTANT: free duplicated strings
  if (str_storage) {
    for (int i = 0; i < argc; i++)
      if (str_storage[i]) release(rls, str_storage[i], 0);
    release(rls, str_storage, argc * sizeof(char*));
  }
  if (arg_ffis) release(rls, arg_ffis, argc * sizeof(ffi_type*));
  if (arg_vals) release(rls, arg_vals, argc * sizeof(void*));
  if (arg_store) release(rls, arg_store, argc * sizeof(ArgStorage));

  return result;
}
