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
void skip_space(Port* in, int* line);
Expr read_glyph(RlState* rls, Port* in);
Expr read_quote(RlState* rls, Port* in, int* line);
Expr read_tick(RlState* rls, Port* in, int* line);
Expr read_tilde(RlState* rls, Port* in, int* line);
Expr read_list(RlState* rls, Port *in, int* line);
Expr read_tuple(RlState* rls, Port* in, int* line);
Expr read_map(RlState* rls, Port* in, int* line);
Expr read_string(RlState* rls, Port* in);
Expr read_atom(RlState* rls, Port* in);

// Function definitions -------------------------------------------------------
// Helpers --------------------------------------------------------------------
#define read_error(rls, args...) eval_error(rls, args)
#define read_require(rls, t, args...)           \
  do {                                          \
    if ( !(t) )                                 \
      read_error(rls, args);                    \
  } while ( false )

bool is_delim_char(int c) {
  return strchr("(){}[]", c);
}

bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[],;\"", c);
}

bool is_num_char(int c) {
  return isdigit(c) || strchr(".+-", c);
}

bool is_sep_char(int c) {
  return isspace(c) || c == ',';
}

void skip_space(Port* p, int* line) {
  int c;

  while ( !peof(p) ) {
    c = pgetc(p);

    if ( c == ';' ) {
      // skip comment to end of line
      while ( !peof(p) && (c = pgetc(p)) != '\n' );
    } else if ( !is_sep_char(c) ) {
      pungetc(p, c);
      break;
    }

    if ( line && c == '\n' )
      (*line)++;
  }
}

// Dispatch -------------------------------------------------------------------
Expr read_glyph(RlState* rls, Port* in) {
  pgetc(in); // consume opening slash

  if ( peof(in) || isspace(ppeekc(in)) )
    read_error(rls, "invalid syntax: empty character.");

  Glyph g; int c;

  if ( !isalpha(ppeekc(in)) ) {
    g = pgetc(in);
    c = ppeekc(in);

    // TODO: need to handle character literals for delimiters and separators
    read_require(rls, isspace(c) || is_delim_char(c), "invalid character literal.");
  }

  else {
    while (!peof(in) && !isspace(c=ppeekc(in)) && !is_delim_char(c) ) {
      add_to_token(rls, c);
      pgetc(in);
    }

    char* token = token_val(rls);

    if ( token_size(rls) == 1 )
      g = token[0];

    else
      switch ( token[0] ) {
        case 'n':
          if ( streq(token+1, "ul") )
            g = '\0';
          else if ( streq(token+1, "ewline") )
            g = '\n';
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        case 'e':
          if ( streq(token+1, "os") )
            g = (int)EOF;
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        case 'b':
          if ( streq(token+1, "el") )
            g = '\a';
          else if ( streq(token+1, "ackspace") )
            g = '\b';
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        case 's':
          if ( streq(token+1, "pace") )
            g = ' ';
          else
            eval_error(rls, "unrecognized character name \\%s", token);
          break;

        case 't':
          if ( streq(token+1, "ab") )
            g = '\t';
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        case 'r':
          if ( streq(token+1, "eturn") )
            g = '\r';
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        case 'f':
          if ( streq(token+1, "ormfeed") )
            g = '\f';
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        case 'v':
          if ( streq(token+1, "tab") )
            g = '\v';
          else
            read_error(rls, "unrecognized character name \\%s", token);
          break;

        default:
          read_error(rls, "unrecognized character name \\%s", token);
      }
  }

  return tag_glyph(g);
}

Expr read_quote(RlState* rls, Port* in, int* line) {
  pgetc(in); // consume opening '

  if ( peof(in) || isspace(ppeekc(in)) )
    read_error(rls, "invalid syntax: quoted nothing.");

  StackRef top = rls->s_top;
  mk_sym_s(rls, "quote");
  Expr x = read_expr(rls, in, line);
  stack_push(rls, x);
  List* qd = mk_list(rls, 2);
  rls->s_top = top;

  return tag_obj(qd);
}

Expr read_tick(RlState* rls, Port* in, int* line) {
  pgetc(in); // consume opening `

  if ( peof(in) || isspace(ppeekc(in)) )
    read_error(rls, "invalid syntax: quoted nothing.");

  StackRef top = rls->s_top;
  mk_sym_s(rls, "backquote");
  Expr x = read_expr(rls, in, line);
  stack_push(rls, x);
  List* qd = mk_list(rls, 2);
  rls->s_top = top;

  return tag_obj(qd);
}


Expr read_tilde(RlState* rls, Port* in, int* line) {
  pgetc(in); // consume opening ~
  int c;

  if ( peof(in) || isspace(c=ppeekc(in)) )
    read_error(rls, "invalid syntax: unquoted nothing.");

  char* macro_name = "unquote";

  if ( c == '@' ) {
    pgetc(in); // consume @

    if ( peof(in) || isspace(ppeekc(in)) )
      read_error(rls, "invalid syntax: unquoted nothing.");

    macro_name = "unquote-splice";
  }

  StackRef top = rls->s_top;
  mk_sym_s(rls, macro_name);
  Expr x = read_expr(rls, in, line);
  stack_push(rls, x);
  List* qd = mk_list(rls, 2);
  rls->s_top = top;

  return tag_obj(qd);
}

