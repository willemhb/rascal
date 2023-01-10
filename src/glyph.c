#include <assert.h>

#include "glyph.h"
#include "util/string.h"

/* API */
/* internal */
void prin_glyph(val_t x) {
  printf("\\%s", gtos(as_glyph(x)));
}

/* external */
int stog(char *s) {
  switch (s[0]) {
  case '\0' ... ' ':
  case '\x7f':
    return EOF; // control & whitespace characters not used in rascal character names
  case 'a':
    switch (s[1]) {
    case '\0':
      return 'a';
    case 'c':
      return streq(s, "ack") ? '\x06' : EOF;
    default:
      return EOF;
    }
  case 'b':
    switch (s[1]) {
    case '\0':
      return 'b';
    case 'e':
      return streq(s, "bel") ? '\x07' : EOF;
    case 's':
      return streq(s, "bs") ? '\b' : EOF;
    default:
      return EOF;
    }
  case 'c':
    switch (s[1]) {
    case '\0':
      return 'c';
    case 'a':
      return streq(s, "can") ? '\x18' : EOF;
    case 'r':
      return streq(s, "cr")  ? '\r' : EOF;
    default:
      return EOF;
    }
  case 'd':
    switch (s[1]) {
    case '\0':
      return 'd';
    case 'c':
      switch (s[2]) {
      case '1':
        return streq(s, "dc1") ? '\x11' : EOF;
      case '2':
        return streq(s, "dc2") ? '\x12' : EOF;
      case '3':
        return streq(s, "dc3") ? '\x13' : EOF;
      case '4':
        return streq(s, "dc4") ? '\x15' : EOF;
      default:
        return EOF;
      }
    case 'e':
      return streq(s, "del") ? '\x7f' : EOF;
    case 'l':
      return streq(s, "dle") ? '\x10' : EOF;
    default:
      return EOF;
    }
  case 'e':
    switch (s[1]) {
    case '\0':
      return 'e';
    case 'm':
      return streq(s, "em") ? '\x19' : EOF;
    case 'n':
      return streq(s, "enq") ? '\x05' : EOF;
    case 'o':
      return streq(s, "eot") ? '\x04' : EOF;
    case 's':
      return streq(s, "esc") ? '\x1b' : EOF;
    case 't':
      switch (s[2]) {
      case 'b':
        return streq(s, "etb") ? '\x17' : EOF;
      case 'x':
        return streq(s, "etx") ? '\x03' : EOF;
      default:
        return EOF;
      }
    default:
      return EOF;
    }
  case 'f':
    switch (s[1]) {
    case '\0':
      return 'f';
    case 'f':
      return streq(s, "ff") ? '\x0c' : EOF;
    case 's':
      return streq(s, "fs") ? '\x1c' : EOF;
    default:
      return EOF;
    }
  case 'g':
    switch (s[1]) {
    case '\0':
      return 'g';
    case 's':
      return streq(s, "gs") ? '\x1d' : EOF;
    default:
      return EOF;
    }
  case 'l':
    switch (s[1]) {
    case '\0':
      return 'l';
    case 'f':
      return streq(s, "lf") ? '\n' : EOF;
    default:
      return EOF;
    }
  case 'n':
    switch (s[1]) {
    case '\0':
      return 'n';
    case 'a':
      return streq(s, "nak") ? '\x15' : EOF;
    case 'u':
      return streq(s, "nul") ? '\0' : EOF;
    default:
      return EOF;
    }
  case 'r':
    switch (s[1]) {
    case '\0':
      return 'r';
    case 's':
      return streq(s, "rs") ? '\x1e' : EOF;
    default:
      return EOF;
    }
  case 's':
    switch (s[1]) {
    case '\0':
      return 's';
    case 'i':
      return streq(s, "si") ? '\x0f' : EOF;
    case 'o':
      switch (s[2]) {
      case '\0':
        return '\x0e';
      case 'h':
        return streq(s, "soh") ? '\x01' : EOF;
      default:
        return EOF;
      }
    case 'p':
      return streq(s, "spc") ? ' ' : EOF;
    case 't':
      return streq(s, "stx") ? '\x02' : EOF;
    case 'u':
      return streq(s, "sub") ? '\x1a' : EOF;
    case 'y':
      return streq(s, "syn") ? '\x15' : EOF;
    default:
      return EOF;
    }
  case 't':
    switch (s[1]) {
    case '\0':
      return 't';
    case 'a':
      return streq(s, "tab") ? '\t' : EOF;
    default:
      return EOF;
    }
  case 'u':
    switch (s[1]) {
    case '\0':
      return 'u';
    case 's':
      return streq(s, "us") ? '\x1f' : EOF;
    default:
      return EOF;
    }
  case 'v':
    switch (s[1]) {
    case '\0':
      return 'v';
    case 't':
      return streq(s, "vt") ? '\v' : EOF;
    default:
      return EOF;
    }
  default:
    switch (s[1]) {
    case '\0': return s[0];
    default:   return EOF;
    }
  }
}

