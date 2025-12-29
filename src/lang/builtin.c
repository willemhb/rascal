#include "lang/builtin.h"
#include "opcode.h"
#include "val.h"
#include "vm.h"
// Globals --------------------------------------------------------------------
Fun* ReplFun, *LoadFun;

// Implementations ------------------------------------------------------------
void define_builtins(void) {
  Fun* fun;

  // initialize builtin functions
  // arithmetic
  def_builtin_fun(&Main, "+", 2, false, OP_ADD);
  def_builtin_fun(&Main, "-", 2, false, OP_SUB);
  def_builtin_fun(&Main, "*", 2, false, OP_MUL);
  def_builtin_fun(&Main, "/", 2, false, OP_DIV);
  def_builtin_fun(&Main, "rem", 2, false, OP_REM);
  def_builtin_fun(&Main, "=", 2, false, OP_NEQ);
  def_builtin_fun(&Main, "<", 2, false, OP_NLT);
  def_builtin_fun(&Main, ">", 2, false, OP_NGT);

  // general
  def_builtin_fun(&Main, "=?", 2, false, OP_EGAL);
  def_builtin_fun(&Main, "hash", 1, false, OP_HASH);
  def_builtin_fun(&Main, "isa?", 2, false, OP_ISA);
  def_builtin_fun(&Main, "typeof", 1, false, OP_TYPE);

  // list
  def_builtin_fun(&Main, "list", 0, true, OP_LIST);
  fun = def_builtin_fun(&Main, "cons", 2, false, OP_CONS_2);
  add_builtin_method(&Main, fun, 2, true, OP_CONS_N);
  def_builtin_fun(&Main, "head", 1, false, OP_HEAD);
  def_builtin_fun(&Main, "tail", 1, false, OP_TAIL);
  def_builtin_fun(&Main, "list-ref", 2, false, OP_LIST_REF);
  def_builtin_fun(&Main, "list-len", 1, false, OP_LIST_LEN);

  // string
  def_builtin_fun(&Main, "str", 0, true, OP_STR);
  def_builtin_fun(&Main, "chars", 1, false, OP_CHARS);
  def_builtin_fun(&Main, "str-ref", 2, false, OP_STR_REF);
  def_builtin_fun(&Main, "str-len", 1, false, OP_STR_LEN);

  // symbol
  fun = def_builtin_fun(&Main, "gensym", 0, false, OP_GENSYM_0);
  add_builtin_method(&Main, fun, 1, false, OP_GENSYM_1);

  // tuple
  def_builtin_fun(&Main, "tuple", 0, true, OP_TUPLE);
  def_builtin_fun(&Main, "tuple-ref", 2, false, OP_TUPLE_REF);
  def_builtin_fun(&Main, "tuple-len", 1, false, OP_TUPLE_LEN);

  // map
  def_builtin_fun(&Main, "make-map", 0, true, OP_MAP);
  fun = def_builtin_fun(&Main, "get", 2, false, OP_MAP_GET);
  add_builtin_method(&Main, fun, 3, false, OP_MAP_GET);  // with default
  def_builtin_fun(&Main, "assoc", 1, true, OP_MAP_ASSOC);
  def_builtin_fun(&Main, "dissoc", 1, true, OP_MAP_DISSOC);
  def_builtin_fun(&Main, "keys", 1, false, OP_MAP_KEYS);
  def_builtin_fun(&Main, "vals", 1, false, OP_MAP_VALS);
  def_builtin_fun(&Main, "map-len", 1, false, OP_MAP_LEN);
  def_builtin_fun(&Main, "contains?", 2, false, OP_MAP_HAS);

  // interpreter
  def_builtin_fun(&Main, "read", 1, false, OP_READ);
  def_builtin_fun(&Main, "eval", 1, false, OP_EVAL);
  def_builtin_fun(&Main, "print", 2, false, OP_PRINT);
  ReplFun = def_builtin_fun(&Main, "repl", 0, false, OP_REPL);
  def_builtin_fun(&Main, "apply", 2, true, OP_APPLY);
  def_builtin_fun(&Main, "compile", 1, false, OP_COMPILE);
  def_builtin_fun(&Main, "exec", 1, false, OP_EXEC);
  LoadFun = def_builtin_fun(&Main, "load", 1, false, OP_LOAD);

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
