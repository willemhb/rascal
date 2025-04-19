#include "common.h"
#include "data.h"
#include "runtime.h"
#include "lang.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Prompt/messages
#define PROMPT  "rl>"
#define VERSION "%d.%d.%d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR   0
#define MINOR   1
#define PATCH   0
#define RELEASE "a"


// expression APIs
Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

Obj* as_obj(Expr x) {
  return (Obj*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ;
}

void* mk_obj(ExpType type) {
  Obj* out = allocate(true, ExpTypeObjSize[type]);

  out->type   = type;
  out->marked = false;
  out->heap   = Heap;
  Heap        = out;

  return out;
}

Sym* mk_sym(char* val) {
  Sym* s = mk_obj(EXP_SYM);
  s->val = strdup(val);

  return s;
}

static List* empty_list(void) {
  List* l = mk_obj(EXP_LIST);

  l->head  = NUL;
  l->tail  = NULL;
  l->count = 0;

  return l;
}

static List* new_lists(size_t n) {
  assert(n > 0);

  size_t nb = (n+1) * sizeof(List);
  List* xs  = allocate(true, nb);

  // initialize terminal empty list
  for ( size_t i=0; i < n; i++ ) {
    List* cell = &xs[i];

    // initialize the list object
    cell->heap   = (Obj*)(&cell+1);
    cell->type   = EXP_LIST;
    cell->marked = false;
    cell->head   = NUL;
    cell->tail   = cell + 1;
    cell->count  = n - i;
  }

  // handle the terminal empty list specially
  List* cell  = &xs[n];
  cell->heap   = Heap;
  cell->type   = EXP_LIST;
  cell->marked = false;
  cell->head   = NUL;
  cell->tail   = NULL;
  cell->count  = 0;

  // add it all to the heap
  Heap = (Obj*)xs;

  return xs;
}

List* mk_list(size_t n, Expr* xs) {
  List* l;

  if ( n == 0 )
    l = empty_list();

  else {
    l = new_lists(n);

    for ( size_t i=0; i<n; i++ )
      l[i].head = xs[i];
  }

  return l;
}

// implmentations
ExpType expr_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NUL  : t = EXP_NUL;         break;
    case EOS_T: t = EXP_EOS;         break;
    case OBJ  : t = as_obj(x)->type; break;
    default   : t = EXP_NUM;         break;
  }

  return t;
}

// Function prototypes
bool is_sym_char(int c);
bool is_num_char(int c);
int peek(FILE *in);
char read_char(FILE *in);
void skip_space(FILE* in);
Expr read_list(FILE *in);
Expr read_atom(FILE* in);
Status read_expr(FILE *in, Expr* out);

// Global symbol table could be added here

// read helpers
bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[];", c);
}

bool is_num_char(int c) {
  return isdigit(c) || strchr(".+-", c);
}

int peek(FILE *in) {
    int c = fgetc(in);
    ungetc(c, in);
    return c;
}

char read_char(FILE *in) {
    return fgetc(in);
}

// read helpers
void skip_space(FILE* f) {
  int c;
  
  while ( !feof(f) ) {
    c = fgetc(f);

    if ( !isspace(c) ) {
      ungetc(c, f);
      break;
    }
  }
}

Status read_expr(FILE *in, Expr* out) {
  Status s = OKAY;
  reset_token();
  skip_space(in);
  Expr x;
  int c = peek(in);
  
  if ( c == EOF )
    x = EOS;
  else if ( c == '(' )
    x = read_list(in);
  else if ( is_sym_char(c) )
    x = read_atom(in);
  else if ( c == ')' )
    runtime_error("dangling ')'");
  else
    runtime_error("unrecognized character %c", c);

  *out = x;

  return s;
}

Expr read_list(FILE *in) {
  List* out;
  skip_space(in);
  read_char(in); // consume the '('
  Expr* base = &Stack[Sp], x;
  int n = 0, c;
  
  while ( (c=peek(in)) != ')' ) {
    if ( feof(in) )
      runtime_error("unterminated list");

    read_expr(in, &x);
    push(x);
    n++;
    skip_space(in);
  }

  read_char(in); // consume ')'

  out = mk_list(n, base);

  if ( n > 0 )
    popn(n);

  return tag_obj(out);
}

Expr read_atom(FILE* in) {
  int c;
  Expr x;
  
  while ( !feof(in) && is_sym_char(c=peek(in)) ) {
    add_to_token(c); // accumulate
    read_char(in);   // consume character
  }

  assert(TBOffset > 0);

  if ( is_num_char(Token[0])) {
    char* end;

    Num n = strtod(Token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(Token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(Token, "<eos>" ) == 0 )
      x = EOS;

    else {
      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// eval
Status eval_expr(Expr x, Expr* out) {
  Expr v;
  Status s = OKAY;

  switch ( expr_type(x) ) {
    default: v = x; break;
  }

  *out = v;

  return s;
}

// print
void print_num(FILE* out, Expr x) {
  
}

void print_expr(FILE* out, Expr x) {
  switch ( expr_type(x) ) {
    case EXP_LIST: {
      fprintf(out, "(");

      List* xs = as_list(x);

      while ( xs->count > 0 ) {
        print_expr(out, xs->head);

        if ( xs->count > 1 )
          fprintf(out, " ");

        xs = xs->tail;
      }

      fprintf(out, ")");

      break;
    }

    case EXP_NUL: fprintf(out, "nul");                             break;
    case EXP_SYM: fprintf(out, "%s", as_sym(x)->val);              break;
    case EXP_NUM: fprintf(out, "%g", as_num(x));                   break;
    default:      fprintf(out, "<%s>", ExpTypeName[expr_type(x)]); break;
  }
}

// void
void repl(void) {
  Expr x, v;

  for (;;) {
    if ( safepoint() )
      recover();

    else {
      fprintf(stdout, PROMPT" ");
      read_expr(stdin, &x);
      eval_expr(x, &v);
      fprintf(stdout, "\n>>> ");
      print_expr(stdout, x);
      fprintf(stdout, "\n\n");
    }
  }
}

// setup/teardown
void setup(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void teardown(void) {}

// entry point
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  setup();
  repl();
  teardown();

  return 0;
}
