#include "opcode.h"


int op_arity(OpCode op) {
  int n;

  switch ( op ) {
    // special case of variable length instruction
    case OP_CAPTURE: 
      n = -2;
      break;

    case OP_GET_VALUE:
    case OP_GET_GLOBAL:
    case OP_SET_GLOBAL:
    case OP_GET_LOCAL:
    case OP_SET_LOCAL:
    case OP_GET_UPVAL:
    case OP_SET_UPVAL:
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
    case OP_NOOP:        out = "NOOP";        break;
    case OP_POP:         out = "POP";         break;
    case OP_RPOP:        out = "RPOP";        break;
    case OP_TRUE:        out = "TRUE";        break;
    case OP_FALSE:       out = "FALSE";       break;
    case OP_NUL:         out = "NUL";         break;
    case OP_GET_VALUE:   out = "GET-VALUE";   break;
    case OP_GET_GLOBAL:  out = "GET-GLOBAL";  break;
    case OP_SET_GLOBAL:  out = "SET-GLOBAL";  break;
    case OP_GET_LOCAL:   out = "GET-LOCAL";   break;
    case OP_SET_LOCAL:   out = "SET-LOCAL";   break;
    case OP_GET_UPVAL:   out = "GET-UPVAL";   break;
    case OP_JUMP:        out = "JUMP";        break;
    case OP_JUMP_F:      out = "JUMP-FALSE";  break;
    case OP_CLOSURE:     out = "CLOSURE";     break;
    case OP_CAPTURE:     out = "CAPTURE";     break;
    case OP_CALL:        out = "CALL";        break;
    case OP_RETURN:      out = "RETURN";      break;
    case OP_ADD:         out = "ADD";         break;
    case OP_SUB:         out = "SUB";         break;
    case OP_MUL:         out = "MUL";         break;
    case OP_DIV:         out = "DIV";         break;
    case OP_EGAL:        out = "EGAL";        break;
    case OP_TYPE:        out = "TYPE";        break;
    case OP_LIST:        out = "LIST";        break;
    case OP_CONS:        out = "CONS";        break;
    case OP_HEAD:        out = "HEAD";        break;
    case OP_TAIL:        out = "TAIL";        break;
    case OP_LIST_REF:    out = "LIST-REF";    break;
    case OP_LIST_LEN:    out = "LIST-LEN";    break;
    case OP_STR_REF:     out = "STR-REF";     break;
    case OP_STR_LEN:     out = "STR-LEN";     break;
    case OP_HEAP_REPORT: out = "HEAP-REPORT"; break;
    case OP_STACK_REPORT:out = "STACK-REPORT";break;
    case OP_ENV_REPORT:  out = "ENV-REPORT";  break;
    case OP_DIS:         out = "DIS";         break;
    case OP_LOAD:        out = "LOAD";        break;
    default:             out = "????";        break;
  }

  return out;
}
