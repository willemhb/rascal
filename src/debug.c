#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "debug.h"

static const char* tokenTypeName(TokenType type) {
  const char* out;

  switch (type) {
    case NUMBER_TOKEN:        out = "number-token";      break;
    case STRING_TOKEN:        out = "string-token";      break;
    case SYMBOL_TOKEN:        out = "symbol-token";      break;
    case KEYWORD_TOKEN:       out = "keyword-token";     break;
    case IDENTIFIER_TOKEN:    out = "identifier-token";  break;
    case TRUE_TOKEN:          out = "true-token";        break;
    case FALSE_TOKEN:         out = "false-token";       break;
    case NUL_TOKEN:           out = "nil-token";         break;
    case LPAR_TOKEN:          out = "left-parenthesis";  break;
    case RPAR_TOKEN:          out = "right-parenthesis"; break;
    case LARROWS_TOKEN:       out = "left-arrows";       break;
    case RARROWS_TOKEN:       out = "right-arrows";      break;
    case LBRACK_TOKEN:        out = "left-bracket";      break;
    case RBRACK_TOKEN:        out = "right-bracket";     break;
    case LBRACE_TOKEN:        out = "left-brace";        break;
    case RBRACE_TOKEN:        out = "right-brace";       break;
    case DO_TOKEN:            out = "do-block-start";    break;
    case END_TOKEN:           out = "do-block-end";      break;
    case COMMA_TOKEN:         out = "comma";             break;
    case DOT_TOKEN:           out = "dot";               break;
    case EQUAL_TOKEN:         out = "equal";             break;
    case MATCH_TOKEN:         out = "match";             break;
    case COLON_COLON_TOKEN:   out = "type-assert";       break;
    case OR_TOKEN:            out = "or";                break;
    case AND_TOKEN:           out = "and";               break;
    case EQUAL_EQUAL_TOKEN:   out = "equals";            break;
    case NOT_EQUAL_TOKEN:     out = "not-equal";         break;
    case LESS_THAN_TOKEN:     out = "less-than";         break;
    case GREATER_THAN_TOKEN:  out = "greater-than";      break;
    case LESS_EQUAL_TOKEN:    out = "less-or-equal";     break;
    case GREATER_EQUAL_TOKEN: out = "greater-or-equal";  break;
    case PLUS_TOKEN:          out = "plus";              break;
    case MINUS_TOKEN:         out = "minus";             break;
    case BAR_TOKEN:           out = "bar";               break;
    case MUL_TOKEN:           out = "multiply";          break;
    case DIV_TOKEN:           out = "divide";            break;
    case REM_TOKEN:           out = "remainder";         break;
    case APOSTROPHE_TOKEN:    out = "apostrophe";        break;
    case NOT_TOKEN:           out = "not";               break;
    case EOF_TOKEN:           out = "end-of-input";      break;
    case ERROR_TOKEN:         out = "error";             break;
  }

  return out;
}

static void displayToken(Token token) {
  static const char* fmt = "  %-20s [%.3d] - %-40s\n";
  char buffer[token.length+1];
  memcpy(buffer, token.start, token.length);
  buffer[token.length] = '\0';
  const char* type = tokenTypeName(token.type);

  fprintf(stdout, fmt, type, token.lineNo, buffer);
}

void displayScanner(Scanner* scanner) {
  fprintf(stdout, "Scan result:\n");

  for (size_t i=0; i<scanner->tokens.count; i++)
    displayToken(peekToken(scanner, i));
}
