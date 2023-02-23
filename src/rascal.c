#include "value.h"
#include "object.h"
#include "type.h"

#include "read.h"
#include "eval.h"
#include "print.h"
#include "compile.h"
#include "compare.h"

#include "util/io.h"

// globals --------------------------------------------------------------------
#define MAJOR       0
#define MINOR       0
#define DEVELOPMENT 1
#define PATCH       "a"
#define VERSION     "%d.%d.%d.%s"

// main sequence --------------------------------------------------------------
void rascal_init(void);
void rascal_welcome(void);
void rascal_main(int argc, const char* argv[argc]);
void rascal_cleanup(void);
void rascal_goodbye(void);

void rascal_init(void) {
  extern void type_init(void);
  extern void object_init(void);
  extern void runtime_init(void);
  extern void native_init(void);

  type_init();
  object_init();
  runtime_init();
  native_init();
}

void rascal_welcome(void) {
  printf("welcome to rascal version "VERSION"!\n\n", MAJOR, MINOR, DEVELOPMENT, PATCH);
}

void rascal_main(int argc, const char* argv[argc]) {
  (void)argv;
  repl();
}

void rascal_goodbye(void) {
  printf("rascal version "VERSION"exiting normally. Later sluts.\n\n", MAJOR, MINOR, DEVELOPMENT, PATCH);
}

void rascal_cleanup(void) {}

int main(const int argc, const char *argv[argc]) {
  rascal_init();
  rascal_welcome();
  rascal_main(argc, argv);
  rascal_cleanup();
  rascal_goodbye();
  return 0;
}
