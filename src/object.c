#include <assert.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "hash.h"
#include "compare.h"

/* globals */
Sym* SymbolTable = NULL;

/* API */
// common utilities -----------------------------------------------------------
ObjType obj_type(Val val) {
  return as_obj(val)->type;
}

usize obj_size(Obj* obj) {
  static usize ObjSize[] = {
    [SYM_OBJ] = sizeof(Sym), [PAIR_OBJ] = sizeof(Pair), [LIST_OBJ] = sizeof(List),
    [VEC_OBJ] = sizeof(Vec), [BIN_OBJ] = sizeof(Bin), [STR_OBJ] = sizeof(Str),
    [TABLE_OBJ] = sizeof(Table), [PORT_OBJ] = sizeof(Port), [USER_FN_OBJ] = sizeof(UserFn),
    [NATIVE_FN_OBJ] = sizeof(NativeFn), [READER_FN_OBJ] = sizeof(ReaderFn)
  };

  return ObjSize[obj->type];
}

char* obj_type_name(Obj* obj) {
  static char* ObjTypeName[] = {
    [SYM_OBJ] = "sym", [PAIR_OBJ] = "pair", [LIST_OBJ] = "list",
    [VEC_OBJ] = "vec", [BIN_OBJ] = "bin", [STR_OBJ] = "str",
    [TABLE_OBJ] = "table", [PORT_OBJ] = "port", [USER_FN_OBJ] = "user-fn",
    [NATIVE_FN_OBJ] = "native-fn", [READER_FN_OBJ] = "reader-fn"
  };

  return ObjTypeName[obj->type];
}

bool is_obj_type(Val value, ObjType type) {
  return is_obj(value) && obj_type(value) == type;
}
