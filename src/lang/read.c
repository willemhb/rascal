/**
 *
 * Toplevel read API.
 *
 **/

// headers --------------------------------------------------------------------
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "sys/error.h"
#include "sys/memory.h"
#include "sys/vm.h"

#include "data/data.h"

#include "lang/read.h"

#include "util/string.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
bool is_delim_char(int c);
bool is_sym_char(int c);
bool is_num_char(int c);
void skip_space(Port* in);
Expr read_glyph(Port* in);
Expr read_quote(Port* in);
Expr read_list(Port *in);
Expr read_string(Port* in);
Expr read_atom(Port* in);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
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

    if ( !isspace(c) ) {
      pungetc(p, c);
      break;
    }
  }
}


Expr read_glyph(Port* in) {
  pgetc(in); // consume opening slash

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error("invalid syntax: empty character");

  Glyph g; int c;

  if ( !isalpha(ppeekc(in)) ) {
    g = ppeekc(in);
    c = ppeekc(in);

    require(isspace(c) || is_delim_char(c), "invalid character literal");
  }

  else {
    while (!peof(in) && !isspace(c=ppeekc(in)) && !is_delim_char(c) ) {
      add_to_token(c);
      ppeekc(in);
    }

    if ( TOff == 1 )
      g = Token[0];

    else
      switch ( Token[0] ) {
        case 'n':
          if ( streq(Token+1, "ul") )
            g = '\0';
          
          else if ( streq(Token+1, "ewline") )
            g = '\n';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'b':
          if ( streq(Token+1, "el") )
            g = '\a';

          else if ( streq(Token+1, "ackspace") )
            g = '\b';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 's':
          if ( streq(Token+1, "pace") )
            g = ' ';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 't':
          if ( streq(Token+1, "ab") )
            g = '\t';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'r':
          if ( streq(Token+1, "eturn") )
            g = '\r';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'f':
          if ( streq(Token+1, "ormfeed") )
            g = '\f';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        case 'v':
          if ( streq(Token+1, "tab") )
            g = '\v';
          
          else
            eval_error("unrecognized character name \\%s", Token);
          
          break;
          
        default:
          eval_error("unrecognized character name \\%s", Token);
      }
  }

  return tag_glyph(g);
}

Expr read_quote(Port* in) {
  ppeekc(in); // consume opening '

  if ( peof(in) || isspace(ppeekc(in)) )
    eval_error("invalid syntax: quoted nothing");

  Sym* hd  = mk_sym("quote"); preserve(2, tag_obj(hd), NUL);
  Expr x   = read_exp(in);    add_to_preserved(1, x);
  List* qd = mk_list(2, preserved());

  return tag_obj(qd);
}

Expr read_list(Port* in) {
  List* out;
  ppeekc(in); // consume the '('
  skip_space(in);
  Expr* base = &Vm.vals[Vm.sp], x;
  int n = 0, c;

  while ( (c=ppeekc(in)) != ')' ) {
    if ( peof(in) )
      runtime_error("unterminated list");

    x = read_exp(in);
    vpush(x);
    n++;
    skip_space(in);
  }

  ppeekc(in); // consume ')'

  out = mk_list(n, base);

  if ( n > 0 )
    vpopn(n);

  return tag_obj(out);
}

Expr read_string(Port* in) {
  Str* out;

  int c;

  ppeekc(in); // consume opening '"'

  while ( (c=ppeekc(in)) != '"' ) {
    if ( peof(in) )
      runtime_error("unterminated string");

    add_to_token(c); // accumulate
    ppeekc(in);   // advance
  }

  ppeekc(in); // consume terminal '"'

  out = mk_str(Token);

  return tag_obj(out);
}

Expr read_atom(Port* in) {
  int c;
  Expr x;
  
  while ( !peof(in) && is_sym_char(c=ppeekc(in)) ) {
    add_to_token(c); // accumulate
    ppeekc(in);   // consume character
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

    else if ( strcmp(Token, "true") == 0 )
      x = TRUE;

    else if ( strcmp(Token, "false") == 0 )
      x = FALSE;

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

// external -------------------------------------------------------------------
Expr read_exp(Port *in) {
  reset_token();
  skip_space(in);
  Expr x;
  int c = ppeekc(in);

  if ( c == EOF )
    x = EOS;
  else if ( c == '\\' )
    x = read_glyph(in);
  else if ( c == '\'' )
    x = read_quote(in);
  else if ( c == '(' )
    x = read_list(in);
  else if ( c == '"')
    x = read_string(in);
  else if ( is_sym_char(c) )
    x = read_atom(in);
  else if ( c == ')' )
    eval_error("dangling ')'");
  else
    eval_error("unrecognized character %c", c);

  return x;
}

// initialization -------------------------------------------------------------
