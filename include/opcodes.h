#ifndef rascal_opcodes_h
#define rascal_opcodes_h

enum
  {
    // loads and stores
    // immediate loads
    OP_LOAD_SI8,    OP_LOAD_UI8,   OP_LOAD_ASCII, OP_LOAD_LATIN1, OP_LOAD_UTF8,

    OP_LOAD_SI16,   OP_LOAD_UI16,  OP_LOAD_UTF16, OP_LOAD_OPCODE, OP_LOAD_PRIMITIVE,

    OP_LOAD_SI32,   OP_LOAD_UI32,  OP_LOAD_R32,   OP_LOAD_CTYPE,

    // special constant loads
    OP_LOAD_TRUE,   OP_LOAD_FALSE, OP_LOAD_NUL,

    // normal loads and stores
    OP_LOAD_CONSTANT,

    OP_LOAD_LOCAL,    OP_STORE_LOCAL,

    OP_LOAD_UPVAL,    OP_STORE_UPVAL,

    OP_LOAD_MODULE,   OP_STORE_MODULE,

    OP_LOAD_TOPLEVEL, OP_STORE_TOPLEVEL,

    // stack manipulation
    OP_PUSH,   OP_POP,   OP_DUP,   OP_ROT,

    OP_PUSH_N, OP_POP_N, OP_DUP_N, OP_ROT_N,

    // control flow
    // jumps
    OP_JUMP, OP_JUMP_TRUE, OP_JUMP_FALSE,

    // continuations & effects

    // function calls
    // call instructions are invoked when the method can't be determined at compile time
    OP_CALL_0, OP_CALL_1, OP_CALL_2, OP_CALL_3, OP_CALL_N,

    OP_INVOKE_NATIVE_0,  OP_INVOKE_NATIVE_1,  OP_INVOKE_NATIVE_2,
    OP_INVOKE_NATIVE_3,  OP_INVOKE_NATIVE_N,

    OP_INVOKE_CLOSURE_0, OP_INVOKE_CLOSURE_1, OP_INVOKE_CLOSURE_2,
    OP_INVOKE_CLOSURE_3, OP_INVOKE_CLOSURE_N,

    OP_INVOKE_CONTROL_0, OP_INVOKE_CONTROL_1, OP_INVOKE_CONTROL_2,
    OP_INVOKE_CONTROL_3, OP_INVOKE_CONTROL_N,

    // primitives (correspond to builtin functions)
    // predicates & comparison
    OP_IDP, OP_ISAP, OP_HASP, OP_SIZEOF, OP_TYPEOF,

    // low-level arithmetic (user arithmetic functions are built on top of these)
    OP_ADD_SI8, OP_ADD_UI8,  OP_ADD_SI16, OP_ADD_UI16, OP_ADD_SI32, OP_ADD_UI32,
    OP_ADD_FIX, OP_ADD_SI64, OP_ADD_UI64, OP_ADD_R32,  OP_ADD_REAL,

    OP_SUB_SI8, OP_SUB_UI8,  OP_SUB_SI16, OP_SUB_UI16, OP_SUB_SI32, OP_SUB_UI32,
    OP_SUB_FIX, OP_SUB_SI64, OP_SUB_UI64, OP_SUB_R32,  OP_SUB_REAL,

    OP_MUL_SI8, OP_MUL_UI8,  OP_MUL_SI16, OP_MUL_UI16, OP_MUL_SI32, OP_MUL_UI32,
    OP_MUL_FIX, OP_MUL_SI64, OP_MUL_UI64, OP_MUL_R32,  OP_MUL_REAL,

    OP_DIV_SI8, OP_DIV_UI8,  OP_DIV_SI16, OP_DIV_UI16, OP_DIV_SI32, OP_DIV_UI32,
    OP_DIV_FIX, OP_DIV_SI64, OP_DIV_UI64, OP_DIV_R32,  OP_DIV_REAL,

    // promote instructions ()
  };

#endif
