#ifndef rl_util_ios_h
#define rl_util_ios_h

#include "common.h"

/* globals */
#define DIGITS   "0123456789"
#define XDIGITS  "abcdefABCDEF"DIGITS
#define BDIGITS  "01"
#define ODIGITS  "01234567"
#define LOWERS   "abcdefghijklmnopqrstuvwxyz"
#define UPPERS   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALPHAS   LOWERS""UPPERS
#define PAIRED   "{}()[]"
#define SPACES   " \n\v\f\r"
#define RLSPACES ","SPACES
#define PUNCTS   "!\"#$%&'()*+-,-./"
#define SYMBOLS  LOWERS"!?$&+-*/%=<>"UPPERS


/* API */
int fpeekc(FILE *file);
int  newline(void);
int  fnewline(FILE *file);
bool isrlspc(int ch);
bool isrldlm(int ch);
bool isrlsymchr(int ch);

#endif
