#include "vm.h"
#include "runtime.h"

static void    compile_expression( value_t *code, value_t x );
static value_t emit_instruction( value_t *code, opcode_t op, short arg );
static size_t  add_constant( value_t *code, value_t x );

value_t compile( value_t x ) {
  index_t form_buffer = push_s( "compile", x );
  
  code_s( 32 );      // instructions
  vector_s( 8, NULL );       // 
  
  index_t code_buffer = vector_s( 2, Stack+Sp-2);  // bytecode object

  for_cons( &Stack[form_buffer], x)
    compile_expression( &Stack[code_buffer], x );

  if ( !nilp( Stack[form_buffer] ) )
    compile_expression( &Stack[code_buffer], Stack[form_buffer] );

  emit_instruction( &Stack[code_buffer], op_return, 0 );

  return popn( 2 );
}

static value_t emit_instruction( value_t *code, opcode_t op, short arg ) {
  size_t instr_len = 2;

  if ( op > op_return )
    instr_len += 2;

  value_t instructions = vdata(*code)[1];
  size_t total = instr_len + alen(instructions);

  if (total >= asize(instructions) )
    instructions = reallocate( instructions, total );

  *(short*)snext(instructions) = op;

  if (instr_len == 4)
    *(((short*)snext(instructions))+1) = arg;

  alen(instructions) += instr_len;
  
  return instructions;
}

static size_t add_constant( value_t *code, value_t x ) {
  value_t  constants = vdata(*code)[0];
  value_t *values    = adata(constants);
  size_t   length    = alen(x);
  size_t   location  = 0;

  for (; location<length; location++) {
    if (values[location] == x)
      break;
  }

  if (location == length) {
    push_s( "compile", x );
    constants = reallocate( constants, location );
    vdata(constants)[location] = pop();
  }

  return location;
}

static void compile_expression( value_t *code, value_t x ) {
  if ( fixnump( x ) || keywordp( x ) ) {
    size_t location = add_constant( code, x );
    emit_instruction( code, op_loadc, location );
  }

  else if ( symbolp( x ) ) {
    size_t location = add_constant( code, x );

    emit_instruction( code, op_loadg, location );

  } else {
    push_s( "compile", x );
    short n = -1;

    for_cons( &Stack[Sp-1], x ) {
      compile_expression( code, x );
      n++;
    }

    emit_instruction( code, op_call, n );
  }
}

void r_aligned builtin_cons( size_t n ) {
  
}

value_t execute( value_t code ) {
  static void *labels[num_opcodes] = {
    [op_return] = &&do_return,

    [op_loadc]  = &&do_loadc, [op_loadg] = &&do_loadg,

    [op_call]   = &&do_call,
  };

  opcode_t op;
  short arg = -1;

  
 do_fetch:
  op = ;

 do_return:
  
}
