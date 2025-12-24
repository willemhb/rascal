#include "val/fun.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"
#include "util/util.h"

// forward declarations
void print_method(Port* ios, Expr x);
void print_fun(Port* ios, Expr x);
void print_mtable(Port* ios, Expr x);
void clone_method(RlState* rls, void* ptr);
void trace_chunk(RlState* rls, void* ptr);
void trace_method(RlState* rls, void* ptr);
void trace_fun(RlState* rls, void* ptr);
void trace_mtable(RlState* rls, void* ptr);

// Type objects
Type ChunkType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_CHUNK,
  .obsize   = sizeof(Chunk),
  .trace_fn = trace_chunk
};

Type MethodType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_METHOD,
  .obsize   = sizeof(Method),
  .clone_fn = clone_method,
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
  .print_fn = print_mtable
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
Chunk* mk_chunk(RlState* rls, Env* vars, Alist* vals, Buf16* code) {
  Chunk* out = mk_obj(rls, &ChunkType, 0);

  out->vars = vars;
  out->vals = vals;
  out->code = code;

  return out;
}

Chunk* mk_chunk_s(RlState* rls, Env* vars, Alist* vals, Buf16* code) {
  Chunk* out = mk_chunk(rls, vars, vals, code);
  push(rls, tag_obj(out));

  return out;
}

static Ref* local_ref(Env* vars, int o) {
  assert(is_local_env(vars));

  Ref* r = NULL;

  for ( int i=0; i<vars->vars.max_count; i++ ) {
    r = vars->vars.kvs[i].val;

    if ( r == NULL || r->offset != o )
      continue;

    else
      break;
  }

  assert(r != NULL);

  return r;
}

