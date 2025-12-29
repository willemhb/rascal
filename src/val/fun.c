#include "val/fun.h"
#include "val/port.h"
#include "val/primitive.h"
#include "vm.h"
#include "lang.h"
#include "util/util.h"
#include "util/collection.h"

// forward declarations
void print_method(Port* ios, Expr x);
void print_fun(Port* ios, Expr x);
void print_mtable(Port* ios, Expr x);

void trace_chunk(RlState* rls, void* ptr);
void trace_method(RlState* rls, void* ptr);
void trace_fun(RlState* rls, void* ptr);
void trace_mtable(RlState* rls, void* ptr);

void free_chunk(RlState* rls, void* ptr);
void free_mtable(RlState* rls, void* ptr);

// Type objects
Type ChunkType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_CHUNK,
  .obsize   = sizeof(Chunk),
  .trace_fn = trace_chunk,
  .free_fn  = free_chunk,
};

Type MethodType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_METHOD,
  .obsize   = sizeof(Method),
  .trace_fn = trace_method,
  .print_fn = print_method
};

Type MethodTableType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_MTABLE,
  .obsize   = sizeof(MethodTable),
  .trace_fn = trace_mtable,
  .print_fn = print_mtable,
  .free_fn  = free_mtable,
};

Type FunType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_FUN,
  .obsize   = sizeof(Fun),
  .trace_fn = trace_fun,
  .print_fn = print_fun
};

// chunk API
Chunk* mk_chunk(RlState* rls, Env* penv, Sym* name, Str* file) {
  Chunk* out = mk_obj_s(rls, &ChunkType, 0);

  // initialize internal arrays
  init_exprs(rls, &out->vals);
  init_code_buf(rls, &out->code);
  init_line_info(rls, &out->lines);

  // initialize other fields
  out->name  = name;
  out->file  = file;
  out->vars  = penv == NULL ? rls->vm->globals : mk_env(rls, penv);

  // remove chunk from the stack
  stack_pop(rls);

  return out;
}

Chunk* mk_chunk_s(RlState* rls, Env* penv, Sym* name, Str* file) {
  Chunk* out = mk_obj_s(rls, &ChunkType, 0);

  // initialize internal arrays
  init_exprs(rls, &out->vals);
  init_code_buf(rls, &out->code);
  init_line_info(rls, &out->lines);

  // initialize other fields
  out->name  = name;
  out->file  = file;
  out->vars  = penv == NULL ? rls->vm->globals : mk_env(rls, penv);

  return out;
}

int get_line_number(Chunk* c, instr_t* off) {
  assert(off >= chunk_code(c) && off < chunk_code(c)+chunk_codec(c));

  int diff = off - chunk_code(c);
  int* lines = chunk_lines(c);

  assert(lines != NULL);

  while ( diff > lines[1] ) {
    lines += 2;

    assert(lines < chunk_lines(c) + chunk_linesc(c));
  }

  return lines[0];
}

void add_line_number(RlState* rls, Chunk* c, int line) {
  add_to_line_info(rls, &c->lines, line, chunk_codec(c));
}

void finalize_chunk(RlState* rls, Chunk* c, int line) {
  if ( env_upvc(c->vars) > 0 )
    code_buf_push(rls, &c->code, OP_CAPTURE);
  
  code_buf_push(rls, &c->code, OP_RETURN);

  add_to_line_info(rls, &c->lines, line, chunk_codec(c));
}

static Ref* local_ref(Env* vars, int o) {
  assert(is_local_env(vars));

  Ref* r = NULL;

  for ( int i=0; i<vars->vars.maxc; i++ ) {
    r = vars->vars.kvs[i].val;

    if ( r == NULL || r->offset != o )
      continue;

    else
      break;
  }

  assert(r != NULL);

  return r;
}

