#include "value.h"
#include "object.h"

/* main sequence */
static void rascal_init(void)    {}
static void rascal_welcome(void) {}
static void rascal_main(int argc, const char *argv[argc]) { (void)argv; }
static void rascal_goodbye(void) {}
static void rascal_cleanup(void) {}

int main(const int argc, const char *argv[argc]) {
  rascal_init();
  rascal_welcome();
  rascal_main(argc, argv);
  rascal_goodbye();
  rascal_cleanup();
  return 0;
}
