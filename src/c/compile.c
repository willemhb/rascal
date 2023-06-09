#include <stdarg.h>

#include "compile.h"
#include "opcodes.h"
#include "object.h"
#include "eval.h"
#include "runtime.h"

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// special form & syntax symbols ----------------------------------------------
value_t Quote, Def, Put, Do, If, Lmb;
value_t Ampersand;
symbol_t* Toplevel, * LmbSym;

// helpers --------------------------------------------------------------------
usize    compile_expression( chunk_t* chunk, value_t expr );
chunk_t* compile_chunk( symbol_t* name, chunk_t* context, list_t* formals, list_t* body, value_t singleton );

usize emit( chunk_t* chunk, opcode_t op, ... ) {
  int argc = opcode_argc(op);
  int size = 1 + argc;
  uint16 buffer[3] = { op, 0, 0 };

  switch ( argc ) {
    default:
      break;
    case 1: {
      va_list va;
      va_start(va, op);
      buffer[1] = va_arg(va, int);
      va_end(va);
      break;
    }
    case 2: {
      va_list va;
      va_start(va, op);
      buffer[1] = va_arg(va, int);
      buffer[2] = va_arg(va, int);
      va_end(va);
      break;
    }
  }

  binary_write(chunk->code, size, buffer);
  return chunk->code->cnt;
}

usize add_value( chunk_t* chunk, value_t x ) {
  return vector_push(chunk->vals, x);
}

usize compile_literal( chunk_t* chunk, value_t x ) {
  usize out;
  if ( x == NIL )
    out = emit(chunk, OP_LOAD_NIL);

  else if ( x == object(&EmptyList) )
    out = emit(chunk, OP_LOAD_LIST0);

  else
    out = emit(chunk, OP_LOAD_VALUE, add_value(chunk, x));

  return out;
}

long define_local( chunk_t* chunk, value_t name ) {
  list_t*  envt     = chunk->envt;
  table_t* locals   = as_table(envt->head);
  value_t  location = table_put(locals, name, number(locals->cnt));

  return   as_number(location);
}

long define_global( value_t name ) {
  return table_add(&Vm.global, name, UNDEFINED);
}

opcode_t define_name( chunk_t* chunk, value_t name, int* i, int* j ) {
  *i = *j = 0;
  opcode_t out;

  list_t* envt = chunk->envt;

  if ( envt == &EmptyList ) { // toplevel
    *i = define_global(name);
    emit(chunk, OP_DEF_GLOBAL, *i);
    out = OP_PUT_GLOBAL;
  } else {
    *j = define_local( chunk, name );
    emit(chunk, OP_DEF_LOCAL, *i, *j);
    out = OP_PUT_LOCAL;
  }

  return out;
}


opcode_t resolve_name( chunk_t* chunk, value_t name, bool assign, int* i, int* j ) {
  *i = 0, *j = -1;
  opcode_t out = OP_NOOP;

  list_t* envt = chunk->envt;

  while ( envt->arity ) {
    table_t* locals = as_table(envt->head);
    *j = table_locate(locals, name);

    if ( *j > -1 )
      break;

    (*i)++;
    envt = envt->tail;
  }

  if ( *j > -1 )
    out = assign ? OP_PUT_LOCAL : OP_LOAD_LOCAL;

  else if ( (*j=table_locate(&Vm.global, name)) > -1 )
    out = assign ? OP_PUT_GLOBAL : OP_LOAD_GLOBAL;

  else if ( as_symbol(name)->constant != UNDEFINED )
    out = OP_LOAD_VALUE;

  return out;
}

usize compile_variable( chunk_t* chunk, value_t name ) {
  int i, j;
  usize out = 0;

  opcode_t op = resolve_name(chunk, name, false, &i, &j);

  if ( op == OP_LOAD_GLOBAL )
    out = emit(chunk, op, j);

  else if ( op == OP_LOAD_LOCAL )
    out = emit(chunk, op, i, j);

  else if ( op == OP_LOAD_VALUE )
    out = compile_literal(chunk, as_symbol(name)->constant);

  else
    error("compile",
          name,
          "unbound symbol '%s'",
          as_symbol(name)->name);

  return out;
}