void disassemble_chunk(Chunk* chunk) {
  instr_t* instr = chunk_code(chunk);
  Env* vars = chunk->vars;
  Expr* vals = chunk_vals(chunk);
  int offset = 0, max_offset = chunk_codec(chunk);
  int* lines = chunk_lines(chunk);

  printf("\n\n=== %s/%s ===\n\n",
         str_val(chunk->file), sym_val(chunk->name));

  printf("%-8s %-8s %-16s %-5s %-5s %-8s\n\n",
         "line", "offset", "instruction",
         "input", "input", "constant");

  while ( offset < max_offset ) {
    OpCode op  = instr[offset];
    int argc = op_arity(op);
    char* name = op_name(op);
    int line = lines[0];

    switch ( argc ) {

      case 1: {
        instr_t arg = instr[offset+1];
        printf("%.8d %.8d %-16s %.5d ----- ", line, offset, name, arg);

        if ( op == OP_GET_VALUE ) {
          print_exp(&Outs, vals[arg]);

        } else if ( op == OP_GET_GLOBAL || op == OP_SET_GLOBAL ) {
          print_exp(&Outs, vals[arg]);
        } else if ( op == OP_GET_LOCAL || op == OP_SET_LOCAL ) {
          print_exp(&Outs, tag_obj(local_ref(vars, arg)));
        } else if ( op == OP_GLYPH ) {
          char* name = char_name(arg);

          if ( name == NULL )
            printf("\\%c", arg);

          else
            printf("\\%s", name);
        } else if ( op == OP_SMALL ) {
          printf("%d", (short)arg);

        } else
          printf("--------");

        printf("\n");
        offset += 2; // advance past argument
        break;
      }

      case -2: { // variadic
        int arg = instr[offset+1];
        printf("%.8d %.8d %-16s %.5d ----- --------\n", line, offset, name, arg);
        offset++;

        for ( int i=0; i < arg; i++, offset += 2 ) {
          int x = instr[offset+1], y = instr[offset+2];
          printf("%.8d %.8d ---------------- %.5d %.5d\n", line, offset, x, y);
        }

        break;
      }

      default:
        printf("%.8d %.8d %-16s ----- ----- ", line, offset, name);

        if ( op == OP_TRUE )
          printf("%-8s", "true");

        else if ( op == OP_FALSE )
          printf("%-8s", "false");

        else if ( op == OP_NUL )
          printf("%-8s", "nul");

        else if ( op == OP_ZERO )
          printf("%-8s", "0");

        else if ( op == OP_ONE )
          printf("%-8s", "1");

        else
          printf("--------");

        printf("\n");

        offset++;
        break;
    }

    if ( offset > lines[1] )
      lines += 2;
  }
}

void trace_chunk(RlState* rls, void* ptr) {
  Chunk* chunk = ptr;

  mark_obj(rls, chunk->name);
  mark_obj(rls, chunk->file);
  mark_obj(rls, chunk->vars);
  trace_exprs(rls, &chunk->vals);
}

void free_chunk(RlState* rls, void* ptr) {
  Chunk* chunk = ptr;

  free_exprs(rls, &chunk->vals);
  free_code_buf(rls, &chunk->code);
  free_line_info(rls, &chunk->lines);
}

// function API
Fun* mk_fun(RlState* rls, Sym* name, bool macro, bool generic) {
  Fun* f = mk_obj(rls, &FunType, 0);
  f->name = name;
  f->macro = macro;
  f->generic = generic;
  f->method = NULL;
  f->mcount = 0;

  return f;
}

Fun* mk_fun_s(RlState* rls, Sym* name, bool macro, bool generic) {
  Fun* out = mk_fun(rls, name, macro, generic);
  stack_push(rls, tag_obj(out));
  return out;
}

void fun_add_method(RlState* rls, Fun* fun, Method* m) {
  assert(fun->generic || fun->method == NULL);

  int count = fun->mcount;

  if ( count == 0 )
    fun->method = m;

  else {
    if ( count == 1 ) {
      MethodTable* mt = mk_mtable_s(rls, fun);
      mtable_add(rls, mt, fun->method);
      fun->methods = mt;
      stack_pop(rls);
    }

    mtable_add(rls, fun->methods, m);
  }

  fun->mcount++;
}

void fun_add_method_s(RlState* rls, Fun* fun, Method* m) {
  stack_preserve(rls, 2, tag_obj(fun), tag_obj(m));
  fun_add_method(rls, fun, m);
  stack_popn(rls, 2);
}

Method* fun_get_method(Fun* fun, int argc) {
  assert(fun->method != NULL);

  Method* out = NULL;

  if ( is_singleton_fn(fun) ) {
    if ( argc_match(fun->method, argc) )
      out = fun->method;
  } else {
    out = mtable_lookup(fun->methods, argc);
  }

  return out;
}

Fun* def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op) {
  StackRef top = rls->s_top;
  Sym* n = mk_sym_s(rls, name);
#ifdef RASCAL_DEBUG
  // pprintf(&Outs, "Defining builtin function %s.\n\n", sym_val(n));
#endif
  Fun* f = mk_fun_s(rls, n, false, true);
  Method* m = mk_builtin_method_s(rls, f, arity, va, op);

  fun_add_method(rls, f, m);
  toplevel_env_def(rls, Vm.globals, n, tag_obj(f), false, true);
  rls->s_top = top;

  return f;
}

void add_builtin_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op) {
  // NB: fun should already be defined at toplevel
  StackRef top = rls->s_top;
  Method* m = mk_builtin_method_s(rls, fun, arity, va, op);
  fun_add_method(rls, fun, m);
  rls->s_top = top;
}

void print_fun(Port* ios, Expr x) {
  Fun* fun = as_fun(x);

  if ( fun->macro )
    pprintf(ios, "<macro:%s/%d>", fn_name(fun), fun->mcount);

  else
    pprintf(ios, "<fun:%s/%d>", fn_name(fun), fun->mcount);
}

