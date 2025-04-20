#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "runtime.h"
#include "lang.h"

// Function prototypes
bool is_sym_char(int c);
bool is_num_char(int c);
int  peek(FILE *in);
char read_char(FILE *in);
void skip_space(FILE* in);
Expr read_list(FILE *in);
Expr read_string(FILE* in);
Expr read_atom(FILE* in);

// Global symbol table could be added here

// read helpers
bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[];\"", c);
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

Status read_exp(FILE *in, Expr* out) {
  Status s = OKAY;
  reset_token();
  skip_space(in);
  Expr x;
  int c = peek(in);
  
  if ( c == EOF )
    x = EOS;
  else if ( c == '(' )
    x = read_list(in);
  else if ( c == '"')
    x = read_string(in);
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
  read_char(in); // consume the '('
  skip_space(in);
  Expr* base = &Stack[Sp], x;
  int n = 0, c;

  while ( (c=peek(in)) != ')' ) {
    if ( feof(in) )
      runtime_error("unterminated list");

    read_exp(in, &x);
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

Expr read_string(FILE* in) {
  Str* out;

  int c;

  read_char(in); // consume opening '"'

  while ( (c=peek(in)) != '"' ) {
    if ( feof(in) )
      runtime_error("unterminated string");

    add_to_token(c); // accumulate
    read_char(in);   // advance
  }

  read_char(in); // consume terminal '"'

  out = mk_str(Token);

  return tag_obj(out);
}

Expr read_atom(FILE* in) {
  int c;
  Expr x;
  
  while ( !feof(in) && is_sym_char(c=peek(in)) ) {
    add_to_token(c); // accumulate
    read_char(in);   // consume character
  }

  assert(TOff > 0);

  if ( is_num_char(Token[0])) {
    char* end;

    Num n = strtod(Token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      if ( TOff > MAX_INTERN )
        runtime_error("symbol name '%s' too long", Token);
      
      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(Token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(Token, "none" ) == 0 )
      x = NONE;
    
    else if ( strcmp(Token, "<eos>" ) == 0 )
      x = EOS;

    else {
      if ( TOff > MAX_INTERN )
        runtime_error("symbol name '%s' too long", Token);

      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// eval
bool is_literal(Expr x) {
  ExpType t = exp_type(x);
  bool out;

  if ( t == EXP_SYM )
    out = as_sym(x)->val->val[0] == ':';

  else if ( t == EXP_LIST )
    out = false;

  else
    out = true;

  return out;
}

Expr eval_literal(Expr x) {
  return x;
}

Status eval_sexp(Expr x, Expr* out) {
  void* labels[] = {
    [OP_ADD] = &&op_add,
    [OP_SUB] = &&op_sub,
    [OP_MUL] = &&op_mul,
    [OP_DIV] = &&op_div,
  };

  Expr v;
  Status s = OKAY;

  if ( is_literal(x) )
    v = x;

  else if ( is_sym(x) )

 op_add:

 op_sub:

 end:
  *out = v;
  return s;
}

// print
void print_exp(FILE* out, Expr x) {
  ExpTypeInfo* info = exp_info(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    fprintf(out, "<%s>", info->name);
}

// repl
void repl(void) {
  Expr x, v;

  for (;;) {
    if ( safepoint() )
      recover();

    else {
      fprintf(stdout, PROMPT" ");
      read_exp(stdin, &x);
      eval_exp(x, &v);
      fprintf(stdout, "\n>>> ");
      print_exp(stdout, x);
      fprintf(stdout, "\n\n");
    }
  }
}