chunk_t* compile_chunk( symbol_t* name, chunk_t* context, list_t* formals, list_t* body, value_t singleton ) {
  chunk_t* out;

  if ( context == NULL ) { // toplevel
    out = chunk(name, &EmptyList, false);
  } else {
    bool variadic = false;
    table_t* locals = table(true, 0, NULL);
    list_t* fcopy = formals;

    for ( ;fcopy->arity; fcopy=fcopy->tail ) {
      require("compile", is_symbol(fcopy->head), object(formals), "syntax error: formals: type");

      if ( formals->head == Ampersand ) {
        require("compile", formals->arity == 2, object(formals), "syntax error: formals: arity");
        require("compile", variadic == false, object(formals), "syntax error: formals: multiple '&'");
        variadic = true;
      } else {
        require("compile", !table_has(locals, fcopy->head), object(formals), "syntax error: formals: duplicate");
        table_set(locals, fcopy->head, number(locals->cnt));
      }
    }

    out = chunk(name, cons(object(locals), context->envt), variadic);
  }

  if ( body != NULL ) {
    for ( ; body->arity; body=body->tail ) {
      compile_expression(out, body->head);

      if ( body->arity > 1 )
        emit(out, OP_POP);
    }
  } else
    compile_expression(out, singleton);

  emit(out, OP_RETURN);

  return out;
}

usize compile_combination( chunk_t* chunk, list_t* form ) {
  value_t head = form->head;
  list_t* args = form->tail;
  usize out = 0;
  int i, j;
  opcode_t op;
  uint16* instr;
  chunk_t* lmb;

  if ( head == Quote ) {
    require("compile", args->arity == 1, object(form), "syntax error: arity");
    out = compile_literal(chunk, args->head);

  } else if ( head == Def ) {
    require("compile", args->arity == 2, object(form), "syntax error: arity");
    require("compile", is_symbol(args->head), args->head, "syntax error: type");
    require("compile", !is_constant(args->head), args->head, "syntax error: value");
    op = define_name(chunk, args->head, &i, &j);
    compile_expression(chunk, args->tail->head);
    out = emit(chunk, op, i, j);

  } else if ( head == Put ) {
    require("compile", args->arity == 2, object(form), "syntax error: arity");
    require("compile", is_symbol(args->head), args->head, "syntax error: type");
    require("compile", !is_constant(args->head), args->head, "syntax error: constant name");
    op = resolve_name(chunk, args->head, true, &i, &j);
    compile_expression(chunk, args->tail->head);
    out = emit(chunk, op, i, j);
 
  } else if ( head == Do ) {
    require("compile", args->arity >= 1, object(form), "syntax error: arity");

    for ( ;args->arity; args=args->tail ) {
      out = compile_expression(chunk, args->head);

      if ( args->arity > 1 )
        emit(chunk, OP_POP);
    }

  } else if ( head == If ) {
    require("compile", args->arity >= 2 && args->arity < 4, object(form), "syntax error: arity");
    value_t test = args->head;
    value_t csqt = args->tail->head;
    value_t altn = args->arity == 2 ? NIL : args->tail->tail->head;

    compile_expression(chunk, test);
    i = emit(chunk, OP_JUMP_NIL, 0);
    compile_expression(chunk, csqt);
    j = emit(chunk, OP_JUMP, 0);
    out = compile_expression(chunk, altn);
    instr = chunk->code->data;
    instr[i-1] = out - i;
    instr[j-1] = out - j;

  } else if ( head == Lmb ) {
    require("compile", args->arity >= 2, object(form), "syntax error: arity");
    require("compile", is_list(args->head), args->head, "syntax-error: type");

    if ( args->arity == 2 )
      lmb = compile_chunk(LmbSym, chunk, as_list(args->head), NULL, args->tail->head);

    else
      lmb = compile_chunk(LmbSym, chunk, as_list(args->head), args->tail, NIL);

    compile_literal(chunk, object(lmb));
    out = emit(chunk, OP_CLOSURE);

  }  else {
    compile_expression(chunk, head);

    for ( i=0; args->arity; i++, args=args->tail )
      compile_expression(chunk, args->head);

    out = emit(chunk, OP_CALL, i);
  }

  return out;
}

usize compile_expression( chunk_t* chunk, value_t expr ) {
  if ( is_literal(expr) )
    return compile_literal(chunk, expr);

  else if ( is_symbol(expr) )
    return compile_variable(chunk, expr);

  else
    return compile_combination(chunk, as_list(expr));
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
chunk_t* compile( list_t* form ) {
  return compile_chunk(Toplevel, NULL, NULL, NULL, object(form) );
}

// misc -----------------------------------------------------------------------
void compile_init( void ) {
  Quote     = object(symbol("quote", true));
  Def       = object(symbol("def", true));
  Put       = object(symbol("put", true));
  Do        = object(symbol("do", true));
  If        = object(symbol("if", true));
  Lmb       = object(symbol("lmb", true));

  Ampersand = object(symbol("&", true));

  Toplevel  = symbol("<toplevel>", true);
  LmbSym    = as_symbol(Lmb);
}