void trace_fun(RlState* rls, void* ptr) {
  Fun* fun = ptr;

  mark_obj(rls, fun->name);
  mark_obj(rls, fun->method);
}

// method API
Method* mk_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op, Chunk* code) {
  Method* m = mk_obj(rls, &MethodType, 0);
  m->fun = fun;
  m->arity = arity;
  m->va = va;
  m->label = op;
  m->chunk = code;

  assert(code != NULL || op != OP_NOOP);

  
  
  return m;
}

Method* mk_method_s(RlState* rls, Fun* fun, int arity, bool va, OpCode op, Chunk* code) {
  Method* out = mk_method(rls, fun, arity, va, op, code);
  stack_push(rls, tag_obj(out));
  return out;
}

Method* mk_closure(RlState* rls, Method* proto) {
  Method* cls; int count = method_upvc(proto);

  if ( count == 0 )
    cls = proto;

  else {
    cls = clone_obj(rls, proto);
    cls->upvs = allocate(NULL, count*sizeof(UpVal*));
  }

  return cls;
}

Method* mk_builtin_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op) {
  return mk_method(rls, fun, arity, va, op, NULL);
}

Method* mk_builtin_method_s(RlState* rls, Fun* fun, int arity, bool va, OpCode op) {
  Method* out = mk_builtin_method(rls, fun, arity, va, op);
  stack_push(rls, tag_obj(out));
  return out;
}

Method* mk_user_method(RlState* rls, Fun* fun, int arity, bool va, Chunk* code) {
  StackRef top = rls->s_top;
  Method* m = mk_method(rls, fun, arity, va, OP_NOOP, code);
  rls->s_top = top;

  return m;
}

Method* mk_user_method_s(RlState* rls, Fun* fun, int arity, bool va, Chunk* code) {
  Method* out = mk_user_method(rls, fun, arity, va, code);
  stack_push(rls, tag_obj(out));
  return out;
}

void disassemble_method(Method* m) {
  disassemble_chunk(m->chunk);
}

Expr upval_ref(Method* m, int i) {
  assert(i >= 0 && i < method_upvc(m));
  UpVal* upv = m->upvs[i];

  return *deref(upv);
}

void upval_set(Method* m, int i, Expr x) {
  assert(i >= 0 && i < method_upvc(m));
  UpVal* upv = m->upvs[i];

  *deref(upv) = x;
}

void print_method(Port* ios, Expr x) {
  Method* m = as_method(x);

  pprintf(ios, "<method:%s/%d%s>",
          method_name(m),
          m->arity,
          m->va ? "+" : "");
}

void trace_method(RlState* rls, void* ptr) {
  Method* m = ptr;

  mark_obj(rls, m->fun);
  mark_obj(rls, m->chunk);

  for ( int i=0; i < method_upvc(m); i++ )
    mark_obj(rls, m->upvs[i]);
}

// method table API
MethodTable* mk_mtable(RlState* rls, Fun* fun) {
  MethodTable* mt = mk_obj(rls, &MethodTableType, 0);
  mt->fun = fun;
  mt->variadic = NULL;
  init_bit_vec(rls, &mt->methods);
  return mt;
}

MethodTable* mk_mtable_s(RlState* rls, Fun* fun) {
  MethodTable* out = mk_mtable(rls, fun);
  stack_push(rls, tag_obj(out));
  return out;
}

void mtable_add(RlState* rls, MethodTable* mt, Method* m) {
  char* fname = mtable_name(mt);
  // check for duplicate signature
  if ( method_va(m) ) {
    if ( mt->variadic != NULL )
      eval_error(rls, "method %s/%d+ already exists", fname, mt->variadic->arity);

    mt->variadic = m;
  }

  else if ( bit_vec_has(&mt->methods, m->arity) )
    eval_error(rls, "method %s/%d already exists", fname, m->arity);

  else {
    bit_vec_set(rls, &mt->methods, m->arity, m); // add to bitmap
  }
}

Method* mtable_lookup(MethodTable* mt, int argc) {
  Method* out = bit_vec_get(&mt->methods, argc);

  if ( out == NULL && mt->variadic && argc >= mt->variadic->arity )
    out = mt->variadic;

  return out;
}

void print_mtable(Port* ios, Expr x) {
  MethodTable* mt = as_mtable(x);
  pprintf(ios, "<method-table:%s/%d>", mtable_name(mt), mtable_count(mt));
}

void trace_mtable(RlState* rls, void* ptr) {
  MethodTable* mt = ptr;
  mark_obj(rls, mt->fun);
  mark_obj(rls, mt->variadic);
  trace_obj_array(rls, (Obj**)mt->methods.data, mt->methods.count);
}

void free_mtable(RlState* rls, void* ptr) {
  MethodTable* mt = ptr;
  free_bit_vec(rls, &mt->methods);
}

