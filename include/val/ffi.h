#ifndef rl_ffi_h
#define rl_ffi_h

#include "val/val.h"
#include "val/str.h"
#include <ffi.h>

// LibHandle - wraps a dlopen() handle
struct LibHandle {
  HEAD;
  void* handle;      // dlopen() handle
  Str* path;         // library path for debugging/printing
};

// ForeignFn - wraps a dlsym() function pointer
struct ForeignFn {
  HEAD;
  void* fn;          // function pointer from dlsym()
  Str* name;         // symbol name for debugging/printing
  LibHandle* lib;    // owning library (for GC tracing)
};

// FFI type codes (internal enum for marshalling)
typedef enum {
  FFI_T_VOID,
  FFI_T_INT,
  FFI_T_DOUBLE,
  FFI_T_PTR,
  FFI_T_STR,
} FfiTypeCode;

// LibHandle API
LibHandle* mk_lib_handle(RlState* rls, void* handle, Str* path);
LibHandle* mk_lib_handle_s(RlState* rls, void* handle, Str* path);
void close_lib_handle(RlState* rls, LibHandle* lh);

// ForeignFn API
ForeignFn* mk_foreign_fn(RlState* rls, void* fn, Str* name, LibHandle* lib);
ForeignFn* mk_foreign_fn_s(RlState* rls, void* fn, Str* name, LibHandle* lib);

// FFI call API
Expr ffi_do_call(RlState* rls, ForeignFn* fn, FfiTypeCode ret_type,
                 int argc, FfiTypeCode* arg_types, Expr* args);

// Type conversion helpers
FfiTypeCode sym_to_ffi_type(RlState* rls, Sym* sym);
ffi_type* ffi_type_code_to_libffi(FfiTypeCode tc);

// Convenience macros
#define as_lib_handle(x)        ((LibHandle*)as_obj(x))
#define as_lib_handle_s(rls, x) ((LibHandle*)as_obj_s(rls, &LibHandleType, x))
#define as_foreign_fn(x)        ((ForeignFn*)as_obj(x))
#define as_foreign_fn_s(rls, x) ((ForeignFn*)as_obj_s(rls, &ForeignFnType, x))

#endif