Expr read_tuple(RlState* rls, Port* in, int* line) {
  Tuple* out;
  pgetc(in); // consume the '['
  skip_space(in, line);
  Expr x;
  int n = 0, c;
  StackRef top = rls->s_top;

  while ( (c=ppeekc(in)) != ']' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated tuple.");

    x = read_expr(rls, in, line);
    stack_push(rls, x);
    n++;
    skip_space(in, line);
  }

  pgetc(in); // consume ']'
  out = mk_tuple(rls, n);
  rls->s_top = top; // restore stack

  return tag_obj(out);
}

Expr read_map(RlState* rls, Port* in, int* line) {
  // Reads {:key1 val1 :key2 val2} and creates (make-map key1 val1 key2 val2)
  List* out;
  pgetc(in); // consume the '{'
  skip_space(in, line);
  Expr x;
  int n = 0, c;
  StackRef top = rls->s_top;

  // push the 'make-map symbol first
  mk_sym_s(rls, "make-map");
  n++;

  while ( (c=ppeekc(in)) != '}' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated map.");

    x = read_expr(rls, in, line);
    stack_push(rls, x);
    n++;
    skip_space(in, line);
  }

  pgetc(in); // consume '}'

  // Validate even number of key-value pairs (n-1 for make-map symbol)
  if ( (n - 1) % 2 != 0 )
    read_error(rls, "map literal requires an even number of elements.");

  out = mk_list(rls, n);
  rls->s_top = top; // restore stack

  return tag_obj(out);
}

Expr read_list(RlState* rls, Port* in, int* line) {
  List* out;
  pgetc(in); // consume the '('
  int n = 0, c;
  skip_space(in, line);
  Expr x;
  StackRef top = rls->s_top;

  while ( (c=ppeekc(in)) != ')' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated list.");

    x = read_expr(rls, in, line);
    stack_push(rls, x);
    n++;
    skip_space(in, line);
  }

  pgetc(in); // consume ')'
  out = mk_list(rls, n);
  rls->s_top = top; // restore stack

  return tag_obj(out);
}

Expr read_string(RlState* rls, Port* in) {
  Str* out;
  int c;
  pgetc(in); // consume opening '"'

  while ( (c=ppeekc(in)) != '"' ) {
    if ( peof(in) )
      runtime_error(rls, "unterminated string.");

    add_to_token(rls, c); // accumulate
    pgetc(in);   // advance
  }

  pgetc(in); // consume terminal '"'
  out = mk_str(rls, token_val(rls));

  return tag_obj(out);
}

Expr read_atom(RlState* rls, Port* in) {
  int c;
  Expr x;

  while ( !peof(in) && is_sym_char(c=ppeekc(in)) ) {
    add_to_token(rls, c); // accumulate
    pgetc(in);   // consume character
  }

  char* token = token_val(rls);

  assert(token_size(rls) > 0);

  if ( is_num_char(token[0])) {
    char* end;

    Num n = strtod(token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      if ( token_size(rls) > MAX_INTERN )
        read_error(rls, "symbol name '%s' too long", token);

      Sym* s = mk_sym(rls, token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(token, "none" ) == 0 )
      x = NONE;

    else if ( strcmp(token, "true") == 0 )
      x = TRUE;

    else if ( strcmp(token, "false") == 0 )
      x = FALSE;

    else {
      if ( token_size(rls) > MAX_INTERN )
        read_error(rls, "symbol name '%s' too long", token);

      Sym* s = mk_sym(rls, token);
      x = tag_obj(s);
    }
  }

  return x;
}

// External -------------------------------------------------------------------
Expr read_expr(RlState* rls, Port *in, int* line) {
  reset_token(rls);
  skip_space(in, line);
  int line1 = line ? *line : -1;
  Expr x;
  int c = ppeekc(in);

  if ( c == EOF )
    x = EOS;
  else if ( c == '\\' )
    x = read_glyph(rls, in);
  else if ( c == '\'' )
    x = read_quote(rls, in, line);
  else if ( c == '`' )
    x = read_tick(rls, in, line);
  else if ( c == '~' )
    x = read_tilde(rls, in, line);
  else if ( c == '(' )
    x = read_list(rls, in, line);
  else if ( c == '"')
    x = read_string(rls, in);
  else if ( c == '[' )
    x = read_tuple(rls, in, line);
  else if ( c == '{' )
    x = read_map(rls, in, line);
  else if ( is_sym_char(c) )
    x = read_atom(rls, in);
  else if ( strchr(")]}", c) ) {
    pgetc(in); // clear dangling delimiter
    read_error(rls, "dangling '%c'", c);
  }
  else
    read_error(rls, "unrecognized character %c", c);

  if ( is_list(x) ) // set line info
    as_list(x)->line = line1;

  return x;
}
