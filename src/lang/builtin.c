#include "lang/builtin.h"
#include "lang/dispatch.h"
#include "lang/env.h"

#include "opcode.h"
#include "val.h"
#include "vm.h"
// Globals --------------------------------------------------------------------
Fun* ReplFun, *LoadFun;

// Implementations ------------------------------------------------------------
void define_builtins(void) {
  // initialize builtin functions
  // arithmetic
  def_builtin_fun(&Main, "+", OP_ADD, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, "-", OP_SUB, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, "*", OP_MUL, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, "/", OP_DIV, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, "rem", OP_REM, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, "=", OP_NEQ, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, "<", OP_NLT, false, 2, &NumType, &NumType);
  def_builtin_fun(&Main, ">", OP_NGT, false, 2, &NumType, &NumType);

  // general
  def_builtin_fun(&Main, "=?", OP_EGAL, false, 2, &AnyType, &AnyType);
  def_builtin_fun(&Main, "hash", OP_HASH, false, 1, &AnyType);
  def_builtin_fun(&Main, "isa?", OP_ISA, false, 2, &AnyType, &TypeType);
  def_builtin_fun(&Main, "typeof", OP_TYPE, false, 1, &AnyType);

  // list
  def_builtin_fun(&Main, "list", OP_LIST, true, 0);
  def_builtin_fun(&Main, "cons", OP_CONS_2, false, 2, &AnyType, &ListType);
  def_builtin_fun(&Main, "cons", OP_CONS_N, true, 2, &AnyType, &AnyType); // NB: problematic
  def_builtin_fun(&Main, "head", OP_HEAD, false, 1, &ListType);
  def_builtin_fun(&Main, "tail", OP_TAIL, false, 1, &ListType);
  def_builtin_fun(&Main, "nth", OP_LIST_REF, false, 2, &ListType, &NumType);
  def_builtin_fun(&Main, "len", OP_LIST_LEN, false, 1, &ListType);

  // string
  def_builtin_fun(&Main, "str", OP_STR, true, 0);
  def_builtin_fun(&Main, "chars", OP_CHARS, false, 1, &StrType);
  def_builtin_fun(&Main, "nth", OP_STR_REF, false, 2, &StrType, &NumType);
  def_builtin_fun(&Main, "len", OP_STR_LEN, false, 1, &StrType);

  // symbol
  def_builtin_fun(&Main, "gensym", OP_GENSYM_0, false, 0);
  def_builtin_fun(&Main, "gensym", OP_GENSYM_1, false, 1, &StrType);

  // tuple
  def_builtin_fun(&Main, "tuple", OP_TUPLE, true, 0);
  def_builtin_fun(&Main, "nth", OP_TUPLE_REF, false, 2, &TupleType, &NumType);
  def_builtin_fun(&Main, "len", OP_TUPLE_LEN, false, 1, &TupleType);

  // map
  def_builtin_fun(&Main, "map", OP_MAP, true, 0);
  def_builtin_fun(&Main, "get", OP_MAP_GET_2, false, 2, &MapType, &AnyType);
  def_builtin_fun(&Main, "get", OP_MAP_GET_3, false, 3, &MapType, &AnyType, &AnyType);
  def_builtin_fun(&Main, "assoc", OP_MAP_ASSOC, false, 3, &MapType, &AnyType, &AnyType);
  def_builtin_fun(&Main, "dissoc", OP_MAP_DISSOC, false, 2, &MapType, &AnyType);
  def_builtin_fun(&Main, "keys", OP_MAP_KEYS, false, 1, &MapType);
  def_builtin_fun(&Main, "vals", OP_MAP_VALS, false, 1, &MapType);
  def_builtin_fun(&Main, "len", OP_MAP_LEN, false, 1, &MapType);
  def_builtin_fun(&Main, "has?", OP_MAP_HAS, false, 2, &MapType, &AnyType);

  // interpreter
  def_builtin_fun(&Main, "read", OP_READ, false, 1, &PortType);
  def_builtin_fun(&Main, "eval", OP_EVAL, false, 1, &AnyType);
  def_builtin_fun(&Main, "print", OP_PRINT, false, 2, &PortType, &AnyType);
  ReplFun = def_builtin_fun(&Main, "repl", OP_REPL, false, 0);
  def_builtin_fun(&Main, "apply", OP_APPLY, true, 2, &FunType, &AnyType);
  def_builtin_fun(&Main, "compile", OP_COMPILE, false, 1, &AnyType);
  def_builtin_fun(&Main, "exec", OP_EXEC, false, 1, &FunType);
  LoadFun = def_builtin_fun(&Main, "load", OP_LOAD, false, 1, &StrType);

  // IO
  def_builtin_fun(&Main, "newline", OP_NEWLINE, false, 1, &PortType);

  // environment
  def_builtin_fun(&Main, "defined?", OP_DEFINED, false, 2, &SymType, &EnvType);
  def_builtin_fun(&Main, "local-env?", OP_LOCAL_ENV, false, 1, &EnvType);
  def_builtin_fun(&Main, "global-env?", OP_GLOBAL_ENV, false, 1, &EnvType);

  // system
  def_builtin_fun(&Main, "*heap-report*", OP_HEAP_REPORT, false, 0);
  def_builtin_fun(&Main, "*stack-report*", OP_STACK_REPORT, false, 0);
  def_builtin_fun(&Main, "*env-report*", OP_ENV_REPORT, false, 0);
  def_builtin_fun(&Main, "*stack-trace*", OP_STACK_TRACE, false, 0);
  def_builtin_fun(&Main, "*methods*", OP_METHODS, false, 1, &FunType);
  def_builtin_fun(&Main, "*dis*", 2, OP_DIS, false, 2, &FunType, &TupleType);

  // FFI
  def_builtin_fun(&Main, "ffi-open",  OP_FFI_OPEN, false, 1, &StrType);
  def_builtin_fun(&Main, "ffi-sym", OP_FFI_SYM, false, 2, &LibHandleType, &StrType);
  def_builtin_fun(&Main, "ffi-call", OP_FFI_CALL, true, 3, &ForeignFnType, &SymType, &ListType);
  def_builtin_fun(&Main, "ffi-close", OP_FFI_CLOSE, false, 1, &LibHandleType);
}