void do_disassemble(Env* vars, Alist* vals, Buf16* code) {
  instr_t* instr = code->binary.vals;
  int offset = 0, max_offset = code->binary.count;

  printf("%-8s %-16s %-5s %-5s %-8s\n\n", "line", "instruction", "input", "input", "constant");

  while ( offset < max_offset ) {
    OpCode op  = instr[offset];
    int argc   = op_arity(op);
    char* name = op_name(op);

    switch ( argc ) {

      case 1: {
        instr_t arg = instr[offset+1];
        printf("%.8d %-16s %.5d ----- ", offset, name, arg);

        if ( op == OP_GET_VALUE ) {
          print_exp(&Outs, vals->exprs.vals[arg]);

        } else if ( op == OP_GET_GLOBAL || op == OP_SET_GLOBAL ) {
          print_exp(&Outs, tag_obj(Vm.globals->vals.vals[arg]));
        } else if ( op == OP_GET_LOCAL || op == OP_SET_LOCAL ) {
          print_exp(&Outs, tag_obj(local_ref(vars, arg)));
        } else if ( op == OP_GLYPH ) {
          if ( arg < CHAR_MAX && CharNames[arg])
            printf("\\%s", CharNames[arg]);

          else
            printf("\\%c", arg);
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
        printf("%.8d %-16s %.5d ----- --------\n", offset, name, arg);
        offset++;

        for ( int i=0; i < arg; i++, offset += 2 ) {
          int x = instr[offset+1], y = instr[offset+2];
          printf("%.8d ---------------- %.5d %.5d\n", offset, x, y);
        }

        break;
      }

      default:
        printf("%.8d %-16s ----- ----- ", offset, name);

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
  }
}

void disassemble_chunk(Chunk* chunk) {
  do_disassemble(chunk->vars, chunk->vals, chunk->code);
}

void trace_chunk(RlState* rls, void* ptr) {
  Chunk* chunk = ptr;

  mark_obj(rls, chunk->vars);
  mark_obj(rls, chunk->vals);
  mark_obj(rls, chunk->code);
}

// function API
Fun* mk_fun(RlState* rls, Sym* name, bool macro, bool generic) {
  Fun* f = mk_obj(rls, &FunType, 0);
  f->name = name;
  f->macro = macro;
  f->generic = generic;
  f->method = NULL;

  return f;
}

Fun* mk_fun_s(RlState* rls, Sym* name, bool macro, bool generic) {
  Fun* out = mk_fun(rls, name, macro, generic);
  push(rls, tag_obj(out));
  return out;
}

void fun_add_method(RlState* rls, Fun* fun, Method* m) {
  assert(fun->generic || fun->method == NULL);

  if ( fn_method_count(fun) == 0 )
    fun->method = m;

  else {
    fun->methods = mk_mtable(rls, fun);
    mtable_add(rls, fun->methods, m);
  }
}

void fun_add_method_s(RlState* rls, Fun* fun, Method* m) {
  preserve(rls, 2, tag_obj(fun), tag_obj(m));
  fun_add_method(rls, fun, m);
  popn(rls, 2);
}

Method* fun_get_method(Fun* fun, int argc) {
  assert(fun->method != NULL);

  Method* out = NULL;

  if ( is_singleton_fn(fun) && argc_match(fun->method, argc) )
      out = fun->method;
  else
    out = mtable_lookup(fun->methods, argc);

  return out;
}

Fun* def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op) {
  int sp = save_sp(rls);
  Sym* n = mk_sym_s(rls, name);
  Fun* f = mk_fun_s(rls, n, false, true);
  Method* m = mk_builtin_method_s(rls, f, arity, va, op);

  fun_add_method(rls, f, m);
  toplevel_env_def(rls, Vm.globals, n, tag_obj(f), false, true);
  restore_sp(rls, sp);

  return f;
}

void print_fun(Port* ios, Expr x) {
  Fun* fun = as_fun(x);

  if ( fun->macro )
    pprintf(ios, "<macro:%s/%d>", fn_name(fun), fn_method_count(fun));

  else
    pprintf(ios, "<fun:%s/%d>", fn_name(fun), fn_method_count(fun));
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

  init_objs(rls, &m->upvs);

  return m;
}

Method* mk_method_s(RlState* rls, Fun* fun, int arity, bool va, OpCode op, Chunk* code) {
  Method* out = mk_method(rls, fun, arity, va, op, code);
  push(rls, tag_obj(out));
  return out;
}

Method* mk_closure(RlState* rls, Method* proto) {
  Method* cls; int count = method_upvalc(proto);

  if ( count == 0 )
    cls = proto;

  else {
    cls = clone_obj(rls, proto);
    objs_write(rls, &cls->upvs, NULL, count);
  }

  return cls;
}

Method* mk_builtin_method(RlState* rls, Fun* fun, int arity, bool va, OpCode op) {
  return mk_method(rls, fun, arity, va, op, NULL);
}

Method* mk_builtin_method_s(RlState* rls, Fun* fun, int arity, bool va, OpCode op) {
  Method* out = mk_builtin_method(rls, fun, arity, va, op);
  push(rls, tag_obj(out));
  return out;
}

Method* mk_user_method(RlState* rls, Fun* fun, int arity, bool va, Chunk* code) {
  int sp = save_sp(rls);
  Method* m = mk_method(rls, fun, arity, va, OP_NOOP, code);
  restore_sp(rls, sp);

  return m;
}

Method* mk_user_method_s(RlState* rls, Fun* fun, int arity, bool va, Chunk* code) {
  Method* out = mk_user_method(rls, fun, arity, va, code);
  push(rls, tag_obj(out));
  return out;
}

void disassemble_method(Method* m) {
  printf("\n\n==== %s ====\n\n", method_name(m));
  disassemble_chunk(m->chunk);
  printf("\n\n");
}

Expr upval_ref(Method* m, int i) {
  assert(i >= 0 && i < m->upvs.count);
  UpVal* upv = m->upvs.vals[i];

  return *deref(upv);
}

void upval_set(Method* m, int i, Expr x) {
  assert(i >= 0 && i < m->upvs.count);
  UpVal* upv = m->upvs.vals[i];

  *deref(upv) = x;
}

void print_method(Port* ios, Expr x) {
  Method* m = as_method(x);

  pprintf(ios, "<method:%s/%d%s>",
          method_name(m),
          m->arity,
          m->va ? "+" : "");
}

void clone_method(RlState* rls, void* ptr) {
  Method* m = ptr;

  if ( m->upvs.vals != NULL )
    m->upvs.vals = duplicate(rls, m->upvs.max_count, m->upvs.vals);
}

void trace_method(RlState* rls, void* ptr) {
  Method* m = ptr;

  mark_obj(rls, m->fun);
  mark_obj(rls, m->chunk);
  trace_objs(rls, &m->upvs);
}

// method table API
MethodTable* mk_mtable(RlState* rls, Fun* fun) {
  MethodTable* mt = mk_obj(rls, &MethodTableType, 0);
  mt->fun = fun;
  mt->bitmap = 0ul;
  mt->variadic = NULL;
  init_objs(rls, &mt->methods);
  return mt;
}

MethodTable* mk_mtable_s(RlState* rls, Fun* fun) {
  MethodTable* out = mk_mtable(rls, fun);
  push(rls, tag_obj(out));
  return out;
}

void mtable_add(RlState* rls, MethodTable* mt, Method* m) {
  // check for duplicate signature
  if ( method_va(m) ) {
    if ( mt->variadic != NULL )
      eval_error(rls, "varidic method already exists");

    mt->variadic = m;
  }

  else if ( bitmap_has(mt->bitmap, m->arity) )
    eval_error(rls, "method with arity %d already exists", m->arity);

  bitmap_set(&mt->bitmap, m->arity); // mark the bitmap
  objs_push(rls, &mt->methods, m);
}

Method* mtable_lookup(MethodTable* mt, int argc) {
  Method* out;

  int idx = bitmap_to_index(mt->bitmap, argc);

  if ( idx >= 0 )
    out = mt->methods.vals[idx];

  else if ( mt->variadic && argc >= mt->variadic->arity )
    out = mt->variadic;

  else
    out = NULL; // failure

  return out;
}

void print_mtable(Port* ios, Expr x) {
  MethodTable* mt = as_mtable(x);
  pprintf(ios, "<method-table:%s\\%d>", mtable_name(mt), mtable_count(mt));
}

void trace_mtable(RlState* rls, void* ptr) {
  MethodTable* mt = ptr;
  mark_obj(rls, mt->fun);
  mark_obj(rls, mt->variadic);
  trace_objs(rls, &mt->methods);
}
