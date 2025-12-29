#include "opcode.h"


int op_arity(OpCode op) {
  int n;

  switch ( op ) {
    // special case of variable length instruction
    case OP_CAPTURE: 
      n = -2;
      break;

    case OP_DUMMY_REF:
    case OP_GET_VALUE:
    case OP_GET_GLOBAL:
    case OP_SET_GLOBAL:
    case OP_GET_LOCAL:
    case OP_SET_LOCAL:
    case OP_GET_UPVAL:
    case OP_SET_UPVAL:
    case OP_GLYPH:
    case OP_SMALL:
    case OP_JUMP:
    case OP_JUMP_F:
    case OP_PJUMP_F:
    case OP_PJUMP_T:
    case OP_CALL:
    case OP_MAP:
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
    case OP_NOOP:         out = "NOOP";         break;
    case OP_DUMMY_REF:    out = "OP-DUMMY-REF"; break;
    case OP_POP:          out = "POP";          break;
    case OP_RPOP:         out = "RPOP";         break;
    case OP_TRUE:         out = "TRUE";         break;
    case OP_FALSE:        out = "FALSE";        break;
    case OP_NUL:          out = "NUL";          break;
    case OP_EOS:          out = "EOS";          break;
    case OP_ZERO:         out = "ZERO";         break;
    case OP_ONE:          out = "ONE";          break;
    case OP_GLYPH:        out = "GLYPH";        break;
    case OP_SMALL:        out = "SMALL";        break;
    case OP_GET_VALUE:    out = "GET-VALUE";    break;
    case OP_GET_GLOBAL:   out = "GET-GLOBAL";   break;
    case OP_SET_GLOBAL:   out = "SET-GLOBAL";   break;
    case OP_ADD_METHOD:   out = "ADD-METHOD";   break;
    case OP_GET_LOCAL:    out = "GET-LOCAL";    break;
    case OP_SET_LOCAL:    out = "SET-LOCAL";    break;
    case OP_GET_UPVAL:    out = "GET-UPVAL";    break;
    case OP_JUMP:         out = "JUMP";         break;
    case OP_JUMP_F:       out = "JUMP-FALSE";   break;
    case OP_PJUMP_F:      out = "PJUMP-FALSE";  break;
    case OP_PJUMP_T:      out = "PJUMP-TRUE";   break;
    case OP_CLOSURE:      out = "CLOSURE";      break;
    case OP_CAPTURE:      out = "CAPTURE";      break;
    case OP_CALL:         out = "CALL";         break;
    case OP_RETURN:       out = "RETURN";       break;
    case OP_CATCH:        out = "CATCH";        break;
    case OP_RAISE:        out = "RAISE";        break;
    case OP_ECATCH:       out = "END-CATCH";    break;
    case OP_ADD:          out = "ADD";          break;
    case OP_SUB:          out = "SUB";          break;
    case OP_MUL:          out = "MUL";          break;
    case OP_DIV:          out = "DIV";          break;
    case OP_REM:          out = "REM";          break;
    case OP_NEQ:          out = "NEQ";          break;
    case OP_NLT:          out = "NLT";          break;
    case OP_NGT:          out = "NGT";          break;
    case OP_EGAL:         out = "EGAL";         break;
    case OP_HASH:         out = "HASH";         break;
    case OP_ISA:          out = "ISA?";         break;
    case OP_TYPE:         out = "TYPE";         break;
    case OP_LIST:         out = "LIST";         break;
    case OP_CONS_2:       out = "CONS-2";       break;
    case OP_CONS_N:       out = "CONS-N";       break;
    case OP_HEAD:         out = "HEAD";         break;
    case OP_TAIL:         out = "TAIL";         break;
    case OP_LIST_REF:     out = "LIST-REF";     break;
    case OP_LIST_LEN:     out = "LIST-LEN";     break;
    case OP_GENSYM_0:     out = "GENSYM-0";     break;
    case OP_GENSYM_1:     out = "GENSYM-1";     break;
    case OP_STR:          out = "STR";          break;
    case OP_CHARS:        out = "CHARS";        break;
    case OP_STR_REF:      out = "STR-REF";      break;
    case OP_STR_LEN:      out = "STR-LEN";      break;
    case OP_TUPLE:        out = "TUPLE";        break;
    case OP_TUPLE_REF:    out = "TUPLE-REF";    break;
    case OP_TUPLE_LEN:    out = "TUPLE-LEN";    break;
    case OP_MAP:          out = "MAP";          break;
    case OP_MAP_GET:      out = "MAP-GET";      break;
    case OP_MAP_ASSOC:    out = "MAP-ASSOC";    break;
    case OP_MAP_DISSOC:   out = "MAP-DISSOC";   break;
    case OP_MAP_KEYS:     out = "MAP-KEYS";     break;
    case OP_MAP_VALS:     out = "MAP-VALS";     break;
    case OP_MAP_LEN:      out = "MAP-LEN";      break;
    case OP_MAP_HAS:      out = "MAP-HAS";      break;
    case OP_READ:         out = "READ";         break;
    case OP_EVAL:         out = "EVAL";         break;
    case OP_PRINT:        out = "PRINT";        break;
    case OP_REPL:         out = "REPL";         break;
    case OP_APPLY:        out = "APPLY";        break;
    case OP_COMPILE:      out = "COMPILE";      break;
    case OP_EXEC:         out = "EXEC";         break;
    case OP_LOAD:         out = "LOAD";         break;
    case OP_NEWLINE:      out = "NEWLINE";      break;
    case OP_DEFINED:      out = "DEFINED";      break;
    case OP_LOCAL_ENV:    out = "LOCAL-ENV";    break;
    case OP_GLOBAL_ENV:   out = "GLOBAL-ENV";   break;
    case OP_HEAP_REPORT:  out = "HEAP-REPORT";  break;
    case OP_STACK_REPORT: out = "STACK-REPORT"; break;
    case OP_ENV_REPORT:   out = "ENV-REPORT";   break;
    case OP_STACK_TRACE:  out = "STACK-TRACE";  break;
    case OP_METHODS:      out = "METHODS";      break;
    case OP_DIS:          out = "DIS";          break;
    case OP_FFI_OPEN:     out = "FFI-OPEN";     break;
    case OP_FFI_SYM:      out = "FFI-SYM";      break;
    case OP_FFI_CALL:     out = "FFI-CALL";     break;
    case OP_FFI_CLOSE:    out = "FFI-CLOSE";    break;
    default:              out = "????";         break;
  }

  return out;
}
