#include "opcode.h"


int op_arity(OpCode op) {
  int n;
  
  switch ( op ) {
    case OP_GET_VALUE:
    case OP_GET_GLOBAL:
    case OP_SET_GLOBAL:
    case OP_JUMP:
    case OP_JUMP_F:
    case OP_CALL:
      n = 1;
      break;

    default:
      n = 0;
      break;
  }

  return n;
}

char* op_name(OpCode op) {
  char* out;

  switch ( op ) {
    case OP_NOOP:       out = "NOOP";       break;
    case OP_POP:        out = "POP";        break;
    case OP_GET_VALUE:  out = "GET-VALUE";  break;
    case OP_GET_GLOBAL: out = "GET-GLOBAL"; break;
    case OP_SET_GLOBAL: out = "SET-GLOBAL"; break;
    case OP_ADD:        out = "ADD";        break;
    case OP_SUB:        out = "SUB";        break;
    case OP_MUL:        out = "MUL";        break;
    case OP_DIV:        out = "DIV";        break;
    case OP_EGAL:       out = "EGAL";       break;
    case OP_TYPE:       out = "TYPE";       break;
    case OP_JUMP:       out = "JUMP";       break;
    case OP_JUMP_F:     out = "JUMP-FALSE"; break;
    case OP_CALL:       out = "CALL";       break;
    case OP_RETURN:     out = "RETURN";     break;
    default:            out = "????";       break;
  }

  return out;
}