char *gtos(glyph_t g) {
  switch (g) {
  case '\0':   return "nul";
  case '\x01': return "soh";
  case '\x02': return "stx";
  case '\x03': return "etx";
  case '\x04': return "eot";
  case '\x05': return "enq";
  case '\x06': return "ack";
  case '\x07': return "bel";
  case '\b':   return "bs";
  case '\t':   return "tab";
  case '\n':   return "lf";
  case '\v':   return "vt";
  case '\x0c': return "ff";
  case '\r':   return "cr";
  case '\x0e': return "so";
  case '\x0f': return "si";
  case '\x10': return "dle";
  case '\x11': return "dc1";
  case '\x12': return "dc2";
  case '\x13': return "dc3";
  case '\x14': return "dc4";
  case '\x15': return "nak";
  case '\x16': return "syn";
  case '\x17': return "etb";
  case '\x18': return "can";
  case '\x19': return "em";
  case '\x1a': return "sub";
  case '\x1b': return "esc";
  case '\x1c': return "fs";
  case '\x1d': return "gs";
  case '\x1e': return "rs";
  case '\x1f': return "us";
  case ' ':    return "spc";
  case '!':    return "!";
  case '"':    return "\"";
  case '#':    return "#";
  case '$':    return "$";
  case '%':    return "%";
  case '&':    return "&";
  case '\'':   return "'";
  case '(':    return "(";
  case ')':    return ")";
  case '*':    return "*";
  case '+':    return "+";
  case ',':    return ",";
  case '-':    return "-";
  case '.':    return ".";
  case '/':    return "/";
  case '0':    return "0";
  case '1':    return "1";
  case '2':    return "2";
  case '3':    return "3";
  case '4':    return "4";
  case '5':    return "5";
  case '6':    return "6";
  case '7':    return "7";
  case '8':    return "8";
  case '9':    return "9";
  case ':':    return ":";
  case ';':    return ";";
  case '<':    return "<";
  case '=':    return "=";
  case '>':    return ">";
  case '?':    return "?";
  case '@':    return "@";
  case 'A':    return "A";
  case 'B':    return "B";
  case 'C':    return "C";
  case 'D':    return "D";
  case 'E':    return "E";
  case 'F':    return "F";
  case 'G':    return "G";
  case 'H':    return "H";
  case 'I':    return "I";
  case 'J':    return "J";
  case 'K':    return "K";
  case 'L':    return "L";
  case 'M':    return "M";
  case 'N':    return "N";
  case 'O':    return "O";
  case 'P':    return "P";
  case 'Q':    return "Q";
  case 'R':    return "R";
  case 'S':    return "S";
  case 'T':    return "T";
  case 'U':    return "U";
  case 'V':    return "V";
  case 'W':    return "W";
  case 'X':    return "X";
  case 'Y':    return "Y";
  case 'Z':    return "Z";
  case '[':    return "[";
  case '\\':   return "\\";
  case ']':    return "]";
  case '^':    return "^";
  case '_':    return "_";
  case '`':    return "`";
  case 'a':    return "a";
  case 'b':    return "b";
  case 'c':    return "c";
  case 'd':    return "d";
  case 'e':    return "e";
  case 'f':    return "f";
  case 'g':    return "g";
  case 'h':    return "h";
  case 'i':    return "i";
  case 'j':    return "j";
  case 'k':    return "k";
  case 'l':    return "l";
  case 'm':    return "m";
  case 'n':    return "n";
  case 'o':    return "o";
  case 'p':    return "p";
  case 'q':    return "q";
  case 'r':    return "r";
  case 's':    return "s";
  case 't':    return "t";
  case 'u':    return "u";
  case 'v':    return "v";
  case 'w':    return "w";
  case 'x':    return "x";
  case 'y':    return "y";
  case 'z':    return "z";
  case '{':    return "{";
  case '|':    return "|";
  case '}':    return "}";
  case '~':    return "~";
  case '\x7f': return "del";
  case '\xff': return "eos";
  }
  rl_unreachable();
}
