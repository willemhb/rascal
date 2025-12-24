#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "lang/read.h"
#include "val.h"
#include "vm.h"
#include "util/util.h"

// Function prototypes --------------------------------------------------------
// Helpers --------------------------------------------------------------------
bool is_delim_char(int c);
bool is_sym_char(int c);
bool is_num_char(int c);
void skip_space(Port* in);
Expr read_glyph(RlState* rls, Port* in);
Expr read_quote(RlState* rls, Port* in);
Expr read_list(RlState* rls, Port *in);
Expr read_string(RlState* rls, Port* in);
Expr read_atom(RlState* rls, Port* in);

// Function definitions -------------------------------------------------------
// Helpers --------------------------------------------------------------------
bool is_delim_char(int c) {
  return strchr("(){}[]", c);
}

bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[];\"", c);
}

bool is_num_char(int c) {
  return isdigit(c) || strchr(".+-", c);
}

void skip_space(Port* p) {
  int c;

  while ( !peof(p) ) {
    c = pgetc(p);

    if ( c == ';' ) {
      // skip comment to end of line
      while ( !peof(p) && (c = pgetc(p)) != '\n' );
    } else if ( !isspace(c) ) {
      pungetc(p, c);
      break;
    }
  }
}

// Dispatch -------------------------------------------------------------------
Expr read_glyph(RlState* rls, Port* in) {
  pgetc(in); // consume opening slash

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error(rls, "invalid syntax: empty character");

  Glyph g; int c;

  if ( !isalpha(ppeekc(in)) ) {
    g = pgetc(in);
    c = ppeekc(in);

    require(rls, isspace(c) || is_delim_char(c), "invalid character literal");
  }

  else {
    while (!peof(in) && !isspace(c=ppeekc(in)) && !is_delim_char(c) ) {
      add_to_token(rls, c);
      pgetc(in);
    }

    if ( rls->toff == 1 )
      g = rls->token[0];

    else
      switch ( rls->token[0] ) {
        case 'n':
          if ( streq(rls->token+1, "ul") )
            g = '\0';

          else if ( streq(rls->token+1, "ewline") )
            g = '\n';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'b':
          if ( streq(rls->token+1, "el") )
            g = '\a';

          else if ( streq(rls->token+1, "ackspace") )
            g = '\b';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 's':
          if ( streq(rls->token+1, "pace") )
            g = ' ';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 't':
          if ( streq(rls->token+1, "ab") )
            g = '\t';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'r':
          if ( streq(rls->token+1, "eturn") )
            g = '\r';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'f':
          if ( streq(rls->token+1, "ormfeed") )
            g = '\f';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        case 'v':
          if ( streq(rls->token+1, "tab") )
            g = '\v';

          else
            eval_error(rls, "unrecognized character name \\%s", rls->token);

          break;

        default:
          eval_error(rls, "unrecognized character name \\%s", rls->token);
      }
  }

  return tag_glyph(g);
}

Expr read_quote(RlState* rls, Port* in) {
  pgetc(in); // consume opening '

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error(rls, "invalid syntax: quoted nothing");

  int sp = save_sp(rls);
  mk_sym_s(rls, "quote");
  Expr x = read_exp(rls, in); push(rls, x);
  List* qd = mk_list(rls, 2, stack_ref(rls, -2));
  restore_sp(rls, sp);

  return tag_obj(qd);
}

Expr read_list(RlState* rls, Port* in) {
  List* out;
  pgetc(in); // consume the '('
  skip_space(in);
  Expr x;
  int n = 0, c, sp = save_sp(rls);

  while ( (c=ppeekc(in)) != ')' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated list");

    x = read_exp(rls, in);
    push(rls, x);
    n++;
    skip_space(in);
  }

  pgetc(in); // consume ')'
  out = mk_list(rls, n, stack_ref(rls, -n));
  restore_sp(rls, sp); // restore stack

  return tag_obj(out);
}

Expr read_string(RlState* rls, Port* in) {
  Str* out;

  int c;

  pgetc(in); // consume opening '"'

  while ( (c=ppeekc(in)) != '"' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated string");

    add_to_token(rls, c); // accumulate
    pgetc(in);   // advance
  }

  pgetc(in); // consume terminal '"'

  out = mk_str(rls, rls->token);

  return tag_obj(out);
}

Expr read_atom(RlState* rls, Port* in) {
  int c;
  Expr x;

  while ( !peof(in) && is_sym_char(c=ppeekc(in)) ) {
    add_to_token(rls, c); // accumulate
    pgetc(in);   // consume character
  }

  assert(rls->toff > 0);

  if ( is_num_char(rls->token[0])) {
    char* end;

    Num n = strtod(rls->token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      if ( rls->toff > MAX_INTERN )
        runtime_error(rls, "symbol name '%s' too long", rls->token);

      Sym* s = mk_sym(rls, rls->token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(rls->token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(rls->token, "none" ) == 0 )
      x = NONE;

    else if ( strcmp(rls->token, "true") == 0 )
      x = TRUE;

    else if ( strcmp(rls->token, "false") == 0 )
      x = FALSE;

    else if ( strcmp(rls->token, "<eos>" ) == 0 )
      x = EOS;

    else {
      if ( rls->toff > MAX_INTERN )
        runtime_error(rls, "symbol name '%s' too long", rls->token);

      Sym* s = mk_sym(rls, rls->token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// External -------------------------------------------------------------------
Expr read_exp(RlState* rls, Port *in) {
  reset_token(rls);
  skip_space(in);
  Expr x;
  int c = ppeekc(in);

  if ( c == EOF )
    x = EOS;
  else if ( c == '\\' )
    x = read_glyph(rls, in);
  else if ( c == '\'' )
    x = read_quote(rls, in);
  else if ( c == '(' )
    x = read_list(rls, in);
  else if ( c == '"')
    x = read_string(rls, in);
  else if ( is_sym_char(c) )
    x = read_atom(rls, in);
  else if ( c == ')' ) {
    pgetc(in); // clear dangling ')'
    eval_error(rls, "dangling ')'");
  }
  else
    eval_error(rls, "unrecognized character %c", c);

  return x;
}

List* read_file(RlState* rls, char* fname) {
  int n = 0, sp = save_sp(rls);
  Port* in = open_port_s(rls, fname, "r");
  Expr x;

  while ( (x = read_exp(rls, in)) != EOS ) {
    push(rls, x);
    n++;
  }

  close_port(in);
  List* out = mk_list(rls, n, stack_ref(rls, -n));
  restore_sp(rls, sp); // restore stack
  return out;
}


List* read_file_s(RlState* rls, char* fname) {
  List* out = read_file(rls, fname);
  push(rls, tag_obj(out));
  return out;
}
