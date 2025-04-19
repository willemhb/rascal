#ifndef rl_common_h
#define rl_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>

typedef unsigned  char byte;
typedef uintptr_t hash_t;

// Prompt/messages
#define PROMPT  "rl>"
#define VERSION "%d.%d.%d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR   0
#define MINOR   2
#define PATCH   0
#define RELEASE "a"

#endif
