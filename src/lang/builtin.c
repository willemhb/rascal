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
  def_builtin_fun(&Main, "map-len", OP_MAP_LEN, false, 1, &MapType);
  def_builtin_fun(&Main, "has?", OP_MAP_HAS, false, 2, &MapType, &AnyType);

  // interpreter
  def_builtin_fun(&Main, "read", OP_READ, false, 1, &PortType);
  def_builtin_fun(&Main, "eval", OP_EVAL, false, 1, &AnyType);
  def_builtin_fun(&Main, "print", OP_PRINT, false, 2, &PortType, &AnyType);
  ReplFun = def_builtin_fun(&Main, "repl", OP_REPL, false, 0);
  def_builtin_fun(&Main, "apply", OP_APPLY, true, 1, &FunType);
  def_builtin_fun(&Main, "compile", 1, false, OP_COMPILE);
  def_builtin_fun(&Main, "exec", 1, false, OP_EXEC);
  LoadFun = def_builtin_fun(&Main, "load", OP_LOAD, false, 1, &StrType);

  // IO
  def_builtin_fun(&Main, "newline", 1, false, OP_NEWLINE);

  // environment
  def_builtin_fun(&Main, "defined?", 2, false, OP_DEFINED);
  def_builtin_fun(&Main, "local-env?", 1, false, OP_LOCAL_ENV);
  def_builtin_fun(&Main, "global-env?", 1, false, OP_GLOBAL_ENV);

  // system
  def_builtin_fun(&Main, "*heap-report*", 0, false, OP_HEAP_REPORT);
  def_builtin_fun(&Main, "*stack-report*", 0, false, OP_STACK_REPORT);
  def_builtin_fun(&Main, "*env-report*", 0, false, OP_ENV_REPORT);
  def_builtin_fun(&Main, "*stack-trace*", 0, false, OP_STACK_TRACE);
  def_builtin_fun(&Main, "*methods*", 1, false, OP_METHODS);
  def_builtin_fun(&Main, "*dis*", 2, false, OP_DIS);

  // FFI
  def_builtin_fun(&Main, "ffi-open", 1, false, OP_FFI_OPEN);
  def_builtin_fun(&Main, "ffi-sym", 2, false, OP_FFI_SYM);
  def_builtin_fun(&Main, "ffi-call", 3, true, OP_FFI_CALL);
  def_builtin_fun(&Main, "ffi-close", 1, false, OP_FFI_CLOSE);
}
