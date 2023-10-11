#include "opcodes.h"

// external API
size_t opCodeArgc(OpCode op) {
  switch (op) {
    case OP_VALUE:
    case OP_CALL:
    case OP_JUMP:
    case OP_JUMPF:
    case OP_JUMPT:
      return 1;

    default:
      return 0;
  }
}
