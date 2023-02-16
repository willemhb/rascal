#ifndef opcode_h
#define opcode_h

typedef enum OpCode OpCode;

enum OpCode {
  OP_START,
  OP_HALT,
  OP_LOADC,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_LIST0,
  OP_POP,
  OP_LOADL,
  OP_STOREL,
  OP_LOADG,
  OP_STOREG,
  OP_DEFG,
  OP_LOADN,
  OP_STOREN,
  OP_JUMP,
  OP_JUMPF,
  OP_INVOKE,
  OP_CLOSURE,
  OP_RETURN
};

#endif
